#pragma once
#include <string>

//this is for memory management of the http struct
struct memory {
    size_t size;
    char* response;
};

struct param {
        //Information which is pulled which was had to do with URL info 
        struct URLInfo{
            std::string host;
            std::string path;
            std::string port;
            std::string query;
            std::string scheme;
        };
        //certificates are handed over a handshake to secure TLS connection 
        struct fingerPrintInfo {
           //domains under same certificate
           std::string CerSAN;
           //org which handed cert
           std::string CerorganizationName;
           //primary domain cert validates
           std::string CercommonName;
           //which legal site gave the certificate
           std::string CercertificateAuthoritySite;
           std::string CervalidityPeriod;
           //what type of cert (type is dependent on cost, so shows hwo much domain spend on cert)
           std::string CerType;
        };
        //This gets information pertaining to tracking codes from third party sites, ad campaigns, and analytic info
        //HTML parsing needede
        struct IDInfo {
            //tracks user behavior, where same ID means same google analytics account and same owner (orgs use consistent
            //GA accounts). Will also default to universal analytics in the case that google analytics fails
            std::string googleAnalytics;
            //Tracks conversion from fb ads, find retarget audience. Same pixel id means same fb ad account
            std::string facebookPixel;
            //GTM  manages tracking codes, same GTM means managed by same team. 
            std::string googleTagManager;
            //business tag management, expensive enterprise means strong organization (fitness of company)
            std::string tealium;
            //Same UET tag means same microsoft ad account
            std::string microsoftAdUET;
        };
        //This struct stores the internal infra of the URL 
        struct infrastructureInfo {
            //HTTP resp head shows server software, version, and which sites share this infra
            std::string serverHeader;
            //Shows backend tech, stack choices, similarity of backend vers in many domain means similar dev teams
            std::string xPower;
            //shows clodflare server idenitifer, and server with similar cloudfare mean cerntral management
            std::string CFRayHead;
            //shows internal load and server names, expose internals, shared server pools
            std::string xServedByHeader;
            std::string IpAddress;
            //Group of IPs typically under the same network (like an ISP)
            std::string ASN;
            //provider hosting your internet and your infra on their end
            std::string ISP;
            //CDN of content delivery strategy speeds up deliver of web content by caching closer to location
            std::string CDN;
            std::string HTTPVersionl;
            //shows allowed HTTP method (GET/POST/etc.)
            std::string supportedMethods;
            //shows caching policy, including freshness policy and caching infra plans
            std::string cacheControl;
        };
        //HTML parsing needed
        struct contentSignatureInfo {
            //checks for legal ownership, checks if identitcal copywrite across diff brands
            std::string copywrite;
            //get dev team info from HTML comments
            std::string htmlComments;
            //find the design choices of the domain
            std::string fontSources;
            //find the dev stack choices, see identitcal libs when it comes to JS codes in implementatiopn
            std::string jsLibs;
            
        };

        //Instantiations of the different values within the struct
        std::unique_ptr<URLInfo> URLInformation = std::make_unique<URLInfo>();
        std::unique_ptr<fingerPrintInfo> fingerInformation = std::make_unique<fingerPrintInfo>();
        std::unique_ptr<IDInfo> IDInformation = std::make_unique<IDInfo>();
        std::unique_ptr<infrastructureInfo> infrasturcutreInformation = std::make_unique<infrastructureInfo>();
        std::unique_ptr<contentSignatureInfo> contentSignatureInformation = std::make_unique<contentSignatureInfo>();
    };