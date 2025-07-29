#pragma once
#include <string>

struct param {
        struct URLInfo{
            std::string host;
            std::string path;
            std::string port;
            std::string query;
            std::string scheme;
        };
        struct fingerPrintInfo {
           std::string CerSAN;
           std::string CerorganizationName;
           std::string CercommonName;
           std::string CercertificateAuthoritySite;
           std::string CervalidityPeriod;
           std::string CerType;
        };
        //Instantiations of the different values within the struct
        std::unique_ptr<URLInfo> URLInformation = std::make_unique<URLInfo>();
        std::unique_ptr<fingerPrintInfo> fingerInformation = std::make_unique<fingerPrintInfo>();

    };