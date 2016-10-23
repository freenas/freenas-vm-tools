/*-
 * Copyright 2016 iXsystems, Inc.
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <fcntl.h>
#include <exception>
#include <functional>
#include <fstream>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sinks.hpp>
#include "json.hh"
#include "utils.hh"
#include "server.hh"

#define HEADER_MAGIC    0xdeadbeef

using namespace boost::log::sinks::syslog;
using namespace boost::uuids;

void
server::start(std::shared_ptr<device> device)
{
        m_device = device;
        m_device->open();
	m_reader = std::thread(&server::reader, this);
}

void
server::emit_event(const std::string &name, const json &args)
{
        uuid id = random_generator()();
        std::string msg = pack("events", "event", id, {
           {"name", name},
           {"args", args}
        });

        send(msg);
}

void
server::register_service(const std::string &name, service *impl)
{

        m_services.insert({name, impl});
}

bool
server::connected()
{

        return (m_device->connected());
}

void
server::handle(std::unique_ptr<std::string> payload)
{
        string_generator gen;
        json frame;
        uuid id;

        dolog(m_logger, debug, format("Payload: %1%") % *payload);

        try {
                frame = json::parse(*payload);
                id = gen(frame["id"].get<std::string>());
        } catch (std::invalid_argument &e) {
                return;
        }

        if (frame["namespace"] != "rpc") {
                send_error(id, EINVAL, "Invalid request");
                return;
        }

        if (!frame.count("args")) {
                send_error(id, EINVAL, "Invalid request");
                return;
        }

        if (frame["name"] == "call")
                on_rpc_call(id, frame["args"]);

        if (frame["name"] == "response")
                on_rpc_response(id, frame["args"]);
}

void
server::send(const std::string &payload)
{
	uint32_t size = static_cast<uint32_t>(payload.size());
	uint32_t header[2] {
	    HEADER_MAGIC,
	    size
	};

	m_device->write((void *)&header, sizeof(header));
	m_device->write((void *)payload.c_str(), size);
}

void
server::dispatch_rpc(call *call)
{
        try {
                json result = call->m_service->dispatch(call->m_method,
                                                       call->m_args);
                send_response(call->m_id, result);
        } catch (exception &e) {
                send_error(call->m_id, e.errnum(), e.what());
        } catch (std::exception &e) {
                send_error(call->m_id, EFAULT, e.what());
        }

        m_mtx.lock();
        m_server_calls.erase(call->m_id);
        m_mtx.unlock();
}

void
server::on_rpc_call(const uuid &id, const json &data)
{
        service *service;
        std::string method;
        std::string path = data["method"];
        std::vector<std::string> parts;

        parts = boost::algorithm::split(parts, path,
            boost::algorithm::is_any_of("."));

	if (parts.size() < 2) {
		send_error(id, EINVAL, "Invalid method");
		return;
	}

        dolog(m_logger, error, format("Request: %1%") % parts[0]);

        try {
                service = m_services.at(parts[0]);
                method = parts[1];
        } catch (std::out_of_range) {
                send_error(id, ENOENT, str(format("Service %1% not found") % parts[0]));
		return;
        }

        m_mtx.lock();

        call *c = new call {
            .m_id = id,
            .m_service = service,
            .m_method = method,
            .m_args = data["args"]
        };

	m_server_calls.insert({id, std::shared_ptr<call>(c)});

        new std::thread([this, c] { dispatch_rpc(c); });

        m_mtx.unlock();
}

void
server::on_rpc_response(const uuid &id, const json &data)
{

}

void
server::send_response(const uuid &id, const json &response)
{
        const std::string &msg = pack("rpc", "response", id, response);

	send(msg);
}

void
server::send_error(const uuid &id, int errnum,
    const std::string &errstr)
{
        const std::string &msg = pack("rpc", "error", id, {
            {"code", errnum},
            {"message", errstr}
        });

        send(msg);
}

const std::string
server::pack(const std::string &ns, const std::string &name,
    const uuid &id, const json &payload)
{
        json msg = {
            {"namespace", ns},
            {"name", name},
            {"id", boost::uuids::to_string(id)},
            {"args", payload}
        };

        return std::move(msg.dump());
}

void
server::reader()
{
        ssize_t ret;

        for (;;) {
                uint32_t header[2];
                uint32_t magic, size;

                /* Read the header first */
                ret = m_device->read((void *)header, sizeof(header));
                if (ret < sizeof(header)) {
                        dolog(m_logger, error,
                            format("Short read: %1% bytes instead of %2%")
                                % ret % sizeof(header));

                        break;
                }

                magic = header[0];
                size = header[1];

                if (magic != HEADER_MAGIC) {
                        dolog(m_logger, error,
                            format("Invalid read: invalid magic %1%")
                                % magic);
                        break;
                }

                auto payload = std::unique_ptr<std::string>(
                    new std::string(size, '\0'));

                ret = m_device->read(&(*payload)[0], size);
                if (ret < size) {
                        dolog(m_logger, error,
                            format("Short payload read: %1% instead of %1%")
                                % magic % ret % size);
                        break;
                }

                handle(std::move(payload));
        }
}
