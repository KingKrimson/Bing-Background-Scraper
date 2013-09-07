#pragma once

#include "pch.h"
#include "request_base.h"

namespace {
    const auto standard_definition = std::string { "_1366x768.jpg" };
    const auto high_definition = std::string { "_1920x1200.jpg" };
    const auto save_location = std::string { "D:/Pictures/Bing Backgrounds" };
    const auto base_path = std::string { "/HPImageArchive.aspx?n=8&mkt=" };
    const auto jpeg_magic_number = std::string { "\xff\xd8\xff\xe0" };

    const auto country_codes = std::vector<std::string>
    {{ "ar-xa", "de-DE", "pl-PL", "en-xa", "zh-HK", "pt-PT", "es-AR", "en-IN",
    "en-PH", "en-AU", "en-ww", "ru-RU", "de-AT", "en-IE", "en-SG", "nl-BE",
    "it-IT", "es-ES", "fr-BE", "ja-JP", "sv-SE", "pt-BR", "ko-KR", "fr-CH",
    "en-CA", "es-xl", "de-CH", "fr-CA", "es-MX", "zh-TW", "es-CL", "nl-NL",
    "tr-TR", "da-DK", "en-NZ", "en-GB", "fi-FI", "nb-NO", "fr-FR", "zh-CN",
    "es-US", "en-US" }};

    class xml_request : public bbd::request_base {
    public:
        xml_request(boost::asio::io_service &, const std::string &,
            const std::string &);

        void write_request(const boost::system::error_code &, size_t);
    };

    class image_request : public bbd::request_base {
    public:
        image_request(boost::asio::io_service &, const std::string &,
            const std::string &, const std::string &);

        void read_content(const boost::system::error_code &, size_t);
        void status_line(const boost::system::error_code &, size_t);

    private:
        std::string filename;
    };
}