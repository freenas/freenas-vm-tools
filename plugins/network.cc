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
#include "../src/json.hh"
#include "../src/server.hh"
#include "../src/context.hh"

#include <sys/types.h>
#include <ifaddrs.h>

using namespace std::placeholders;

class network_service: public service
{
public:
    virtual void init();
    virtual const std::string name() { return "network"; }
    virtual json interfaces(const json &args);
};

void
network_service::init()
{
	m_methods = std::map<std::string, service::method_type> {
	    {"interfaces", BIND_METHOD(&network_service::interfaces)},
	};
}

json
network_service::interfaces(const json &args)
{
	struct ifaddrs *ifaddr, *ifa;
	json result;

	if (getifaddrs(&ifaddr) != 0)
		throw exception(errno, ::strerror(errno));

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		result.push_back({
			{"name", ifa->ifa_name}
		});
	}

	return (result);
}

REGISTER_SERVICE(network_service)
