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
#include <Poco/DateTimeFormatter.h>
#include <Poco/File.h>
#include <Poco/DirectoryIterator.h>
#include <Poco/FileStream.h>
#include <Poco/Base64Decoder.h>
#include <Poco/Base64Encoder.h>
#include <json.hh>
#include "../src/Server.hh"
#include "../src/Context.hh"
#include "../src/Utils.h"

using namespace std::placeholders;

class FileService: public Service
{
public:
    virtual void init();
    virtual const std::string name() { return "file"; }
    virtual json ls(const json &args);
    virtual json get(const json &args);
};

void
FileService::init()
{
        m_methods = std::map<std::string, Service::method_type> {
	    {"ls", BIND_METHOD(&FileService::ls)},
            {"get", BIND_METHOD(&FileService::get)}
        };
}

json
FileService::ls(const json &args)
{
	Poco::DirectoryIterator it(args[0].get<std::string>());
	json result = json::array();

	for (; it != Poco::DirectoryIterator(); it++) {
		Poco::File file(it->path());
		std::string type = "FILE";
		const json &created_at = toDatetime(file.created());
		const json &modified_at = toDatetime(file.getLastModified());

		if (file.isDirectory())
			type = "DIRECTORY";

		if (file.isDevice())
			type = "DEVICE";

		if (file.isLink())
			type = "LINK";

		result.push_back({
			{"type", type},
			{"name", file.path()},
			{"size", file.getSize()},
			{"created_at", created_at},
			{"modified_at", modified_at}
		});
	}

	return (result);
}

json
FileService::get(const json &args)
{
	std::ostringstream ss;
	Poco::FileStream f(args[0], std::ios::in);
	Poco::Base64Encoder b64enc(ss);

	b64enc << f.rdbuf();

	return (json {
	    {"$binary", ss.str()}
	});
}

REGISTER_SERVICE(FileService)
