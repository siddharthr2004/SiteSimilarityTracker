#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <ostream>
#include <cstdlib>
#include <unistd.h> 
#include <curl/curl.h>
#include <openssl/ssl.h>
#include <memory>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "param.hpp" 
using namespace std;

class runConcurrently {
    public:
    std::vector<std::thread>threadPool;
    std::queue<std::function<void()>>taskQueue;
    std::vector<param> matrix;
    std::mutex taskLock;
    std::mutex writeLock;
    std::condition_variable cv;

    runConcurrently(std::vector<std::string> URLs) {
        for (std::string URL : URLs) {
            std::unique_ptr<findSiteInfo> toAdd = std::make_unique<findSiteInfo>(URL);
            taskQueue.push([toAdd = std::move(toAdd)]() -> void {
                toAdd->getInfo();
            });
        }
    }
    void runPrograms(void) {
        ssize_t amountCPUs = std::max(1u, std::thread::hardware_concurrency()-1);
    }

};
class findSiteInfo {
    public: 
    std::string URL;

    findSiteInfo(std::string& inputURL) : URL(inputURL) {}

    void getInfo(void) {
        CURLU *h = curl_url();
        CURLUcode rh = curl_url_set(h, CURLUPART_URL, URL.c_str(), 0);
            if (rh != CURLUE_OK) {
                fprintf(stderr, "Error setting URL: %s\n");
            }
            std::unique_ptr<param> foundVals = std::make_unique<param>();
        }
    };

int main(int argc, char* argv[]) {
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
        }
        std::string line;
        while(std::getline(inputfile, line)) {
            URLs.push_back(line);
        }
    }
    std::unique_ptr<findSiteInfo> Info = std::make_unique<findSiteInfo>(URLs);
    return 0;
}