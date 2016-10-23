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

#ifndef FREENAS_VM_TOOLS_SERVER_HH
#define FREENAS_VM_TOOLS_SERVER_HH

#include <fstream>
#include <thread>
#include <map>
#include <functional>
#include <mutex>
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include "device.hh"
#include "json.hh"

#define BIND_METHOD(_name)       std::bind(_name, this, _1)

using nlohmann::json;
using boost::format;
using namespace boost::uuids;

class exception: public std::runtime_error
{
public:
    exception(int errnum, const std::string &errstr):
        runtime_error(errstr), m_errnum(errnum)
    {}

    int errnum()
    {
            return (m_errnum);
    }

private:
    int m_errnum;
};

class service
{
public:
    typedef std::function<json (const json &)> method_type;

    virtual void init()
    {
    }

    virtual const std::string name() = 0;

    virtual json dispatch(const std::string &method, json &args)
    {
            try {
                    auto func = m_methods.at(method);
                    return func(args);
            } catch (std::out_of_range &e) {
                    throw exception(ENOENT,
                        str(format("Method %1% not found") % method));
            }
    }

protected:
    std::map<std::string, method_type> m_methods;
};

class call
{
public:
    boost::uuids::uuid m_id;
    service *m_service;
    json m_args;
    std::string m_method;
};

class server
{
public:
    void start(std::shared_ptr<device> device);
    void emit_event(const std::string &name, const json &args);
    void register_service(const std::string &name, service *impl);
    bool connected();

private:
    void reader();
    void handle(std::unique_ptr<std::string> payload);
    void send(const std::string &payload);
    void on_rpc_call(const uuid &id, const json &data);
    void on_rpc_response(const uuid &id, const json &data);
    void dispatch_rpc(call *c);
    void send_response(const uuid &id, const json &response);
    void send_error(const uuid &id, int errnum,
        const std::string &errstr);
    const std::string pack(const std::string &ns, const std::string &name,
        const uuid &id, const json &payload);

    boost::log::sources::severity_logger<> m_logger;
    std::map<std::string, service *> m_services;
    std::map<uuid, std::shared_ptr<call>> m_server_calls;
    std::map<uuid, std::shared_ptr<call>> m_client_calls;
    std::thread m_reader;
    std::mutex m_mtx;
    std::shared_ptr<device> m_device;
};

#endif //FREENAS_VM_TOOLS_SERVER_HH
