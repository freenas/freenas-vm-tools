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

#include <fstream>
#include <Poco/Foundation.h>
#include <Poco/SharedPtr.h>
#include <Poco/AutoPtr.h>
#include <Poco/Message.h>
#include <Poco/DirectoryIterator.h>
#include <Poco/SharedLibrary.h>
#include "Config.hh"
#include "Server.hh"
#include "Context.hh"

Context::Context():
    m_logger(Poco::Logger::get("Context"))
{
}

void
Context::init(const std::string &plugin_path)
{
	Poco::DirectoryIterator it(plugin_path);

        for (; it != Poco::DirectoryIterator(); it++) {
		Poco::Path i(it.path().absolute());
		Poco::SharedLibrary *library;
		Service *plugin;

		try {
			library = new Poco::SharedLibrary(i.toString(),
			    Poco::SharedLibrary::Flags::SHLIB_LOCAL);
			plugin = (Service *)library->getSymbol("service");
		} catch (Poco::Exception &e) {
			m_logger.warning("Cannot load plugin %s: %s",
			    i.toString(), std::string(e.what()));
			continue;
		}

                const std::string &name = plugin->name();

		m_libraries.push_back(library);
                m_services.insert({name, plugin});
                m_server.registerService(name, plugin);
                m_logger.information("Plugin loaded: %s", name);
                plugin->init();
        }
}


void
Context::addDevice(Poco::SharedPtr<Device> device)
{

        m_device = device;
}


int
Context::run()
{
        try {
                m_server.start(m_device);
        } catch (std::runtime_error &e) {
                m_logger.critical("Cannot start server: %s", std::string(e.what()));
                m_logger.critical("Exiting");
                exit(0);
        }

	m_server.emitEvent("vmtools.ready", {
	    {"version_major", VERSION_MAJOR},
	    {"version_minor", VERSION_MINOR}
	});

        return (0);
}

Server &
Context::getServer()
{

        return (m_server);
}

void
Context::parseConfig(const std::string &path)
{

}