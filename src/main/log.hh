#ifndef LOG_HH
#define LOG_HH

#include <boost/log/trivial.hpp>

#define Log BOOST_LOG_TRIVIAL

void log_init(bool quiet=false);

#endif /* LOG_HH */
