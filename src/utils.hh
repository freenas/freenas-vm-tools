//
// Created by Jakub Klama on 15.07.2016.
//

#ifndef FREENAS_VM_TOOLS_UTILS_HH
#define FREENAS_VM_TOOLS_UTILS_HH

#include <boost/log/sources/severity_logger.hpp>
#include <boost/format.hpp>

void dolog(boost::log::sources::severity_logger<> &logger, int severity,
    boost::format fmt);


#endif //FREENAS_VM_TOOLS_UTILS_HH
