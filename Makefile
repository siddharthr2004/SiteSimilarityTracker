CXX := g++
CXXFLAGS := -std=c++20 -Wall -O2
CXX_TARGET := executable
CXX_SRC := main.cpp

CURL_PREFIX := /opt/homebrew/opt/curl-8.7.1
SSL_PREFIX := /opt/homebrew/opt/openssl
BOTAN_PREFIX := /opt/homebrew/Cellar/botan@2/2.19.5
GUMBO_PREFIX := /opt/homebrew/opt/gumbo-parser
MAXMIND_PREFIX := /opt/homebrew/opt/libmaxminddb
INCLUDES := -I$(SSL_PREFIX)/include \
            -I$(CURL_PREFIX)/include \
            -I$(BOTAN_PREFIX)/include/botan-2 \
            -I$(GUMBO_PREFIX)/include \
            -I$(MAXMIND_PREFIX)/include
LIBS := -L$(SSL_PREFIX)/lib \
        -L$(CURL_PREFIX)/lib \
        -L$(BOTAN_PREFIX)/lib \
        -L$(GUMBO_PREFIX)/lib \
        -L$(MAXMIND_PREFIX)/lib \
        -Wl,-rpath,$(SSL_PREFIX)/lib \
        -Wl,-rpath,$(CURL_PREFIX)/lib \
        -Wl,-rpath,$(BOTAN_PREFIX)/lib \
        -Wl,-rpath,$(GUMBO_PREFIX)/lib \
        -Wl,-rpath,$(MAXMIND_PREFIX)/lib \
        -lcurl -lssl -lcrypto -lbotan-2 -lgumbo -lmaxminddb

all: $(CXX_TARGET)

$(CXX_TARGET): $(CXX_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -g -o $@ $^ $(LIBS)

clean:
	rm -f $(CXX_TARGET)



