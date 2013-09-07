#include "include\pch.h"

#include "include\main.h"
#include "include\request_base.h"

namespace {
    std::mutex image_urls_mutex;
    std::vector<std::string> image_urls;

    xml_request::xml_request(boost::asio::io_service& io_service,
        const std::string& server, const std::string& path) :
        request_base(io_service, server, path)
    {
    }

    void xml_request::write_request(const boost::system::error_code& ec,
        size_t bytes_transferred)
    {
        if (!ec) {
            boost::asio::async_read(sock, response,
                boost::asio::transfer_at_least(1),
                std::bind(&xml_request::write_request, this,
                std::placeholders::_1, std::placeholders::_2));
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

    image_request::image_request(boost::asio::io_service& io_service,
        const std::string &server, const std::string &path,
        const std::string &filename) :
        request_base(io_service, server, path), filename(filename)
    {
    }

    void image_request::read_content(const boost::system::error_code& ec,
        size_t bytes_transferred)
    {
        if (!ec) {
            boost::asio::async_read(sock, response,
                boost::asio::transfer_at_least(1),
                std::bind(&image_request::read_content, this,
                std::placeholders::_1, std::placeholders::_2));
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

    void image_request::status_line(const boost::system::error_code& ec,
        size_t bytes_transferred)
    {
        if (!ec) {
            std::istream response_stream(&response);
            std::string http_version;
            response_stream >> http_version;
            unsigned int status_code;
            response_stream >> status_code;

            if (status_code != 200) {
                return;
            }

            request_base::status_line(ec, bytes_transferred);
        }
    }
}

int main()
{
    boost::filesystem::create_directories(
        boost::filesystem::path { save_location });

    auto definition = high_definition;

    std::vector<std::future<void>> xml_tasks;

    for (const auto& it : country_codes) {
        xml_tasks.emplace_back(std::async(std::launch::async, [=]() {
            boost::asio::io_service io_service;
            xml_request requester(io_service, "www.bing.com", base_path + it);
            io_service.run();
        }));
    }

    for (const auto &it : xml_tasks) {
        it.wait();
    }

    std::sort(image_urls.begin(), image_urls.end());
    image_urls.erase(std::unique(image_urls.begin(), image_urls.end()),
        image_urls.end());

    std::vector<std::future<void>> image_threads;

    for (const auto& it : image_urls) {
        auto off = it.find_last_of('/');
        auto len = it.find_first_of('_') - off;
        auto filename = save_location + it.substr(off, len) + definition;

        if (boost::filesystem::exists(
            boost::filesystem::path { filename })) {
                continue;
        }

        image_threads.emplace_back(std::async(std::launch::async, [=]() {
            boost::asio::io_service io_service;
            image_request requester(io_service, "www.bing.com", it + definition,
                filename);
            io_service.run();
        }));
    }

    for (const auto& it : image_threads) {
        it.wait();
    }

        return 0;
}
