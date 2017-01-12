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
#include <Poco/Foundation.h>
#include <Poco/SharedPtr.h>
#include <Poco/Thread.h>
#include <Poco/RunnableAdapter.h>
#include <Poco/ScopedLock.h>
#include <Poco/StringTokenizer.h>
#include <Poco/UUID.h>
#include <Poco/UUIDGenerator.h>
#include "json.hh"
#include "Device.hh"
#include "Server.hh"

#define HEADER_MAGIC    0xdeadbeef

Server::Server():
    m_logger(Poco::Logger::get("Server"))
{
}

void
Server::start(Poco::SharedPtr<Device> device)
{
	static Poco::RunnableAdapter<Server> runnable(*this, &Server::reader);

	m_device = device;
	m_device->open();
	m_reader.start(runnable);
}

void
Server::emitEvent(const std::string &name, const json &args)
{
	Poco::UUID id = Poco::UUIDGenerator().createRandom();
	std::string msg = pack("events", "event", id, {
	   {"name", name},
	   {"args", args}
	});

	send(msg);
}

void
Server::registerService(const std::string &name, Service *impl)
{

	m_services.insert({name, impl});
}

bool
Server::connected()
{

	return (m_device->connected());
}

void
Server::handle(Poco::SharedPtr<std::string> payload)
{
	json frame;
	Poco::UUID id;

	m_logger.debug("Request payload: %s", *payload);

	try {
		frame = json::parse(*payload);
		id = Poco::UUID(frame["id"].get<std::string>());
	} catch (std::invalid_argument &e) {
		return;
	}

	if (frame["namespace"] != "rpc") {
		sendError(id, EINVAL, "Invalid request");
		return;
	}

	if (!frame.count("args")) {
		sendError(id, EINVAL, "Invalid request");
		return;
	}

	if (frame["name"] == "call") {
		onRpcCall(id, frame["args"]);
		return;
	}

	if (frame["name"] == "response") {
		onRpcResponse(id, frame["args"]);
		return;
	}
}

void
Server::send(const std::string &payload)
{
	Poco::ScopedLock<Poco::Mutex> mtx(m_mtx);
	int ret;
	uint32_t size = static_cast<uint32_t>(payload.size());
	uint32_t header[2] {
	    HEADER_MAGIC,
	    size
	};

	m_device->write((void *)&header, sizeof(header));
	ret = m_device->write((void *)payload.c_str(), size);

	if (ret != size)
		m_logger.error("Short write: %d bytes instead of %d", ret, size);
}

void
Server::dispatchRpc(Call *call)
{
	try {
		json result = call->m_service->dispatch(call->m_method,
		    call->m_args);
		sendResponse(call->m_id, result);
	} catch (RpcException &e) {
		sendError(call->m_id, e.errnum(), e.what());
	} catch (std::exception &e) {
		sendError(call->m_id, EFAULT, e.what());
	}

	m_mtx.lock();
	m_server_calls.erase(call->m_id);
	m_mtx.unlock();
}

void
Server::onRpcCall(const Poco::UUID &id, const json &data)
{
	Poco::ScopedLock<Poco::Mutex> mtx(m_mtx);
	Service *service;
	std::thread *t;
	std::string method;
	std::string path = data["method"];
	Poco::StringTokenizer parts(path, ".");

	if (parts.count() < 2) {
		sendError(id, EINVAL, "Invalid method");
		return;
	}

	m_logger.debug("Request: %s", parts[0]);

	try {
		service = m_services.at(parts[0]);
		method = parts[1];
	} catch (std::out_of_range) {
		sendError(id, ENOENT, Poco::format("Service %s not found",
		    parts[0]));
		return;
	}

	Call *c = new Call {
	    .m_id = id,
	    .m_service = service,
	    .m_method = method,
	    .m_args = data["args"]
	};

	m_server_calls.insert({id, Poco::SharedPtr<Call>(c)});
	c->m_thread.startFunc([this, c] { Server::dispatchRpc(c); });
}

void
Server::onRpcResponse(const Poco::UUID &id, const json &data)
{

}

void
Server::sendResponse(const Poco::UUID &id, const json &response)
{
	const std::string &msg = pack("rpc", "response", id, response);

	send(msg);
}

void
Server::sendError(const Poco::UUID &id, int errnum,
    const std::string &errstr)
{
	const std::string &msg = pack("rpc", "error", id, {
	    {"code", errnum},
	    {"message", errstr}
	});

	send(msg);
}

const std::string
Server::pack(const std::string &ns, const std::string &name,
    const Poco::UUID &id, const json &payload)
{
	json msg = {
	    {"namespace", ns},
	    {"name", name},
	    {"id", id.toString()},
	    {"args", payload}
	};

	return std::move(msg.dump());
}

void
Server::reader()
{
	ssize_t ret;

	for (;;) {
		uint32_t header[2];
		uint32_t magic, size;

		/* Read the header first */
		ret = m_device->read((void *)header, sizeof(header));
		if (ret < sizeof(header)) {
			m_logger.critical("Short read: %d bytes instead of %d",
			    ret, sizeof(header));

			break;
		}

		magic = header[0];
		size = header[1];

		if (magic != HEADER_MAGIC) {
			m_logger.critical("Invalid read: invalid magic %d",
			    magic);
			break;
		}

		auto payload = Poco::SharedPtr<std::string>(new std::string(size, '\0'));

		ret = m_device->read(&(*payload)[0], size);
		if (ret < size) {
			m_logger.critical("Short payload read: %d instead of %d",
			    ret, size);
			break;
		}

		handle(payload);
	}
}
