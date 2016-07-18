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

#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>
#include <boost/format.hpp>
#include <ostream>
#include <istream>
#include <sstream>

using namespace boost::archive::iterators;

void
dolog(boost::log::sources::severity_logger<> &logger, int severity,
    boost::format fmt)
{
	BOOST_LOG_SEV(logger, severity) << fmt.str();
}

void
b64encode(const std::string &text, std::stringstream &out)
{
	typedef
	    base64_from_binary<transform_width<std::string::const_iterator, 6, 8>>
	    base64_text;

	std::copy(
	    base64_text(text.begin()),
	    base64_text(text.end()),
	    std::ostream_iterator<char>(out)
	);
}

void
b64decode(const std::string &text, std::stringstream &out)
{
	typedef
	    transform_width<binary_from_base64<std::string::const_iterator>, 6, 8>
	    base64_text;

	std::copy(
	    base64_text(text.begin()),
	    base64_text(text.end()),
	    std::ostream_iterator<char>(out)
	);
}
