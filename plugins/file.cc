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
#include <Poco/Foundation.h>
#include <Poco/FileStream.h>
#include <Poco/Base64Decoder.h>
#include <Poco/Base64Encoder.h>
#include <json.hh>
#include "../src/Server.hh"
#include "../src/Context.hh"

#ifdef __FreeBSD__
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

using namespace std::placeholders;

class file_service: public Service
{
public:
    virtual void init();
    virtual const std::string name() { return "file"; }
    virtual json get(const json &args);
};

void
file_service::init()
{
        m_methods = std::map<std::string, Service::method_type> {
            {"get", BIND_METHOD(&file_service::get)}
        };
}

json
file_service::get(const json &args)
{
	std::ostringstream ss;
	Poco::FileStream f(args[0], std::ios::in);
	Poco::Base64Encoder b64enc(ss);

	b64enc << f.rdbuf();

	return (json {
	    {"$binary", ss.str()}
	});
}

REGISTER_SERVICE(file_service)
