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

//class for sorting the info which is pulled from each of the sites. This portion is done AFTER pulling 
//all information from before
class sortInfo {
    public:
    sortInfo(std::vector<std::unique_ptr<param>> inputMap) : inputVector(std::move(inputMap)) {}

    void populateMap(void) {
        for (const auto& val : inputVector) {
            //first for URLInfo
            if (!val->URLInformation->host.empty() && val->URLInformation->host != "No match found") {
                mainMap["host"][val->URLInformation->host].push_back(val->URL);
            }
            if (!val->URLInformation->path.empty() && val->URLInformation->path != "No match found") {
                mainMap["path"][val->URLInformation->path].push_back(val->URL);
            }
            if (!val->URLInformation->port.empty() && val->URLInformation->port != "No match found") {
                mainMap["port"][val->URLInformation->port].push_back(val->URL);
            }
            if (!val->URLInformation->query.empty() && val->URLInformation->query != "No match found") {
                mainMap["query"][val->URLInformation->query].push_back(val->URL);
            }
            if (!val->URLInformation->scheme.empty() && val->URLInformation->scheme != "No match found") {
                mainMap["scheme"][val->URLInformation->scheme].push_back(val->URL);
            }
            //now fingerPrintInfo
            if (!val->fingerInformation->CerSAN.empty() && val->fingerInformation->CerSAN != "No match found") {
                mainMap["CerSAN"][val->fingerInformation->CerSAN].push_back(val->URL);
            } 
            if (!val->fingerInformation->CerorganizationName.empty() && val->fingerInformation->CerorganizationName != "No match found") {
                mainMap["CerorganizationName"][val->fingerInformation->CerorganizationName].push_back(val->URL);
            } 
            if (!val->fingerInformation->CercommonName.empty() && val->fingerInformation->CercommonName != "No match found") {
                mainMap["CercommonName"][val->fingerInformation->CercommonName].push_back(val->URL);
            }
            if (!val->fingerInformation->CercertificateAuthoritySite.empty() && val->fingerInformation->CercertificateAuthoritySite != "No match found") {
                mainMap["CercertificateAuthoritySite"][val->fingerInformation->CercertificateAuthoritySite].push_back(val->URL);
            }
            if (!val->fingerInformation->CervalidityPeriod.empty() && val->fingerInformation->CervalidityPeriod != "No match found") {
                mainMap["CervalidityPeriod"][val->fingerInformation->CervalidityPeriod].push_back(val->URL);
            }
            if (!val->fingerInformation->CerType.empty() && val->fingerInformation->CerType != "No match found") {
                mainMap["CerType"][val->fingerInformation->CerType].push_back(val->URL);
            }
            //Now infrstructure info
            if (!val->infrasturcutreInformation->serverHeader.empty() && val->infrasturcutreInformation->serverHeader != "No match found") {
                mainMap["serverHeader"][val->infrasturcutreInformation->serverHeader].push_back(val->URL);
            } 
            if (!val->infrasturcutreInformation->xPower.empty() && val->infrasturcutreInformation->xPower != "No match found") {
                mainMap["xPower"][val->infrasturcutreInformation->xPower].push_back(val->URL);
            } 
            if (!val->infrasturcutreInformation->CFRayHead.empty() && val->infrasturcutreInformation->CFRayHead != "No match found") {
                mainMap["CFRayHead"][val->infrasturcutreInformation->CFRayHead].push_back(val->URL);
            } 
            if (!val->infrasturcutreInformation->xServedByHeader.empty() && val->infrasturcutreInformation->xServedByHeader != "No match found") {
                mainMap["xServedByHeader"][val->infrasturcutreInformation->xServedByHeader].push_back(val->URL);
            }
            if (!val->infrasturcutreInformation->IpAddress.empty() && val->infrasturcutreInformation->IpAddress != "No match found") {
                mainMap["IpAddress"][val->infrasturcutreInformation->IpAddress].push_back(val->URL);
            }  
            if (!val->infrasturcutreInformation->ASN.empty() && val->infrasturcutreInformation->ASN != "No match found") {
                mainMap["ASN"][val->infrasturcutreInformation->ASN].push_back(val->URL);
            }  
            if (!val->infrasturcutreInformation->ISP.empty() && val->infrasturcutreInformation->ISP != "No match found") {
                mainMap["ISP"][val->infrasturcutreInformation->ISP].push_back(val->URL);
            } 
            if (!val->infrasturcutreInformation->CDN.empty() && val->infrasturcutreInformation->CDN != "No match found") {
                mainMap["CDN"][val->infrasturcutreInformation->CDN].push_back(val->URL);
            } 
            if (!val->infrasturcutreInformation->HTTPVersionl.empty() && val->infrasturcutreInformation->HTTPVersionl != "No match found") {
                mainMap["HTTPVersionl"][val->infrasturcutreInformation->HTTPVersionl].push_back(val->URL);
            }  
            if (!val->infrasturcutreInformation->supportedMethods.empty() && val->infrasturcutreInformation->supportedMethods != "No match found") {
                mainMap["supportedMethods"][val->infrasturcutreInformation->supportedMethods].push_back(val->URL);
            } 
            if (!val->infrasturcutreInformation->cacheControl.empty() && val->infrasturcutreInformation->cacheControl != "No match found") {
                mainMap["cacheControl"][val->infrasturcutreInformation->cacheControl].push_back(val->URL);
            } 
            //now contentSignatureInfo
            if (!val->contentSignatureInformation->copywrite.empty() && val->contentSignatureInformation->copywrite != "No match found") {
                mainMap["copywrite"][val->contentSignatureInformation->copywrite].push_back(val->URL);
            } 
            if (!val->contentSignatureInformation->htmlComments.empty() && val->contentSignatureInformation->htmlComments != "No match found") {
                mainMap["htmlComments"][val->contentSignatureInformation->htmlComments].push_back(val->URL);
            } 
            if (!val->contentSignatureInformation->fontSources.empty() && val->contentSignatureInformation->fontSources != "No match found") {
                mainMap["fontSources"][val->contentSignatureInformation->fontSources].push_back(val->URL);
            }
            if (!val->contentSignatureInformation->jsLibs.empty() && val->contentSignatureInformation->jsLibs != "No match found") {
                mainMap["jsLibs"][val->contentSignatureInformation->jsLibs].push_back(val->URL);
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
        char *curlHost = nullptr;
        char *path = nullptr;
        char *port = nullptr;
        char *query = nullptr;
        char *scheme = nullptr;
        
        auto URLInfo = std::make_unique<param::URLInfo>();
        
        rh = curl_url_get(h, CURLUPART_HOST, &curlHost, 0);
        
        if (rh == CURLUE_OK) {
            URLInfo->host = (std::string)(curlHost);
        } else {
            URLInfo->host = "No explicit host found";
        }
        rh = curl_url_get(h, CURLUPART_PATH, &path, 0);
        if (rh == CURLUE_OK) {
            URLInfo->path = (std::string)(path);
        } else {
            URLInfo->path = "No explicit path found";
        }
        
        rh = curl_url_get(h, CURLUPART_PORT, &port, 0);
        if (rh == CURLUE_OK) {
            URLInfo->port = (std::string)(port);
        } else {
            URLInfo->port = "No explicit port found";
        }
        //parameters which are queried/int
        rh = curl_url_get(h, CURLUPART_QUERY, &query, 0);
        if (rh == CURLUE_OK) {
            URLInfo->query = (std::string)(query);
        } else {
            URLInfo->query = "No explicit query found";
        }
        //Http vs Https prootocl differentiation
        rh = curl_url_get(h, CURLUPART_SCHEME, &scheme, 0);
        if (rh == CURLUE_OK) {
            URLInfo->scheme = (std::string)(scheme);
        } else {
            URLInfo->scheme = "No explicit scheme found";
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
        getHTMLInfo(IdInfo, contentSignatureInfo);
        //test
        auto foundVals = std::make_unique<param>();
        foundVals->URLInformation = std::move(this->getURLInfo(rh, h));
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