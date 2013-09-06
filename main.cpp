#if defined(_WIN32)
#define _WIN32_WINNT 0x0501
#include <filesystem>
namespace boost {
    namespace filesystem = std::tr2::sys;
}
#else
#include <boost/filesystem.hpp>
#endif

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <functional>

#include <boost/asio.hpp>

namespace {
    using boost::asio::ip::tcp;

    std::vector<std::string> image_urls;

    const auto save_location = std::string { "D:\\Pictures\\Bing Backgrounds" };
    const auto base_path = std::string { "/HPImageArchive.aspx?n=8&mkt=" };
    const auto jpeg_magic_number = std::string { "\xff\xd8\xff\xe0" };

    const auto country_codes = std::vector<std::string>
    {{ "ar-xa", "de-DE", "pl-PL", "en-xa", "zh-HK", "pt-PT", "es-AR", "en-IN",
    "en-PH", "en-AU", "en-ww", "ru-RU", "de-AT", "en-IE", "en-SG", "nl-BE",
    "it-IT", "es-ES", "fr-BE", "ja-JP", "sv-SE", "pt-BR", "ko-KR", "fr-CH",
    "en-CA", "es-xl", "de-CH", "fr-CA", "es-MX", "zh-TW", "es-CL", "nl-NL",
    "tr-TR", "da-DK", "en-NZ", "en-GB", "fi-FI", "nb-NO", "fr-FR", "zh-CN",
    "es-US", "en-US" }};

    class request_base {
    public:
        request_base(boost::asio::io_service& io_service,
            const std::string& server, const std::string& path) :
            resolver(io_service), sock(io_service)
        {
            std::ostream request_stream { &request };
            request_stream << "GET " << path << " HTTP/1.0\r\n";
            request_stream << "Host: " << server << "\r\n\r\n";

            tcp::resolver::query query(server, "http");
            resolver.async_resolve(query,
                std::bind(&request_base::handle_resolve, this,
                std::placeholders::_1, std::placeholders::_2));
        }

        virtual void handle_read_content(const boost::system::error_code& ec,
            size_t bytes_transferred)
        {
            if (!ec) {
                response.consume(bytes_transferred);

                boost::asio::async_read(sock, response,
                    boost::asio::transfer_at_least(1),
                    std::bind(&request_base::handle_read_content, this,
                    std::placeholders::_1, std::placeholders::_2));
            }
        }

        virtual void handle_read_headers(const boost::system::error_code& ec,
            size_t bytes_transferred)
        {
            if (!ec) {
                response.consume(bytes_transferred);

                boost::asio::async_read(sock, response,
                    boost::asio::transfer_at_least(1),
                    std::bind(&request_base::handle_read_content, this,
                    std::placeholders::_1, std::placeholders::_2));
            }
        }

        virtual void handle_status_line(const boost::system::error_code& ec,
            size_t bytes_transferred)
        {
            if (!ec) {
                response.consume(bytes_transferred);

                boost::asio::async_read_until(sock, response, "\r\n\r\n",
                    std::bind(&request_base::handle_read_headers, this,
                    std::placeholders::_1, std::placeholders::_2));
            }
        }

        virtual void write_request_handler(const boost::system::error_code& ec,
            size_t bytes_transferred)
        {
            if (!ec) {
                response.consume(bytes_transferred);

                boost::asio::async_read_until(sock, response, "\r\n",
                    std::bind(&request_base::handle_status_line, this,
                    std::placeholders::_1, std::placeholders::_2));
            }
        }

        virtual void handle_connect(const boost::system::error_code& ec)
        {
            if (!ec) {
                boost::asio::async_write(sock, request,
                    std::bind(&request_base::write_request_handler, this,
                    std::placeholders::_1, std::placeholders::_2));
            }
        }

        virtual void handle_resolve(const boost::system::error_code& ec,
            tcp::resolver::iterator it)
        {
            if (!ec) {
                sock.async_connect(*it,
                    std::bind(&request_base::handle_connect, this,
                    std::placeholders::_1));
            }
        }

    protected:
        boost::asio::streambuf request;
        boost::asio::streambuf response;
        tcp::socket sock;
        tcp::resolver resolver;
        std::stringstream data;

    };

    class xml_request : public request_base {
    public:
        xml_request(boost::asio::io_service& io_service,
            const std::string& server, const std::string& path) :
            request_base(io_service, server, path)
        {
        }

        virtual void write_request_handler(const boost::system::error_code& ec,
            size_t bytes_transferred)
        {
            if (!ec) {
                boost::asio::async_read(sock, response,
                    boost::asio::transfer_at_least(1),
                    std::bind(&xml_request::write_request_handler, this,
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

                    image_urls.emplace_back(std::string { data.str(),
                        start, end - start });

                    pos = end + 10;
                };
            }
        }
    };

    class image_request : public request_base {
    public:
        image_request(boost::asio::io_service& io_service,
            const std::string& server, const std::string& path) :
            request_base(io_service, server, path), filename(path)
        {
        }

        virtual void handle_read_content(const boost::system::error_code& ec,
            size_t bytes_transferred)
        {
            if (!ec) {
                boost::asio::async_read(sock, response,
                    boost::asio::transfer_at_least(1),
                    std::bind(&image_request::handle_read_content, this,
                    std::placeholders::_1, std::placeholders::_2));
            } else {
                data << &response;

                if (data.str().substr(0, 4) != jpeg_magic_number) {
                    return;
                }

                auto pos = filename.find_last_of('/');
                filename = save_location + filename.substr(pos);

                std::cout << filename << std::endl;

                auto out = std::ofstream { filename, std::ios::binary };
                if (out.is_open()) {
                    out << data.str();
                    out.close();
                }
            }
        }

        void handle_read_status_line(const boost::system::error_code& ec,
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

                request_base::handle_status_line(ec, bytes_transferred);
            }
        }


    private:
        std::string filename;

    };
}

int main()
{
    for (const auto& it : country_codes) {
        boost::asio::io_service io_service;
        xml_request requester(io_service, "www.bing.com", base_path + it);
        io_service.run();
    }

    std::sort(image_urls.begin(), image_urls.end());
    image_urls.erase(std::unique(image_urls.begin(), image_urls.end()),
        image_urls.end());

    for (const auto& it : image_urls) {
        auto filename = it + "_1920x1200.jpg";

        auto pos = filename.find_last_of('/');
        if (boost::filesystem::exists(
            boost::filesystem::path { save_location + filename.substr(pos) })) {
                std::cout << "file already exists" << std::endl;
                continue;
        }

        std::cout << filename << std::endl;

        boost::asio::io_service io_service;
        image_request requester(io_service, "www.bing.com", filename);
        io_service.run();
    }

    return 0;
}
