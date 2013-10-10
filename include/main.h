/*=============================================================================\
|
|   Copyright (C) 2013 by Christopher Harpum.
|
|   File:       main.h
|   Project:    bing background scraper
|   Created:    2013-08-27
|
\=============================================================================*/

#pragma once

#include "request_base.h"
#include <string>
#include <sstream>
#include <vector>
#include <array>

namespace boost {
    namespace system {
        class error_code;
    }
}

namespace {
    // all _known_ resolutions
    const std::array<std::string, 13> resolutions { {
        "_240x240.jpg", "_320x320.jpg", "_240x480.jpg", "_480x800.jpg",
        "_720x1280.jpg", "_768x1280.jpg", "_640x480.jpg", "_800x600.jpg",
        "_1024x768.jpg", "_1280x720.jpg", "_1280x768.jpg", "_1366x768.jpg",
        "_1920x1200.jpg"
    } };

    const std::array<std::string, 10> country_codes { {
        "de-DE", "en-AU", "ja-JP", "pt-BR", "en-CA", "en-GB", "fr-FR", "zh-CN",
        "en-US", "en-IN"
    } };

    /* all unique market codes, will add to search if they get unique images
    const auto country_codes = std::vector<std::string>
    { "ar-xa", "de-DE", "pl-PL", "en-xa", "zh-HK", "pt-PT", "es-AR", "en-IN",
    "en-PH", "en-AU", "en-ww", "ru-RU", "de-AT", "en-IE", "en-SG", "nl-BE",
    "it-IT", "es-ES", "fr-BE", "ja-JP", "sv-SE", "pt-BR", "ko-KR", "fr-CH",
    "en-CA", "es-xl", "de-CH", "fr-CA", "es-MX", "zh-TW", "es-CL", "nl-NL",
    "tr-TR", "da-DK", "en-NZ", "en-GB", "fi-FI", "nb-NO", "fr-FR", "zh-CN",
    "es-US", "en-US" };
    */

    const std::string website { "www.bing.com" };
    const std::string base_path { "/HPImageArchive.aspx?n=8&mkt=" };
    const std::string jpeg_magic_number { "\xff\xd8\xff\xe0" };

    class bing_xml_request final : public bbd::request_base {
    public:
        bing_xml_request(std::string, std::string);

        void read_content(const boost::system::error_code&, size_t) override;

    private:
        std::stringstream data;
    };

    class image_request final : public bbd::request_base {
    public:
        image_request(std::string, std::string, std::string);

        void read_content(const boost::system::error_code&, size_t) override;
        void read_status(const boost::system::error_code&, size_t) override;

    private:
        std::stringstream data;
        std::string filename;
    };
}