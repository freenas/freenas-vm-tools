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

#include <exception>
#include <fstream>
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include "json.hh"
#include "server.hh"

#define HEADER_MAGIC    0xbadbeef0

static const std::vector<std::string> dev_paths {
    "/dev/virtio-ports/org.freenas.vm-tools",
    "/dev/vtcon/org.freenas.vm-tools"
};

void
server::start()
{

}

void
server::emit_event(const std::string &name, json &args)
{

}

void
server::register_service(const std::string &name, service &impl)
{

        m_services.insert({name, impl});
}

void
server::handle(std::string &payload)
{
        json frame;

        try {
                frame = json::parse(payload);

        } catch (std::invalid_argument) {

        }

}

void
server::reader()
{
        while (!m_fd.eof()) {
                std::auto_ptr<std::string> payload;
                uint32_t header[2];
                uint32_t magic, size;

                /* Read the header first */
                m_fd.read(reinterpret_cast<char *>(&header), sizeof(header));

                if (m_fd.gcount() < sizeof(header)) {
                        /* short read */
                }

                magic = header[0];
                size = header[1];

                if (magic != HEADER_MAGIC) {
                        /* invalid magic */
                }

                payload = std::auto_ptr<std::string>(new std::string(size, '\0'));
                m_fd.read(&(*payload)[0], size);
                handle(*payload);
        }
}

const std::string &
server::find_device_node() const
{
        for (auto &i : dev_paths) {
                if (boost::filesystem::exists(i))
                        return (i);
        }

        throw new std::runtime_error("No vm-tools device node found");
}
