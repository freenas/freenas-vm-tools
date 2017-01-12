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

#include <algorithm>
#include <fstream>
#include <sstream>
#include <map>
#include <ctime>
#include <Poco/Foundation.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/File.h>
#include <Poco/DirectoryIterator.h>
#include <Poco/FileStream.h>
#include <Poco/Base64Decoder.h>
#include <Poco/Base64Encoder.h>
#include <json.hh>
#include "../src/Server.hh"
#include "../src/Context.hh"

using namespace std::placeholders;

class TimeService: public Service
{
public:
    virtual void init();
    virtual const std::string name() { return "time"; }
    virtual json gettime(const json &args);
    virtual json settime(const json &args);
};

void
TimeService::init()
{
	m_methods = std::map<std::string, Service::method_type> {
	    {"gettime", BIND_METHOD(&TimeService::gettime)},
	    {"settime", BIND_METHOD(&TimeService::settime)},
	};
}

json
TimeService::gettime(const json &args)
{
	struct timespec ts;
	Poco::Timestamp timestamp;

	if (clock_gettime(CLOCK_REALTIME, &ts) != 0)
		throw RpcException(errno, strerror(errno));

	throw RpcException(ENOTSUP, "Not implemented");
}

json
TimeService::settime(const json &args)
{

	throw RpcException(ENOTSUP, "Not implemented");
}

REGISTER_SERVICE(TimeService)
