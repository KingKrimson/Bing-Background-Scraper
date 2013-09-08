#include "include\main.h"
#include "include\request_base.h"

#if _MSC_VER>=1700
#include <filesystem>
namespace boost {
    namespace filesystem = std::tr2::sys;
}
#else
#include <boost\filesystem.hpp>
#endif

#include <mutex>
#include <string>
#include <vector>
#include <future>
#include <boost\system\error_code.hpp>

namespace {
    std::mutex image_urls_mutex;
    std::vector<std::string> image_urls;

    bing_xml_request::bing_xml_request(const std::string& server,
        const std::string& path)
        : request_base(server, path)
    {}

    void bing_xml_request::read_content(const boost::system::error_code& error,
        size_t bytes_transferred)
    {
        if (!error) {
            request_base::read_content(error, bytes_transferred);
        } else {
            data << &response;

            size_t pos { 0 };
            for (;;) {
                auto start = data.str().find("<urlBase>", pos) + 9;
                auto end = data.str().find("</urlBase>", start);

                if (start - 9 == std::string::npos ||
                    end == std::string::npos) {
                        break;
                }

                image_urls_mutex.lock();
                image_urls.emplace_back(std::string { data.str(),
                    start, end - start });
                image_urls_mutex.unlock();

                pos = end + 10;
            };
        }
    }

    image_request::image_request(const std::string &server,
        const std::string &path, const std::string &filename)
        : request_base(server, path), filename(filename)
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

int main()
{
    boost::filesystem::create_directories(
        boost::filesystem::path { save_location });

    auto resolution = resolutions[12];

    std::vector<std::future<void>> xml_tasks;

    for (const auto& it : country_codes) {
        xml_tasks.emplace_back(std::async(std::launch::async, [=]() {
            bing_xml_request { "www.bing.com", base_path + it }.run();
        }));
    }

    for (const auto& it : xml_tasks) {
        it.wait();
    }

    std::vector<std::future<void>> image_threads;

    for (const auto& it : image_urls) {
        auto off = it.find_last_of('/');
        auto len = it.find_first_of('_') - off;
        std::string filename = save_location + it.substr(off, len) + resolution;

        if (boost::filesystem::exists(boost::filesystem::path { filename })) {
            continue;
        }

        image_threads.emplace_back(std::async(std::launch::async, [=]() {
            image_request { "www.bing.com", it + resolution, filename }.run();
        }));
    }

    for (const auto& it : image_threads) {
        it.wait();
    }

    return 0;
}
