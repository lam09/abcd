XLINE_HOME=../xlinedevkit_x64
CXXFLAGS=-pedantic -fPIC -Wall -O2 -DX10_LINUX_BUILD
INCLUDES=-I$(XLINE_HOME)/include -I$(XLINE_HOME)/samples
LIBS=unlockio_x64.o $(XLINE_HOME)/samples/obj/authenticate_linux.o

unlock_x64: unlock.cpp Makefile
	g++ $(CXXFLAGS) $(INCLUDES) $(LIBS) -o $@ unlock.cpp  -lfflyusb_x64 -pthread


