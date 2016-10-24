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
#include <Poco/Foundation.h>
#include <Poco/Format.h>
#include <Poco/UUID.h>
#include <Poco/SharedPtr.h>
#include <Poco/Logger.h>
#include <Poco/Thread.h>
#include <Poco/ThreadPool.h>
#include <json.hh>
#include "Device.hh"

#define BIND_METHOD(_name)       std::bind(_name, this, _1)

using nlohmann::json;

class RpcException: public std::runtime_error
{
public:
    RpcException(int errnum, const std::string &errstr):
        runtime_error(errstr), m_errnum(errnum)
    {}

    int errnum()
    {
            return (m_errnum);
    }

private:
    int m_errnum;
};

class Service
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
                    throw RpcException(ENOENT,
	                Poco::format("Method %s not found", method));
            }
    }

protected:
    std::map<std::string, method_type> m_methods;
};

class Call
{
public:
    Poco::Thread m_thread;
    Poco::UUID m_id;
    Service *m_service;
    json m_args;
    std::string m_method;
};

class Server
{
public:
    Server();
    void start(Poco::SharedPtr<Device> device);
    void emitEvent(const std::string &name, const json &args);
    void registerService(const std::string &name, Service *impl);
    bool connected();

private:
    void reader();
    void handle(Poco::SharedPtr<std::string> payload);
    void send(const std::string &payload);
    void onRpcCall(const Poco::UUID &id, const json &data);
    void onRpcResponse(const Poco::UUID &id, const json &data);
    void dispatchRpc(Call *c);
    void sendResponse(const Poco::UUID &id, const json &response);
    void sendError(const Poco::UUID &id, int errnum,
        const std::string &errstr);
    const std::string pack(const std::string &ns, const std::string &name,
        const Poco::UUID &id, const json &payload);

    Poco::Logger &m_logger;
    Poco::Thread m_reader;
    Poco::Mutex m_mtx;
    Poco::SharedPtr<Device> m_device;
    std::map<std::string, Service *> m_services;
    std::map<Poco::UUID, Poco::SharedPtr<Call>> m_server_calls;
    std::map<Poco::UUID, Poco::SharedPtr<Call>> m_client_calls;
};

#endif //FREENAS_VM_TOOLS_SERVER_HH
