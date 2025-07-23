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
    runConcurrently(std::vector<std::string> URLs) : shutdownFlag(false) {
        ssize_t amountCPUs = std::max(1u, std::thread::hardware_concurrency()-1);
        //Current thread:
        std::cout<<"This is the main thread ID: "<<std::this_thread::get_id()<<std::endl;
        for (int i=0; i<amountCPUs; ++i) {
            threadPool.emplace_back([this]() {
                try {
                    std::cout << "Thread " << std::this_thread::get_id() << " has been spawned and will sleep now" << std::endl;
                    
                    while (true) {
                        std::function<std::unique_ptr<param>()> task; {
                        std::ostringstream oss;

                        std::unique_lock<std::mutex>lock(taskLock);
                        std::cout<<"thread "<<std::this_thread::get_id()<<" has been spawned and will sleep now"<<std::endl;
                        cv.wait(lock, [this]() { 
                            return !taskQueue.empty() || shutdownFlag; 
                        });
                        //Printing out first "locked" piece
                        oss <<"thread "<<std::this_thread::get_id()<<"Has entered the locked segment";
                        std::string toAddOne = oss.str(); 
                        std::cout<<toAddOne<<std::endl;
                        if (shutdownFlag && taskQueue.empty()) {
                            return;
                        }
                        oss.clear();
                        //printing out second "locked" piece
                        oss <<"Taking task out in thread: "<<std::this_thread::get_id;
                        std::string toAddTwo = oss.str();
                        std::cout<<toAddTwo<<std::endl;
                                    
                        if (taskQueue.empty() == false) {
                            task = std::move(taskQueue.front());
                            taskQueue.pop();
                        } else {
                            std::cout<<"Empty taskQueue"<<std::endl;
                        }
                                    
                        oss.clear();
                        //Printing out third "locked" piece
                        oss <<"unlocking thread: "<<std::this_thread::get_id();
                        std::string toAddThree = oss.str();
                        std::cout<<toAddThree<<std::endl;
                        }
                        // Outside the lock scope, execute task
                        std::unique_ptr<param> val;
                        if (task) {
                            try {
                                std::cout << "Running task from thread" << std::this_thread::get_id() << "now..." << std::endl;
                                val = task();
                                std::cout << "Task completed in " << std::this_thread::get_id() << std::endl;
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
                        std::cout<<std::this_thread::get_id()<<" is now writing to matrix now"<<std::endl;
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
        std::cout<<std::this_thread::get_id()<<" threas is now adding processes in"<<std::endl;
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
                        std::cout << std::this_thread::get_id() << " added the URL" << std::endl;
                        return s->getInfo();
                    } catch (const std::exception& e) {
                        std::cerr << "Exception in lambda: " << e.what() << std::endl;
                        return nullptr;
                    } catch (...) {
                        std::cerr << "Unknown exception in lambda" << std::endl;
                        return nullptr;
                    }
                });
                cv.notify_one(); // Notify only one waiting thread
            }
        }
        std::cout<<"WILL START RUNNING THE PROCESSES NOW. This is the size of the taskQueue "<<taskQueue.size()<<std::endl;
        std::cout<<"this the size of the threadPool "<<threadPool.size()<<std::endl;
        cv.notify_all();
        std::cout<<std::this_thread::get_id()<<" thread is now waiting for other values to finish"<<std::endl;
    } 
    void shutdown() {
        {
            std::unique_lock<std::mutex> lock(taskLock);
            shutdownFlag = true;
        }
        cv.notify_all();
        
        // Join all threads
        for (auto& thread : threadPool) {
            if (thread.joinable()) {
                thread.join();
            }
        }
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
    return 0;
}