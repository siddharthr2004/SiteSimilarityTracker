#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <ostream>
#include <cstdlib>
#include <unistd.h>
#include <curl/curl.h>
#include <curl/urlapi.h> 
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <memory>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "param.hpp" 
#include <sstream> 
#include <botan/x509cert.h>
#include <botan/tls_client.h>
#include <maxminddb.h>
#include <gumbo.h> 
#include <regex>
#include <chrono>

//class for sorting the info which is pulled from each of the sites. This portion is done AFTER pulling 
//all information from before
class sortInfo {
    public:
    sortInfo(std::vector<std::unique_ptr<param>> inputMap) : inputVector(std::move(inputMap)) {}

    void populateMap(void) {

        size_t length = inputVector.size();
        for (int i=0; i<length; ++i) {
            //get URL val
            std::string url = inputVector[i]->URL;
            //packaging out URLInfo sub-struct
            std::vector<std::string*> allVals = {
                //packaging for URLInformation sub-struct
                &inputVector[i]->URLInformation->host, &inputVector[i]->URLInformation->path, 
                &inputVector[i]->URLInformation->port,&inputVector[i]->URLInformation->query, 
                &inputVector[i]->URLInformation->scheme,
                //packing out fingerPrintInfo sub-struct
                &inputVector[i]->fingerInformation->CerSAN,
                &inputVector[i]->fingerInformation->CerorganizationName, &inputVector[i]->fingerInformation->CercommonName,
                &inputVector[i]->fingerInformation->CercertificateAuthoritySite, 
                &inputVector[i]->fingerInformation->CervalidityPeriod,&inputVector[i]->fingerInformation->CerType,
                //packging out IDInfo sub-struct
                &inputVector[i]->IDInformation->googleAnalytics,
                &inputVector[i]->IDInformation->facebookPixel, &inputVector[i]->IDInformation->googleTagManager,
                &inputVector[i]->IDInformation->tealium, &inputVector[i]->IDInformation->microsoftAdUET, 
                //packaging for infrastructureInfo sub-struct
                &inputVector[i]->infrasturcutreInformation->serverHeader,
                &inputVector[i]->infrasturcutreInformation->xPower, &inputVector[i]->infrasturcutreInformation->CFRayHead,
                &inputVector[i]->infrasturcutreInformation->xServedByHeader, &inputVector[i]->infrasturcutreInformation->IpAddress,
                &inputVector[i]->infrasturcutreInformation->ASN, &inputVector[i]->infrasturcutreInformation->ISP, 
                &inputVector[i]->infrasturcutreInformation->CDN, &inputVector[i]->infrasturcutreInformation->HTTPVersionl,
                &inputVector[i]->infrasturcutreInformation->supportedMethods, &inputVector[i]->infrasturcutreInformation->cacheControl,
                //packaging for contentSignatureInformation sub-struct
                &inputVector[i]->contentSignatureInformation->copywrite, 
                &inputVector[i]->contentSignatureInformation->htmlComments, &inputVector[i]->contentSignatureInformation->fontSources,
                &inputVector[i]->contentSignatureInformation->jsLibs,
            };
            std::vector<std::string> mainKeys = {
                "host", "path", "port", "query", "scheme", "CerSAN", "CerorganizationName", "CercommonName", "CercertificateAuthoritySite",
                "CervalidityPeriod", "CerType", "googleAnalytics", "facebookPixel", "googleTagManager", "tealium", "microsoftAdUET", 
                "serverHeader", "xPower", "CFRayHead", "xServedByHeader", "IpAddress", "ASN", "ISP", "CDN", "HTTPVersionl", "supportedMethods",
                "cacheControl", "copywrite", "htmlComments", "fontSources", "jsLibs", 
            };
            if (mainKeys.size() != allVals.size()) {
                std::cerr<<"ERROR: AllVals and mainKeys are different sizes!"<<std::endl;
            }
            for (int i=0; i<mainKeys.size(); ++i) {
                if (mainMap.count(mainKeys[i]) > 0)  {
                    //first retireve outer map
                    std::string val = *allVals[i];
                    std::map<std::string, std::vector<std::string>> *outerMap = &mainMap[mainKeys[i]];
                    std::vector<std::string> *innerVector = &((*outerMap)[val]);
                    innerVector->push_back(url);
                } else {
                    std::vector<std::string> vectorToAdd = {url};
                    std::map<std::string, std::vector<std::string>> mapToAdd = {{*allVals[i], vectorToAdd}};
                    //map which the keyword will be mapped onto 
                    mainMap[mainKeys[i]] = mapToAdd;
                }
            }
        }
    }
    private:
    std::map<std::string, std::map<std::string, std::vector<std::string>>> mainMap;
    std::vector<std::unique_ptr<param>> inputVector;
};

class findSiteInfo {
    public: 
    findSiteInfo(std::string inputURL) : URL(inputURL) {}

    bool openFile() {
        htmlFile = fopen("htmlOutput.txt", "wb");
        if (!htmlFile) {
            return false;
        }
        return true;
    }
     // Write callback function for CURL, fed solely into parseDocumentation file 
    static size_t writeCallback(char *ptr, size_t size, size_t nmemb, void *userData) {
        size_t realSize = size * nmemb;
        std::string *html = static_cast<std::string*>(userData);
        html->append(ptr, realSize);
        return realSize;
    }

    //Method used to parse through the html document
    std::string parseDocumentation () {
        std::string htmlContent;
        CURL *curl = curl_easy_init();
        if (!curl) {
            std::cerr<<"Unable to instantiate the curl url handle"<<std::endl;
        } 
        curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &htmlContent);
        CURLcode result = curl_easy_perform(curl);
        if (result != CURLE_OK) {
            std::cerr<<"error pulling data from html file"<<std::endl;
        }
        return htmlContent;
    }

    std::string getContext(GumboNode* root) {
        if (root->type == GUMBO_NODE_TEXT) {
            return std::string(root->v.text.text);
        }
    }

    //test value containing the number of recursive depth added in for clarity into argument. Remove when going into 
    //main section 
    void extractHTMLData(std::unique_ptr<param::IDInfo>& idInfo, std::unique_ptr<param::contentSignatureInfo>& contentSignature, GumboNode *root, int val) {
        if (root == nullptr) {
            return;
        }
        //Information extracted only if gumbo text or gumo comment
        if (root->type == GUMBO_NODE_TEXT || root->type == GUMBO_NODE_COMMENT) {
            //get context for root
            std::string data = getContext(root);
            //the match you are to use and which you can reuse within each regex capture
            std::smatch match;
            //fields storing values to populate ID information
            std::vector<std::string*> fieldsIdInfo {
                &idInfo->googleAnalytics,
                &idInfo->facebookPixel,
                &idInfo->googleTagManager,
                &idInfo->tealium,
                &idInfo->microsoftAdUET,
               
            };
            //fields storing content to populate content signatue information
            std::vector<std::string*> fieldsContentSignature {
                &contentSignature->copywrite,
                &contentSignature->htmlComments
            };
            //capture match region, if -1 the entire area is captured
            int captureRegion [7] = {-1, -1, -1, 1, 1, 3, 2};
            //mapping out regex patterns
            std::vector<std::regex> regexVals {
                std::regex ("G- [A-Z0-9]{6,}|UA-\\d+-\\d+"), 
                std::regex ("fbq\\(['\"]init['\"],\\s*['\"]([0-9]{15,16})['\"]"),
                std::regex ("GTM-([A-Z0-9]{7})"),
                std::regex ("tags\\.tiqcdn\\.com/utag/([^/]+)/[^/]+/[^/]+"),
                std::regex ("ti:\\s*([A-Z0-9]+)"),
                std::regex("(?:©|Copyright)\\s*(\\d{4})(?:-(\\d{4}))?\\s*(.+)"),
                std::regex ("<!--\\s*(Developed by|Powered by|Team|Created by|Maintained by|Built by)\\s*(.*?)\\s*-->")
            };
            //iterat through each value, find their matching region if it exists
            for (int i=0; i<7; ++i) {
                if (std::regex_search(data, match, regexVals[i])) {
                    if (i<5) {
                        if (captureRegion[i] == -1) {
                            *fieldsIdInfo[i] = match.str();
                        } else {
                            *fieldsIdInfo[i] = match[captureRegion[i]].str();
                        }
                    } else {
                        if (captureRegion[i] == -1) {
                            *fieldsContentSignature[i-5] = match.str();
                        } else {
                            *fieldsContentSignature[i-5] = match[captureRegion[i]].str();
                        }
                    }
                }
            } 
            return;
        } 
        //if gumbo node element, continue recursing 
        else if (root->type == GUMBO_NODE_ELEMENT) {
            GumboVector* children = &root->v.element.children;
            if (children->length == 0) {
                return;
            } else {
                for (int i=0; i<children->length; ++i) {
                    GumboNode* child = (GumboNode*) children->data[i];
                    extractHTMLData(idInfo, contentSignature, child, val+1);
                }
            }
        //or return back to prev stack call
        } else {
            return;
        }
    }

    std::unique_ptr<param::URLInfo> getURLInfo(CURLUcode rh, CURLU *h) {
        //struct to return
        auto URLInfo = std::make_unique<param::URLInfo>();
        //five non-names pointers holding answer to return to dict
        std::vector<char*> charPointers(5, nullptr);
        //curlCommands 
        std::vector<CURLUPart> curlCommands = {
            CURLUPART_HOST, CURLUPART_PATH, CURLUPART_PORT, CURLUPART_QUERY, CURLUPART_SCHEME
        };
        //struct vals as pointers
        std::vector<std::string*> stringPointers = {
            &URLInfo->host, &URLInfo->path, &URLInfo->port, &URLInfo->query, &URLInfo->scheme
        };
        //quit out if arrays are not same size
        if (curlCommands.size() != stringPointers.size()) {
            std::cerr<<"Irregular array shaping, quiting now"<<std::endl;
            exit(1);
        }
        //iterate through and do operatons on vals
        for (int i=0; i<5; ++i) {
            rh = curl_url_get(h, curlCommands[i], &charPointers[i], 0);
            if (rh == CURLUE_OK) {
                *(stringPointers[i]) = (std::string)(charPointers[i]);
            } else {
                *(stringPointers[i]) = "no explicit host found";
            }
        }
        return URLInfo;
    }
    std::unique_ptr<param::fingerPrintInfo> getFinerPrintInfo(CURLUcode rh, CURLU *h) {
        
    }
    //method created gumbo node and then parses through the DOM
    void getHTMLInfo(std::unique_ptr<param::IDInfo>& idInfo, std::unique_ptr<param::contentSignatureInfo>& contentSignature) {
        std::string htmlFile = parseDocumentation();
        GumboOutput* output = gumbo_parse(htmlFile.c_str());
        GumboNode* root = output->root;
        extractHTMLData(idInfo, contentSignature, root, 0);

    }
    std::unique_ptr<param::infrastructureInfo> getInfraInfo(CURLUcode rh, CURLU *h) {
        //TODO:
    }
    std::unique_ptr<param::contentSignatureInfo> getContentSignature(CURLUcode rh, CURLU *h) {
        //TODO:
    }

    std::unique_ptr<param> getInfo(void) {
        CURLU *h = curl_url();
        CURLUcode rh = curl_url_set(h, CURLUPART_URL, URL.c_str(), 0);
        if (!openFile()) {
            std::cerr<<"Error instantiating the output html file"<<std::endl;
        }
        if (rh != CURLUE_OK) {
            fprintf(stderr, "Error setting URL\n");
        }
        //create the pointers for the structs which need html parsing
        auto IdInfo = std::make_unique<param::IDInfo>();
        auto contentSignatureInfo = std::make_unique<param::contentSignatureInfo>();
        //TEST: commented out
        //getHTMLInfo(IdInfo, contentSignatureInfo);
        //test
        auto foundVals = std::make_unique<param>();
        foundVals->URLInformation = std::move(this->getURLInfo(rh, h));
        //TEST
        std::cout<<"finished retieving values"<<std::endl;
        foundVals->contentSignatureInformation = std::move(contentSignatureInfo);
        foundVals->IDInformation = std::move(IdInfo);
        foundVals->URL = URL;
        //commented out until full implementation put in
        //foundVals->fingerInformation = std::move(this->getFinerPrintInfo(rh, h));
        //foundVals->infrasturcutreInformation = std::move(this->getInfraInfo(rh, h))
        return foundVals;
    } 
    private:
    std::string URL;
    FILE* htmlFile;

};

class runConcurrently {
    public:
    runConcurrently(std::vector<std::string> URLs) : shutdownFlag(false) {
        ssize_t amountCPUs = std::max(1u, std::thread::hardware_concurrency()-1);
        
        for (int i=0; i<amountCPUs; ++i) {
            threadPool.emplace_back([this]() {
                try {
                    while (true) {
                        std::function<std::unique_ptr<param>()> task; {

                        std::unique_lock<std::mutex>lock(taskLock);
                        cv.wait(lock, [this]() { 
                            return !taskQueue.empty() || shutdownFlag; 
                        });
                        
                        if (shutdownFlag && taskQueue.empty()) {
                            return;
                        }
                       
                        if (taskQueue.empty() == false) {
                            task = std::move(taskQueue.front());
                            taskQueue.pop();
                        } else {
                            std::cout<<"Empty taskQueue"<<std::endl;
                        }
                        }
                        std::unique_ptr<param> val;
                        if (task) {
                            try {
                                val = task();
                            } catch (const std::exception& e) {
                                std::cerr << "Exception in task execution: " << e.what() << std::endl;
                            } catch (...) {
                                std::cerr << "Unknown exception in task execution" << std::endl;
                            }
                        } else {
                            std::cerr << "WARNING: Null task detected in worker thread!" << std::endl;
                        }
                        {
                        std::unique_lock<std::mutex>writeGuard(writeLock); 
                        matrix.emplace_back(std::move(val));
                        }
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Exception in worker thread: " << e.what() << std::endl;
                } catch (...) {
                    std::cerr << "Unknown exception in worker thread" << std::endl;
                }
            });
        } 
        usleep(5000);
        for (std::string& URL : URLs) {
            auto toAdd = std::make_shared<findSiteInfo>(URL);
            {
                std::unique_lock<std::mutex> lock(taskLock);
                taskQueue.push([s = std::move(toAdd)] () -> std::unique_ptr<param> {
                    try {
                        if (!s) {
                            std::cerr << "ERROR: Null pointer in lambda" << std::endl;
                            return nullptr;
                        }
                        return s->getInfo();
                    } catch (const std::exception& e) {
                        std::cerr << "Exception in lambda: " << e.what() << std::endl;
                        return nullptr;
                    } catch (...) {
                        std::cerr << "Unknown exception in lambda" << std::endl;
                        return nullptr;
                    }
                });
                cv.notify_one(); 
            }
        }
        cv.notify_all();
    } 
    void shutdown() {
        {
            std::unique_lock<std::mutex> lock(taskLock);
            shutdownFlag = true;
        }
        cv.notify_all();
        for (auto& thread : threadPool) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }
    std::vector<std::unique_ptr<param>> returnResults() {
        return std::move(matrix);
    }

    private:
    std::vector<std::thread>threadPool;
    std::queue<std::function<std::unique_ptr<param>()>>taskQueue;
    std::vector<std::unique_ptr<param>> matrix;
    std::mutex taskLock;
    std::mutex writeLock;
    std::condition_variable cv;
    bool shutdownFlag; 

};

int main(int argc, char* argv[]) {
    std::set_terminate([] {
        std::cerr << " Unhandled exception — calling std::terminate()." << std::endl;
        std::abort();
    });

    std::vector<std::string> URLs;
    if (argc < 3) {
        std::cout<<"Not enough commands were inputted, aborting..."<< std::endl;
        return 1;
    }
    if (std::string (argv[1]) == "-f") {
        std::ifstream inputfile;
        inputfile.open(std::string(argv[2]), std::ios::in);
        //text
        if (!inputfile.is_open()) {
            std::cout<<"Error opening file"<<std::endl;
            exit(1);
        }
        std::string line;
        while(std::getline(inputfile, line)) {
            URLs.push_back(line);
        }
    }
    auto Info = std::make_unique<runConcurrently>(URLs);
    Info->shutdown();
    std::vector<std::unique_ptr<param>> matrix = Info->returnResults();
    return 0;
}