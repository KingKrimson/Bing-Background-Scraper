#pragma once

#include "pch.h"
#include "request_base.h"

namespace {
    const auto resolutions = std::vector<std::string>
    { "_240x240.jpg", "_320x320.jpg", "_240x480.jpg", "_480x800.jpg",
    "_720x1280.jpg", "_768x1280.jpg", "_640x480.jpg", "_800x600.jpg",
    "_1024x768.jpg", "_1280x720.jpg", "_1280x768.jpg", "_1366x768.jpg",
    "_1920x1200.jpg" };

    const auto country_codes = std::vector<std::string>
    { "de-DE", "en-AU", "ja-JP", "pt-BR", "en-CA", "en-GB", "fr-FR", "zh-CN",
    "en-US", "en-IN" };

    /* all unique market codes, will add to search if they get unique images
    const auto country_codes = std::vector<std::string>
    { "ar-xa", "de-DE", "pl-PL", "en-xa", "zh-HK", "pt-PT", "es-AR", "en-IN",
    "en-PH", "en-AU", "en-ww", "ru-RU", "de-AT", "en-IE", "en-SG", "nl-BE",
    "it-IT", "es-ES", "fr-BE", "ja-JP", "sv-SE", "pt-BR", "ko-KR", "fr-CH",
    "en-CA", "es-xl", "de-CH", "fr-CA", "es-MX", "zh-TW", "es-CL", "nl-NL",
    "tr-TR", "da-DK", "en-NZ", "en-GB", "fi-FI", "nb-NO", "fr-FR", "zh-CN",
    "es-US", "en-US" };
    */

    const auto save_location = std::string { "D:/Pictures/Bing Backgrounds" };
    const auto base_path = std::string { "/HPImageArchive.aspx?n=8&mkt=" };
    const auto jpeg_magic_number = std::string { "\xff\xd8\xff\xe0" };

    class xml_request : public bbd::request_base {
    public:
        xml_request(const std::string &, const std::string &);

        void read_content(const boost::system::error_code &, size_t);
    };

    class image_request : public bbd::request_base {
    public:
        image_request(const std::string &, const std::string &,
            const std::string &);

        void read_content(const boost::system::error_code &, size_t);
        void read_status(const boost::system::error_code &, size_t);

    private:
        std::string filename;
    };
}