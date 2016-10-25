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
#include <Poco/Foundation.h>
#include <Poco/Format.h>
#include <json.hh>
#include "../src/Server.hh"
#include "../src/Context.hh"
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <netdb.h>

#if defined(__linux__)
#include <linux/if_packet.h>
#endif

#if defined(__FreeBSD__) || defined(__APPLE__)
#include <net/if_dl.h>
#endif

using namespace std::placeholders;

class NetworkService: public Service
{
public:
    virtual void init();
    virtual const std::string name() { return "network"; }
    virtual json interfaces(const json &args);
};

void
NetworkService::init()
{
	m_methods = std::map<std::string, Service::method_type> {
	    {"interfaces", BIND_METHOD(&NetworkService::interfaces)},
	};
}

json
NetworkService::interfaces(const json &args)
{
	struct ifaddrs *ifaddr, *ifa;
	char addr[NI_MAXHOST];
	char netmask[NI_MAXHOST];
	char broadcast[NI_MAXHOST];
	json result;

	if (getifaddrs(&ifaddr) != 0)
		throw RpcException(errno, ::strerror(errno));

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (!result.count(ifa->ifa_name)) {
			result[ifa->ifa_name] = {
			    {"aliases", json::array()}
			};
		}

#if defined(__linux__)
		if (ifa->ifa_addr->sa_family == AF_PACKET) {
			struct sockaddr_ll *sll = (struct sockaddr_ll *)ifa->ifa_addr;
			const std::string fmt = Poco::format(
			    "%02x:%02x:%02x:%02x:%02x:%02x",
			    sll->sll_addr[0], sll->sll_addr[1],
			    sll->sll_addr[2], sll->sll_addr[3],
			    sll->sll_addr[4], sll->sll_addr[5]);

			result[ifa->ifa_name]["aliases"].push_back({
			    {"af", "LINK"},
			    {"address", fmt},
			});

			continue;
		}
#endif

#if defined(__FreeBSD__) || defined(__APPLE__)
		if (ifa->ifa_addr->sa_family == AF_LINK) {
			struct sockaddr_dl *sdl = (struct sockaddr_dl *)ifa->ifa_addr;
			unsigned char mac = (unsigned char *)LLADDR(sdl);
			const std::string fmt = Poco::format(
			    "%02x:%02x:%02x:%02x:%02x:%02x",
			    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);


			result[ifa->ifa_name]["aliases"].push_back({
			    {"af", "LINK"},
			    {"address", fmt},
			});

			continue;
		}
#endif

		if (getnameinfo(ifa->ifa_addr,
		    sizeof(struct sockaddr_storage), addr, sizeof(addr),
		    NULL, 0, NI_NUMERICHOST) != 0)
			continue;

		if (getnameinfo(ifa->ifa_netmask,
		    sizeof(struct sockaddr_storage), netmask,
		    sizeof(netmask), NULL, 0, NI_NUMERICHOST) != 0)
			continue;

		result[ifa->ifa_name]["aliases"].push_back({
		    {"af", ifa->ifa_addr->sa_family == AF_INET ? "INET" : "INET6"},
		    {"address", addr},
		    {"netmask", netmask}
		});
	}

	return (result);
}

REGISTER_SERVICE(NetworkService)
