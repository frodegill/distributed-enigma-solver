SERVER = enigma-solver-server

############# Main application #################
all:	$(SERVER)
.PHONY: all

# source files
DEBUG_INFO = YES
SERVER_SOURCES = server.c
SERVER_OBJECTS = $(SERVER_SOURCES:.c=.o)

######## compiler- and linker settings #########
CXXFLAGS = -I/usr/include -W -Wall -Werror -pipe
LIBSFLAGS = -L/usr/lib
ifdef DEBUG_INFO
 CXXFLAGS += -g
else
 CXXFLAGS += -O3
endif

%.o: %.c
	g++ $(CXXFLAGS) -o $@ -c $<

############# Main application #################
$(SERVER):	$(SERVER_OBJECTS)
	g++ -o $@ $(SERVER_OBJECTS) $(LIBSFLAGS)

################### Clean ######################
clean:
	find . -name '*~' -delete
	rm -f $(SERVER) $(SERVER_OBJECTS)
