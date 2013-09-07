#pragma once

#include "pch.h"

namespace bbd {
    using boost::asio::ip::tcp;

    class request_base {
    public:
        request_base(boost::asio::io_service&, const std::string&,
            const std::string&);

        virtual void read_content(const boost::system::error_code&, size_t);
        virtual void read_headers(const boost::system::error_code&, size_t);
        virtual void status_line(const boost::system::error_code&, size_t);
        virtual void write_request(const boost::system::error_code&, size_t);
        virtual void connect(const boost::system::error_code&);
        virtual void resolve(const boost::system::error_code&,
            tcp::resolver::iterator);

    protected:
        boost::asio::streambuf request;
        boost::asio::streambuf response;
        tcp::socket sock;
        tcp::resolver resolver;
        std::stringstream data;
    };
}