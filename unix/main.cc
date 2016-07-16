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

#include "../src/config.hh"

#include <unistd.h>
#include <iostream>
#include <boost/log/core.hpp>
#include <boost/log/sinks.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include "../src/context.hh"
#include "unix_device.hh"

using namespace boost;
namespace po = boost::program_options;

boost::shared_ptr<boost::log::sinks::sink>
init_native_syslog()
{
        typedef log::sinks::synchronous_sink<log::sinks::syslog_backend> sink_type;

        // Create a backend
        boost::shared_ptr<log::sinks::syslog_backend> backend(new log::sinks::syslog_backend(
            log::keywords::facility = log::sinks::syslog::user,
            log::keywords::use_impl = log::sinks::syslog::native
        ));

        // Set the straightforward level translator for the "Severity" attribute of type int
        backend->set_severity_mapper(log::sinks::syslog::direct_severity_mapping<int>("Severity"));

        // Wrap it into the frontend and register in the core.
        // The backend requires synchronization in the frontend.
        return (boost::make_shared<sink_type>(backend));
}

int
main(int argc, char *argv[])
{
        context ctx;
        bool foreground;
        bool help;
        std::string config;

        po::options_description desc("Allowed options");
        desc.add_options()
            ("h", "Produce help message")
            ("c", "Config file path")
            ("foreground,f", "Do not daemonize");

        po::parse_command_line(argc, argv, desc);

        if (0) {
                if (::daemon(0, 0) < 0) {
                        std::cerr <<
                        format("Cannot daemonize: %1%") % strerror(errno);
                        exit(1);
                }
        }

        ctx.add_device(std::shared_ptr<device>(new unix_device()));
        ctx.add_log_backend(init_native_syslog());
        ctx.init(config);
        return (ctx.run());
}
