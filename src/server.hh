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
#include <boost/asio.hpp>
#include <boost/uuid/uuid.hpp>
#include "json.hh"

#define BIND_METHOD(name)       std::bind(name, this, _1)

using nlohmann::json;

class service
{
public:
    typedef std::function<json (const json &)> method_type;

    virtual void init() = 0;
    virtual json dispatch(const std::string &method, json &args)
    {
            if (!m_methods.count(method)) {

            }

            auto func = m_methods[method];
            return func(args);
    }

protected:
    std::map<std::string, method_type> m_methods;
};

class call
{
private:
    boost::uuids::uuid m_id;
    std::thread m_thread;
};

class server
{
public:
    void start();
    void emit_event(const std::string &name, json &args);
    void register_service(const std::string &name, service &impl);

private:
    void reader();
    void handle(std::string &payload);
    const std::string &find_device_node() const;

    std::map<std::string, service &> m_services;
    std::map<boost::uuids::uuid, call &> m_calls;
    std::thread m_reader;
    std::fstream m_fd;
};

#endif //FREENAS_VM_TOOLS_SERVER_HH
