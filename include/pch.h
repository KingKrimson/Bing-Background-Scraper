#pragma once

#if defined(_WIN32)
#define _WIN32_WINNT 0x0501
#endif

#if _MSC_VER>=1700
#include <filesystem>
namespace boost {
    namespace filesystem = std::tr2::sys;
}
#else
#include <boost\filesystem.hpp>
#endif

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <functional>
#include <future>
#include <thread>
#include <mutex>
#include <boost/asio.hpp>