CXX = g++
CXXSTD = -std=c++11
CXXFLAGS = $(CXXSTD)

objects = main.o confirm.o handshake-ss.o handshake-pass.o handshake-socks5.o handshake-private.o listener.o worker.o

.PHONY : all clean debug release

debug: CXXFLAGS += -DDEBUG -g
debug: socks5-server

release: CXXFLAGS += -O3
release: socks5-server

$(objects): %.o: %.cc
	$(CXX) -c $(CXXFLAGS) $< -o $@

socks5-server: $(objects)
	$(CXX) $(CXXFLAGS) -o $@ $(objects) 

all: $(objects) socks5-server 

clean:
	rm -rf *.o
	if [ -f socks5-server ]; then rm socks5-server; fi
