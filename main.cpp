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
using namespace std;


class findSiteInfo {
    public: 
    std::string URL;

    findSiteInfo(std::string inputURL) : URL(inputURL) {}

    std::unique_ptr<param> getInfo(void) {
        //COMMENTED OUT TO GET CONCURRENCY CORRECT
        /*
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
            std::cout<<"CAME HERE 3"<<std::endl;
        } else {
            fprintf(stderr, "error getting host");
        }
        rh = curl_url_get(h, CURLUPART_PATH, &path, 0);
        if (rh == CURLUE_OK) {
            foundVals->path = (std::string)(path);
        } else {
            fprintf(stderr, "error getting path");
        }
        
        rh = curl_url_get(h, CURLUPART_PORT, &port, 0);
        if (rh == CURLUE_OK) {
            foundVals->port = (std::string)(port);
        } else {
            fprintf(stderr, "error getting port");
        }
            
        rh = curl_url_get(h, CURLUPART_QUERY, &query, 0);
        if (rh == CURLUE_OK) {
            foundVals->query = (std::string)(query);
        } else {
            fprintf(stderr, "error getting query");
        }
        rh = curl_url_get(h, CURLUPART_SCHEME, &scheme, 0);
        if (rh == CURLUE_OK) {
            foundVals->scheme = (std::string)(scheme);
        } else {
            fprintf(stderr, "error getting scheme");
        }
            */
        std::cout<<"I WAS RUN"<<std::endl;
        return std::make_unique<param>();
        } 
};

class runConcurrently {
    public:
    runConcurrently(std::vector<std::string> URLs) : shutdown(false) {
        ssize_t amountCPUs = std::max(1u, std::thread::hardware_concurrency()-1);
        for (int i=0; i<amountCPUs; ++i) {
            try {
                threadPool.emplace_back([this]() -> void {
                    while(true) {
                        try {
                            std::function<std::unique_ptr<param>()> task; {
                            std::unique_lock<std::mutex>lock(taskLock);
                            try {
                                cv.wait(lock, [this]() { 
                                    return !taskQueue.empty() || shutdown; 
                                });
                                std::cout<<"Thread "<<std::this_thread::get_id()<< "has entered the locked segment"<<std::endl;
                                if (shutdown && taskQueue.empty()) {
                                    return;
                                }
                                std::cout<<"Taking task out in thread: "<<std::this_thread::get_id()<<std::endl;
                            } catch (const std::exception &e) {
                                std::cerr<<"error within second inner first part: "<<e.what()<<std::endl;
                            } catch (...) {
                                std::cerr<<"Unknwon error within second inner first part"<<std::endl;
                            }
                            try {
                                task = std::move(taskQueue.front());
                                taskQueue.pop();
                            } catch (const std::exception& e) {
                                std::cerr<<"second inner second part error: "<<e.what()<<std::endl;
                            } catch (...) {
                                std::cerr<<"unknown second inner second part error"<<std::endl;
                            }
                    
                            std::cout<<"unclocking thread: "<<std::this_thread::get_id()<<std::endl;
                        }
                            if (!task) {
                                std::cerr << "WARNING: Null task detected in worker thread!" << std::endl;
                                continue;
                            }
                            std::cout<<"Running task from thread"<<std::this_thread::get_id()<<"now..."<<std::endl;
                            //THIS IS JUST FOR TESTING
                            std::unique_ptr<param>val;
                            try {
                                val = task();
                            } catch (std::exception& e) {
                                std::cerr<<"Error running task: "<<e.what()<<std::endl;
                            } catch (...) {
                                std::cerr<<"Unknown error running task"<<std::endl;
                            }
                            std::cout<<"Task completed in "<<std::this_thread::get_id()<<std::endl;
                            {
                            std::unique_lock<std::mutex>writeGuard(writeLock); 
                            std::cout<<std::this_thread::get_id()<<" is now writing to matrix now"<<std::endl;
                            matrix.emplace_back(std::move(val));
                            }
                        }
                        catch (const std::exception& e) {
                            std::cerr<<"Exception in inner worker thread "<<e.what()<<std::endl;
                        }
                        catch (...) {
                            std::cerr<<"Unknown exception in worker thread"<<std::endl;
                        }
                    }
                });
            } catch (const std::exception& e) {
                std::cerr<<"Exception in outer thread"<<e.what()<<std::endl;
            } catch (...) {
                std::cerr<<"Unknown exception occured"<<std::endl;
            }
        } 
        usleep(5000);
        for (std::string& URL : URLs) {
            auto toAdd = std::make_shared<findSiteInfo>(URL);
            taskQueue.push([s = std::move(toAdd)] () -> std::unique_ptr<param> {
                return s->getInfo();
            });
        }
        std::cout<<"WILL START RUNNING THE PROCESSES NOW"<<std::endl;
        try{
            cv.notify_all();
        } catch (const std::exception& e) {
            std::cerr<<"Error with notifying all vals "<<e.what()<<std::endl;
        }
    }
    

    void Completeshutdown(void) {
        std::unique_lock<std::mutex>finalGate(taskLock);
        shutdown = true;
        cv.notify_all();
    }

    private:
    std::vector<std::thread>threadPool;
    std::queue<std::function<std::unique_ptr<param>()>>taskQueue;
    std::vector<std::unique_ptr<param>> matrix;
    std::mutex taskLock;
    std::mutex writeLock;
    std::condition_variable cv;
    bool shutdown; 

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
    Info->Completeshutdown();
    return 0;
}