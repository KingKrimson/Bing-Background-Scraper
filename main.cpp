#include "include\main.h"
#include "include\request_base.h"

#if _MSC_VER>=1700
#include <filesystem>
#else
#include <boost\filesystem.hpp>
namespace std {
    namespace tr2 {
        namespace sys = boost::filesystem;
    }
}
#endif

#include <mutex>
#include <string>
#include <vector>
#include <algorithm>
#include <future>
#include <chrono>
#include <boost\system\error_code.hpp>

namespace {
    using url = std::pair<std::string, std::string>;
    std::mutex image_urls_mutex;
    std::vector<url> image_urls;

    bing_xml_request::bing_xml_request(std::string server, std::string path)
        : request_base(std::move(server), std::move(path))
    {}

    void bing_xml_request::read_content(const boost::system::error_code& error,
        size_t bytes_transferred)
    {
        if (!error) {
            request_base::read_content(error, bytes_transferred);
        } else {
            data << &response;

            // find each image URL
            size_t pos { 0 };
            for (;;) {
                auto start = data.str().find("<urlBase>", pos) + 9;
                auto end = data.str().find("</urlBase>", start);

                if (start - 9 == std::string::npos ||
                    end == std::string::npos) {
                        break;
                }

                std::string lhs { data.str(), start, end - start };

                auto off = lhs.find_last_of('/');
                auto len = lhs.find_first_of('_') - off;
                std::string rhs { lhs.substr(off, len) };

                image_urls_mutex.lock();
                image_urls.emplace_back(lhs, rhs);
                image_urls_mutex.unlock();

                pos = end + 10;
            };
        }
    }

    image_request::image_request(std::string server, std::string path,
        std::string filename)
        : request_base(std::move(server), std::move(path)),
        filename(std::move(filename))
    {}

    void image_request::read_content(const boost::system::error_code& error,
        size_t bytes_transferred)
    {
        if (!error) {
            request_base::read_content(error, bytes_transferred);
        } else {
            data << &response;

            if (data.str().substr(0, 4) != jpeg_magic_number) {
                return;
            }

            auto out = std::ofstream { filename, std::ios::binary };
            if (out.is_open()) {
                out << data.str();
                out.close();

                std::cout << "saved: " << filename << std::endl;
            }
        }
    }

    void image_request::read_status(const boost::system::error_code& error,
        size_t bytes_transferred)
    {
        if (!error) {
            std::istream response_stream(&response);
            std::string http_version;
            response_stream >> http_version;
            unsigned int status_code;
            response_stream >> status_code;

            if (status_code != 200) {
                return;
            }

            request_base::read_status(error, bytes_transferred);
        }
    }
}

int main(int argc, char **argv)
{
    // check for user-defined directory
    if (argc == 1) {
        std::string executable_name { argv[0] };
        auto off = executable_name.find_last_of('\\') + 1;
        auto len = executable_name.length();
        executable_name = executable_name.substr(off, len - off);

        std::cout << "please supply a save directory" << std::endl <<
            "Example: \"" << executable_name << "\" \"C:/bing backgrounds\"" <<
            std::endl;
        return 1;
    }

    std::tr2::sys::create_directories(std::tr2::sys::path { argv[1] });

    std::vector<std::future<void>> xml_tasks;
    xml_tasks.reserve(country_codes.size());

    // asynchronously download and parse the image URLs
    for (const auto& it : country_codes) {
        xml_tasks.emplace_back(std::async(std::launch::async, [=]() {
            bing_xml_request { website, base_path + it }.run();
        }));
    }

    for (const auto& it : xml_tasks) {
        it.wait();
    }

    // sort URLs by their filename
    std::sort(image_urls.begin(), image_urls.end(),
        [](const url& lhs, const url& rhs) {
            return lhs.second < rhs.second;
    });

    // split the collection of URLs into groups based on the filename
    std::vector<std::vector<url>> image_groups;
    auto start = image_urls.begin();
    while (start != image_urls.end()) {
        auto end = std::find_if(start, image_urls.end(), [=](const url& item) {
            return item.second != start->second;
        });

        image_groups.emplace_back(start, end);
        start = end;
    }

    std::vector<std::future<void>> image_threads;
    image_threads.reserve(image_groups.size());

    const auto resolution = resolutions[12];

    // asynchronously download and save the images
    for (const auto& group : image_groups) {
        image_threads.emplace_back(std::async(std::launch::async, [=]() {
            for (const auto& it : group) {
                std::string file { argv[1] + it.second + resolution };

                if (std::tr2::sys::exists(std::tr2::sys::path { file })) {
                    break;
                }

                image_request { website, it.first + resolution, file }.run();
            }
        }));
    }

    for (const auto& it : image_threads) {
        it.wait();
    }

    return 0;
}
