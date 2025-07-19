CXX := g++
CXXFLAGS := -std=c++20 -Wall -O2
CXX_TARGET := executable
CXX_SRC := main.cpp

CURL_PREFIX := /opt/homebrew/opt/curl-8.7.1
SSL_PREFIX := /opt/homebrew/opt/openssl
INCLUDES := -I$(SSL_PREFIX)/include \
            -I$(CURL_PREFIX)/include
LIBS := -L$(SSL_PREFIX)/lib \
        -L$(CURL_PREFIX)/lib \
        -Wl,-rpath,$(SSL_PREFIX)/lib \
        -Wl,-rpath,$(CURL_PREFIX)/lib \
        -lcurl -lssl -lcrypto

all: $(CXX_TARGET)

$(CXX_TARGET): $(CXX_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -g -o $@ $^ $(LIBS)

clean:
	rm -f $(CXX_TARGET)



