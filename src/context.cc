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

#include <dlfcn.h>
#include <fstream>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sinks.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include "server.hh"
#include "utils.hh"
#include "context.hh"

using namespace boost::filesystem;
using namespace boost::log::sinks::syslog;

void
context::init(const std::string &config_path)
{
	service *plugin;

        for (auto &i : boost::make_iterator_range(directory_iterator("."), {})) {
                if (i.path().extension() != m_loader->extension())
                        continue;

		try {
			plugin = m_loader->load(i.path().string());
		} catch (std::invalid_argument &e) {
			dolog(m_logger, error, format("Cannot load plugin %1%: %2%")
			    % i.path() % e.what());
			continue;
		}

                const std::string &name = plugin->name();

                m_services.insert({name, plugin});
                m_server.register_service(name, plugin);
                dolog(m_logger, error, format("Plugin loaded: %1%") % name);
                plugin->init();
        }
}

void
context::add_log_backend(std::shared_ptr<boost::log::sinks::sink> sink)
{
        boost::log::core::get()->add_sink(boost::shared_ptr<boost::log::sinks::sink>(&*sink));
        dolog(m_logger, error, format("Logging initialized"));
}

void
context::add_device(std::shared_ptr<device> device)
{
        m_device = device;
}

void
context::add_loader(std::shared_ptr<loader> loader)
{
	m_loader = loader;
}

int
context::run()
{
        try {
                m_server.start(m_device);
        } catch (std::runtime_error &e) {
                dolog(m_logger, critical, format("Cannot start server: %1%") % e.what());
                dolog(m_logger, critical, format("Exiting"));
                exit(0);
        }
        return (0);
}

server &
context::get_server()
{
        return (m_server);
}
