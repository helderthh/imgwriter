#ifndef __UTILS_H__
#define __UTILS_H__

#include <iostream>


// Report a failure
inline void fail(boost::system::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << std::endl;
}

#endif