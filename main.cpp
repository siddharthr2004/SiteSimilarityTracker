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
     // Write callback function for CURL
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

    void extractHTMLData(std::unique_ptr<param::IDInfo>& idInfo, std::unique_ptr<param::contentSignatureInfo>& contentSignature, GumboNode *root) {
        if (root == nullptr) {
            return;
        }
        //test
        std::cout << "root pointer: " << root << std::endl;
        //only extract information pertaining to the textual portion of the nodes if it is a 
        //gumbo_node_text
        if (root->type == GUMBO_NODE_TEXT || root->type == GUMBO_NODE_COMMENT) {
            std::string data = getContext(root);
            //the match you are to use and which you can reuse within each regex capture
            std::smatch match;
            //test
            std::cout<<"This is the text here: "<<data<<std::endl;
            //go into checking script vals, googleAnalytics, FBPixel, gglTagMngr, tealium, ggladremrkting, 
            //mcsftAduet and jsLibs are all here
            //regex patterns for google analytics:
            std::regex googlePattern("G- [A-Z0-9]{6,}");
            std::regex universalPattern("UA-\\d+-\\d+");
            if (std::regex_search(data, match, googlePattern)) {
                //test
                std::cout<<match.str()<<std::endl;
                idInfo->googleAnalytics = match.str();
            } else if (std::regex_search(data, match, universalPattern)) {
                //test
                std::cout<<"google pattern 1 match: "<<match.str()<<std::endl;
                idInfo->googleAnalytics = match.str();
            } else {
                idInfo->googleAnalytics = "No match found";
            }
            //regex patterns for facebook analytics
            std::regex facebookPattern("fbq\\(['\"]init['\"],\\s*['\"]([0-9]{15,16})['\"]");
            if (std::regex_search(data, match, facebookPattern)) {
                //test
                std::cout<<"This is the facebookPattern check"<<match.str()<<std::endl;
                idInfo->facebookPixel = match.str();
            } else {
                idInfo->facebookPixel = "No match found";
            }
            //google tag manager analytics 
            std::regex googleTagManager("GTM-([A-Z0-9]{7})");
            if (std::regex_search(data, match, googleTagManager)) {
                //test
                std::cout<<"Google tag manager"<<match.str()<<std::endl;
                idInfo->googleTagManager = match.str();
            } else {
                idInfo->googleTagManager = "no match found";
                }
            //tealium analytics
            std::regex tealium("tags\\.tiqcdn\\.com/utag/([^/]+)/[^/]+/[^/]+");
            if (std::regex_search(data, match, tealium)) {
                //test
                std::cout<<"tealium check"<<match.str()<<std::endl;
                idInfo->tealium = match[1].str();
            } else {
                idInfo->tealium = "no match found";
            }
            std::regex microsoftAd("ti:\\s*([A-Z0-9]+)");
            if (std::regex_search(data, match, microsoftAd)) {
                //test
                std::cout<<"microsoft ad check"<<match.str()<<std::endl;
                idInfo->microsoftAdUET = match[1].str();
            } else {
                idInfo->microsoftAdUET = "no match found";
            }
        
            //checking copywrite info all portions of the div
            std::regex copywright("(?:©|\bCopyright\b)\\s*(\\d{4})(?:-(\\d{4}))?\\s*(.*?)$");
            if (std::regex_search(data, match, copywright)) {
                //test
                std::cout<<"copywright check"<<match.str()<<std::endl;
                contentSignature->copywrite = match[3].str();
            } else {
                contentSignature->copywrite = "no match found";
            }
            //find the team members within the match here
            std::regex teamCommentPattern("<!--\\s*(Developed by|Powered by|Team|Created by|Maintained by|Built by)\\s*(.*?)\\s*-->");
            if (std::regex_search(data, match, teamCommentPattern)) {
                //test
                std::cout<<"teamCommentPattern check: "<<match.str()<<std::endl;
                contentSignature->htmlComments = match[2].str();
            } else {
                contentSignature->htmlComments = "No match found";
            }
            return;
        } 
        else if (root->type == GUMBO_NODE_ELEMENT) {
            GumboVector* children = &root->v.element.children;
            if (children->length == 0) {
                //test
                std::cout<<"Length of children here is 0 and will return"<<std::endl;
                return;
            } else {
                for (int i=0; i<children->length; ++i) {
                    std::cout<<"Length of children here is "<<children->length<<std::endl;
                    GumboNode* child = (GumboNode*) children->data[i];
                    std::cout<<i<<"th child aded in from parent node out of "<<children->length<<" children"<<std::endl;
                    extractHTMLData(idInfo, contentSignature, child);
                }
            }
        } else {
            std::cout<<"Error assigning what gumbo node is, moving onto next value..."<<std::endl;
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
    void getHTMLInfo(std::unique_ptr<param::IDInfo>& idInfo, std::unique_ptr<param::contentSignatureInfo>& contentSignature) {
        //test for getting html:
        std::ifstream file("check1.txt");
        std::stringstream html_stream;
        html_stream << file.rdbuf();
        std::cout<<html_stream<<std::endl;
        std::string html = html_stream.str();
        std::cout<<"This is the entire html file"<<html<<std::endl;
        //below is commented out for testing
        //std::string html = parseDocumentation();
        //create gumbo node
        GumboOutput* output = gumbo_parse(html.c_str());
        GumboNode* root = output->root;
        //extract html info
        extractHTMLData(idInfo, contentSignature, root);

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
        //create the main params struct and move the rest of the values in
        auto foundVals = std::make_unique<param>();
        foundVals->URLInformation = std::move(this->getURLInfo(rh, h));
        foundVals->fingerInformation = std::move(this->getFinerPrintInfo(rh, h));
        foundVals->IDInformation = std::move(IdInfo);
        foundVals->infrasturcutreInformation = std::move(this->getInfraInfo(rh, h));
        foundVals->contentSignatureInformation = std::move(contentSignatureInfo);
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