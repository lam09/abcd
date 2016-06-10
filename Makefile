JAVA_HOME=/usr/lib/jvm/java-1.7.0-openjdk-amd64
XLINE_HOME=xlinedevkit_x64
CLASS_NAME=com_ag_game_card
PACKAGE=com/ag/game
CXXFLAGS=-shared -pedantic -fPIC -Wall -O2 -DX10_LINUX_BUILD
INCLUDES=-I$(JAVA_HOME)/include -I$(XLINE_HOME)/include -I$(XLINE_HOME)/samples
UNLOCKIO=unlockio_x64.o


.PHONY: all

all: $(CLASS_NAME).so class

$(CLASS_NAME).so: $(CLASS_NAME).cpp $(CLASS_NAME).h Makefile
	g++ $(CXXFLAGS) $(INCLUDES) -o $@ $(CLASS_NAME).cpp $(XLINE_HOME)/samples/authenticate_linux.cpp $(UNLOCKIO) -lfflyusb_x64 -pthread
class: Makefile
#	$(MAKE) -C `echo $(CLASS_NAME) | sed -r 's/(.+)_.+/\1/' | sed -r 's/_/\//g'`
	$(MAKE) -C $(PACKAGE)


