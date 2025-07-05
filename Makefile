CXX := g++
CXXFLAGS := -std=c++20 -Wall -O2
CXX_TARGET := executable
CXX_SRC := main.cpp

CURL_PREFIX := $(HOME)/curl-openssl
INCLUDES := -I$(CURL_PREFIX)/include
LIBS := -L$(CURL_PREFIX)/lib \
        -L$(OPENSSL_PREFIX)/lib \
        -Wl,-rpath,$(OPENSSL_PREFIX)/lib \
        -lcurl -lssl -lcrypto

all: $(CXX_TARGET)

$(CXX_TARGET): $(CXX_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

clean:
	rm -f $(CXX_TARGET)



