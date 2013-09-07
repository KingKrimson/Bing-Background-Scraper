#include "include\pch.h"
#include "include\request_base.h"

namespace bbd {
    using boost::asio::ip::tcp;

    request_base::request_base(boost::asio::io_service& io_service,
        const std::string& server, const std::string& path) :
        resolver(io_service), sock(io_service)
    {
        std::ostream request_stream { &request };
        request_stream << "GET " << path << " HTTP/1.0\r\n";
        request_stream << "Host: " << server << "\r\n\r\n";

        tcp::resolver::query query(server, "http");
        resolver.async_resolve(query,
            std::bind(&request_base::resolve, this,
            std::placeholders::_1, std::placeholders::_2));
    }

    void request_base::read_content(const boost::system::error_code& ec,
        size_t bytes_transferred)
    {
        if (!ec) {
            response.consume(bytes_transferred);

            boost::asio::async_read(sock, response,
                boost::asio::transfer_at_least(1),
                std::bind(&request_base::read_content, this,
                std::placeholders::_1, std::placeholders::_2));
        }
    }

    void request_base::read_headers(const boost::system::error_code& ec,
        size_t bytes_transferred)
    {
        if (!ec) {
            response.consume(bytes_transferred);

            boost::asio::async_read(sock, response,
                boost::asio::transfer_at_least(1),
                std::bind(&request_base::read_content, this,
                std::placeholders::_1, std::placeholders::_2));
        }
    }

    void request_base::status_line(const boost::system::error_code& ec,
        size_t bytes_transferred)
    {
        if (!ec) {
            response.consume(bytes_transferred);

            boost::asio::async_read_until(sock, response, "\r\n\r\n",
                std::bind(&request_base::read_headers, this,
                std::placeholders::_1, std::placeholders::_2));
        }
    }

    void request_base::write_request(const boost::system::error_code& ec,
        size_t bytes_transferred)
    {
        if (!ec) {
            response.consume(bytes_transferred);

            boost::asio::async_read_until(sock, response, "\r\n",
                std::bind(&request_base::status_line, this,
                std::placeholders::_1, std::placeholders::_2));
        }
    }

    void request_base::connect(const boost::system::error_code& ec)
    {
        if (!ec) {
            boost::asio::async_write(sock, request,
                std::bind(&request_base::write_request, this,
                std::placeholders::_1, std::placeholders::_2));
        }
    }

    void request_base::resolve(const boost::system::error_code& ec,
        tcp::resolver::iterator it)
    {
        if (!ec) {
            sock.async_connect(*it,
                std::bind(&request_base::connect, this,
                std::placeholders::_1));
        }
    }
}