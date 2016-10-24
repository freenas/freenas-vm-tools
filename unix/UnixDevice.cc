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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <vector>
#include <string>
#include <cstring>
#include <cerrno>
#include <Poco/Foundation.h>
#include <Poco/Format.h>
#include <Poco/File.h>
#include "UnixDevice.hh"

static const std::vector<std::string> dev_paths {
    "/dev/virtio-ports/org.freenas.vm-tools",
    "/dev/vtcon/org.freenas.vm-tools",
    "/dev/ttyV0.0"
};

void
UnixDevice::open(const std::string &devnode)
{
        struct termios tc;

        m_path = devnode != "" ? devnode : findDeviceNode();
        m_fd = ::open(m_path.c_str(), O_RDWR);

	if (m_fd < 0)
		throw std::runtime_error(Poco::format("Cannot open device node: %s", ::strerror(errno)));

#ifdef __FreeBSD__
        tcgetattr(m_fd, &tc);
        tc.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(m_fd, TCSAFLUSH, &tc);
#endif
}

void
UnixDevice::close()
{
        if (m_fd >= 0) {
		::close(m_fd);
		m_fd = -1;
	}
}

bool
UnixDevice::connected()
{
        return (m_fd != -1);
}

int
UnixDevice::read(void *buf, int count) {
        size_t done = 0;
        ssize_t ret;

        while (done < count) {
                ret = ::read(m_fd, (char *) buf + done, count - done);
                if (ret < 0) {
                        if (errno == EINTR)
                                continue;

			::close(m_fd);
			m_fd = -1;
                        return (-1);
                }

                if (ret == 0)
                        return ((int)done);

                done += (size_t) ret;
        }

        return ((int)done);
}

int
UnixDevice::write(void *buf, int count) {
        size_t done = 0;
        ssize_t ret;

        while (done < count) {
                ret = ::write(m_fd, (char *) buf + done, count - done);
                if (ret < 0) {
                        if (errno == EINTR)
                                continue;

			::close(m_fd);
			m_fd = -1;
                        return (-1);
                }

                if (ret == 0)
                        return ((int)done);

		fsync(m_fd);
                done += (size_t) ret;
        }

        return ((int)done);
}

const std::string &
UnixDevice::findDeviceNode()
{
        for (auto &i : dev_paths) {
		Poco::File f(i);
                if (f.exists() && f.isDevice())
                        return (i);
        }

        throw std::runtime_error("No vm-tools device node found");
}
