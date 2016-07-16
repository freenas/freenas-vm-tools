//
// Created by Jakub Klama on 15.07.2016.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <vector>
#include <string>
#include <boost/filesystem.hpp>
#include "unix_device.hh"

namespace fs = boost::filesystem;

static const std::vector<std::string> dev_paths {
    "/dev/virtio-ports/org.freenas.vm-tools",
    "/dev/vtcon/org.freenas.vm-tools",
    "/dev/ttyV0.0"
};

void
unix_device::open(const std::string &devnode)
{
        struct termios tc;

        m_path = devnode != "" ? devnode : find_device_node();
        m_fd = ::open(m_path.c_str(), O_RDWR);

        tcgetattr(m_fd, &tc);
        tc.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(m_fd, TCSAFLUSH, &tc);
}

void
unix_device::close()
{
        if (m_fd >= 0) {
		::close(m_fd);
		m_fd = -1;
	}
}

bool
unix_device::connected()
{
        return (m_fd != -1);
}

int
unix_device::read(void *buf, int count) {
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
unix_device::write(void *buf, int count) {
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
unix_device::find_device_node()
{
        for (auto &i : dev_paths) {
                if (fs::status(i).type() == fs::character_file)
                        return (i);
        }

        throw std::runtime_error("No vm-tools device node found");
}
