//
// Created by Jakub Klama on 15.07.2016.
//

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/format.hpp>

void
dolog(boost::log::sources::severity_logger<> &logger, int severity,
boost::format fmt) {
    BOOST_LOG_SEV(logger, severity) << fmt.str();
}
