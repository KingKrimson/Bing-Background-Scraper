/*=============================================================================\
|
|   Copyright (C) 2013 by Christopher Harpum.
|
|   File:       request_base.h
|   Project:    bing background scraper
|   Created:    2013-08-27
|
\=============================================================================*/

#pragma once

#include <string>
#include <boost\asio.hpp>

namespace bbd {
    using boost::asio::ip::tcp;

    class request_base {
    public:
        request_base(const std::string&, const std::string&);

        virtual void read_content(const boost::system::error_code&, size_t);
        virtual void read_headers(const boost::system::error_code&, size_t);
        virtual void read_status(const boost::system::error_code&, size_t);
        virtual void write_request(const boost::system::error_code&, size_t);
        virtual void connect(const boost::system::error_code&);
        virtual void resolve(const boost::system::error_code&,
            tcp::resolver::iterator);

        virtual void run();

    protected:
        boost::asio::io_service io_service;
        tcp::socket sock;
        tcp::resolver resolver;
        boost::asio::streambuf request;
        boost::asio::streambuf response;
    };
}