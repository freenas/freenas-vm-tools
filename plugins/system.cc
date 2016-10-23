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

#include <functional>
#include <string>
#include <map>
#include <boost/config.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "../src/json.hh"
#include "../src/server.hh"
#include "../src/context.hh"

#if defined(__FreeBSD__) || defined(__APPLE__)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

using namespace boost::posix_time;
using namespace std::placeholders;

class system_service: public service
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
system_service::init()
{
        m_methods = std::map<std::string, service::method_type> {
            {"ping", BIND_METHOD(&system_service::ping)},
	    {"uptime", BIND_METHOD(&system_service::uptime)},
	    {"loadavg", BIND_METHOD(&system_service::loadavg)},
	    {"exec", BIND_METHOD(&system_service::exec)}
        };
}

json
system_service::ping(const json &args)
{
        return ("pong");
}

json
system_service::uptime(const json &args)
{
	struct timeval tv;
	size_t size = sizeof(struct timeval);

#if defined(__FreeBSD__) || defined(__APPLE__)
	if (sysctlbyname("kern.boottime", &tv, &size, NULL, 0) < 0)
		throw exception(errno, "Cannot obtain system uptime");
#else
	throw exception(ENOTSUP, "Not supported");
#endif

	ptime boottime = from_time_t(tv.tv_sec);
	ptime now = second_clock::universal_time();
	return ((now - boottime).total_seconds());
}

json
system_service::loadavg(const json &args)
{
	double loadavg[3];

	if (getloadavg(loadavg, 3) == -1)
		throw exception(errno, ::strerror(errno));

	return {loadavg[0], loadavg[1], loadavg[2]};
}

json
system_service::exec(const json &args)
{

}

REGISTER_SERVICE(system_service)
