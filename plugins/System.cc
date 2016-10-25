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

#include <sstream>
#include <vector>
#include <map>
#include <Poco/Foundation.h>
#include <Poco/DateTime.h>
#include <Poco/Process.h>
#include <Poco/PipeStream.h>
#include <Poco/StreamCopier.h>
#include <json.hh>
#include "../src/Server.hh"
#include "../src/Context.hh"

#if defined(__FreeBSD__) || defined(__APPLE__)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

using namespace std::placeholders;

class SystemService: public Service
{
public:
    virtual void init();
    virtual const std::string name() { return "system"; }
    virtual json ping(const json &args);
    virtual json uptime(const json &args);
    virtual json loadavg(const json &args);
    virtual json exec(const json &args);
};

void
SystemService::init()
{
        m_methods = std::map<std::string, Service::method_type> {
            {"ping", BIND_METHOD(&SystemService::ping)},
	    {"uptime", BIND_METHOD(&SystemService::uptime)},
	    {"loadavg", BIND_METHOD(&SystemService::loadavg)},
	    {"exec", BIND_METHOD(&SystemService::exec)}
        };
}

json
SystemService::ping(const json &args)
{
        return ("pong");
}

json
SystemService::uptime(const json &args)
{
	struct timeval tv;
	size_t size = sizeof(struct timeval);

#if defined(__FreeBSD__) || defined(__APPLE__)
	if (sysctlbyname("kern.boottime", &tv, &size, NULL, 0) < 0)
		throw RpcException(errno, "Cannot obtain system uptime");
#else
	throw RpcException(ENOTSUP, "Not supported");
#endif

	Poco::DateTime boottime(tv.tv_sec);
	Poco::DateTime now;
	return ((now - boottime).totalSeconds());
}

json
SystemService::loadavg(const json &args)
{
	double loadavg[3];

	if (getloadavg(loadavg, 3) == -1)
		throw RpcException(errno, ::strerror(errno));

	return {loadavg[0], loadavg[1], loadavg[2]};
}

json
SystemService::exec(const json &args)
{
	std::ostringstream ret;
	Poco::Pipe out;
	Poco::PipeInputStream istr(out);
	Poco::ProcessHandle p = Poco::Process::launch(args[0], args[1], nullptr,
	    &out, &out);

	Poco::StreamCopier::copyStream(istr, ret);
	p.wait();
	return (ret.str());
}

REGISTER_SERVICE(SystemService)
