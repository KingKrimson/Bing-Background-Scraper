// main.cpp

#pragma comment(lib, "winhttp.lib")

extern "C" {
#include <Windows.h>
#include <winhttp.h>
}

#include <string>
#include <fstream>
#include <iostream>
#include <array>
#include <vector>
#include <algorithm>
#include <filesystem>

namespace {
    const auto save_location = std::string {"D:\\Pictures\\Bing Backgrounds"};
    const auto base_path = std::string {"/HPImageArchive.aspx?n=8&mkt="};
    const auto jpeg_magic_number = std::string {"\xff\xd8\xff\xe0"};

    const auto country_codes = std::array<std::string, 48> {{
        "ar-xa", "de-DE", "pl-PL", "en-xa", "zh-HK", "pt-PT", "es-AR", "en-IN",
        "en-PH", "en-AU", "en-ww", "ru-RU", "de-AT", "en-IE", "en-SG", "nl-BE",
        "it-IT", "es-ES", "fr-BE", "ja-JP", "sv-SE", "pt-BR", "ko-KR", "fr-CH",
        "en-CA", "es-xl", "de-CH", "fr-CA", "es-MX", "zh-TW", "es-CL", "nl-NL",
        "tr-TR", "da-DK", "en-NZ", "en-GB", "fi-FI", "nb-NO", "fr-FR", "zh-CN",
        "es-US" //,en-US
    }};

    class requester {
    public:
        requester(std::wstring);
        ~requester();
        auto request(std::wstring)->std::string;
        auto request(std::string)->std::string;

    private:
        HINTERNET hSession;
        HINTERNET hConnect;
        HINTERNET hRequest;
    };

    requester::requester(std::wstring url) : hRequest(NULL)
    {
        hSession = WinHttpOpen(L"requester/1.0",
                WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME,
                WINHTTP_NO_PROXY_BYPASS, NULL);

        if (hSession) {
            hConnect = WinHttpConnect(hSession, url.c_str(),
                    INTERNET_DEFAULT_HTTP_PORT, 0);
        }
    }

    requester::~requester()
    {
        if (hConnect) {
            WinHttpCloseHandle(hConnect);
        }

        if (hSession) {
            WinHttpCloseHandle(hSession);
        }
    }

    auto requester::request(std::wstring query) -> std::string
    {
        DWORD dwSize = 0;
        DWORD dwDownloaded = 0;
        LPSTR pszOutBuffer;
        BOOL bResults = FALSE;
        std::string buffer;

        if (hConnect) {
            hRequest = WinHttpOpenRequest(hConnect, L"GET", query.c_str(),
                    NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
                    NULL);
        }

        if (hRequest) {
            bResults = WinHttpSendRequest(hRequest,
                    WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA,
                    0, 0, 0);
        }

        if (bResults) {
            bResults = WinHttpReceiveResponse(hRequest, NULL);
        }

        if (bResults) {
            do {
                dwSize = 0;
                if (WinHttpQueryDataAvailable(hRequest, &dwSize) == 0) {
                    printf("Error %u in WinHttpQueryDataAvailable.\n",
                            GetLastError());
                }

                pszOutBuffer = new char[dwSize + 1];
                if (!pszOutBuffer) {
                    printf("Out of memory\n");
                    break;
                } else {
                    ZeroMemory(pszOutBuffer, dwSize + 1);
                    if (!WinHttpReadData(hRequest, (LPVOID) pszOutBuffer,
                            dwSize, &dwDownloaded)) {
                        printf("Error %u in WinHttpReadData.\n",
                                GetLastError());
                    } else {
                        buffer.append(std::string(pszOutBuffer, dwSize));
                    }
                }
                delete[] pszOutBuffer;
            } while (dwSize);
        }

        if (!bResults) {
            printf("Error %d has occurred.\n", GetLastError());
        }

        if (hRequest) {
            WinHttpCloseHandle(hRequest);
        }

        return buffer;
    }

    auto requester::request(std::string query) -> std::string
    {
        std::wstring temp(query.begin(), query.end());
        return this->request(temp);
    }
}

auto main() -> int
{
    requester bing_requester(L"www.bing.com");
    std::vector<std::string> image_urls;

    // retrieve all image addresses from the server
    for (const auto it : country_codes) {
        auto web_data = bing_requester.request(base_path + it);

        size_t pos {0};
        for (;;) {
            auto start = web_data.find("<urlBase>", pos) + 9;
            auto end = web_data.find("</urlBase>", start);

            if (start - 9 == std::string::npos || end == std::string::npos) {
                break;
            }

            image_urls.emplace_back(std::string(web_data, start, end - start));

            pos = end + 10;
        }
    }

    // erase any duplicate images
    std::sort(image_urls.begin(), image_urls.end());
    image_urls.erase(std::unique(image_urls.begin(), image_urls.end()),
            image_urls.end());

    for (const auto it : image_urls) {
        auto filename = std::string {it + "_1920x1200.jpg"};
        auto image_data = bing_requester.request(filename);
        if (image_data.substr(0, 4) != jpeg_magic_number) {
            continue;
        }

        // truncate useless filename characters
        auto pos = filename.find_last_of('/');
        filename = save_location + filename.substr(pos);

        // check if file already exists
        if (std::tr2::sys::exists(std::tr2::sys::path {filename})) {
            continue;
        }

        // attempt to save file
        auto out = std::ofstream {filename, std::ios::binary};
        if (out.is_open()) {
            out << image_data;
            out.close();
        } else {
            std::cout << "Failed to save \"" << filename << "\"" << std::endl;
        }

        std::cout << filename << std::endl;
    }

    return 0;
}

#if 0
        using std::chrono::system_clock;
        tm time_info;
        time_t current_time = system_clock::to_time_t(system_clock::now());
        localtime_s(&time_info, &current_time);

        std::ostringstream filename;
        filename << image_dir << std::setfill('0') << std::setw(2) <<
            time_info.tm_mday << "-" << std::setw(2) <<
            (time_info.tm_mon + 1) << "-" << (time_info.tm_year + 1900) <<
            ".jpg";

        std::string image_data = bing_request(image_address);
        std::ofstream out(filename.str(), std::ios::binary);
        out << image_data;
        out.close();

        out.open(image_dir + "image.last");
        out << image_address;
        out.close();

        SystemParametersInfo(SPI_SETDESKWALLPAPER, 0,
                             (PVOID) filename.str().c_str(), SPIF_UPDATEINIFILE);
    }

    std::this_thread::sleep_for(std::chrono::hours(1));
}

return 0;
}
#endif