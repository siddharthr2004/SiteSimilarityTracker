#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <ostream>
#include <cstdlib>
#include <unistd.h> 
#include <curl/curl.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <memory>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "param.hpp" 
#include <sstream> 

class findSiteInfo {
    public: 
    std::string URL;

    findSiteInfo(std::string inputURL) : URL(inputURL) {}

    std::unique_ptr<param> getInfo(void) {
        CURLU *h = curl_url();
        CURLUcode rh = curl_url_set(h, CURLUPART_URL, URL.c_str(), 0);
        if (rh != CURLUE_OK) {
            fprintf(stderr, "Error setting URL\n");
        }
        char *curlHost = nullptr;
        char *path = nullptr;
        char *port = nullptr;
        char *query = nullptr;
        char *scheme = nullptr;
        
        std::unique_ptr<param> foundVals = std::make_unique<param>();
        
        rh = curl_url_get(h, CURLUPART_HOST, &curlHost, 0);
        
        if (rh == CURLUE_OK) {
            foundVals->host = (std::string)(curlHost);
        } else {
            foundVals->host = "No explicit host found";
        }
        rh = curl_url_get(h, CURLUPART_PATH, &path, 0);
        if (rh == CURLUE_OK) {
            foundVals->path = (std::string)(path);
        } else {
            foundVals->path = "No explicit path found";
        }
        
        rh = curl_url_get(h, CURLUPART_PORT, &port, 0);
        if (rh == CURLUE_OK) {
            foundVals->port = (std::string)(port);
        } else {
            foundVals->port = "No explicit port found";
        }
        //parameters which are queried/int
        rh = curl_url_get(h, CURLUPART_QUERY, &query, 0);
        if (rh == CURLUE_OK) {
            foundVals->query = (std::string)(query);
        } else {
            foundVals->query = "No explicit query found";
        }
        //Http vs Https prootocl differentiation
        rh = curl_url_get(h, CURLUPART_SCHEME, &scheme, 0);
        if (rh == CURLUE_OK) {
            foundVals->scheme = (std::string)(scheme);
        } else {
            foundVals->scheme = "No explicit scheme found";
        }
        return foundVals;
    } 
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
    for (auto& val: matrix) {
        std::cout<<val->path<<" This is the host here"<<std::endl;
    }

    return 0;
}