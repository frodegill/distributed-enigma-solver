CLIENT = enigma-solver-client

############# Main application #################
all:	$(CLIENT)
.PHONY: all

# source files
DEBUG_INFO = YES
CLIENT_SOURCES = client.c keysetting.c plugboard.c common.c
CLIENT_OBJECTS = $(CLIENT_SOURCES:.c=.o)

######## compiler- and linker settings #########
CXXFLAGS = -I/usr/include -W -Wall -Werror -pipe
LIBSFLAGS = -L/usr/lib
ifdef DEBUG_INFO
 CXXFLAGS += -g -DDEBUG
else
 CXXFLAGS += -O3
endif

%.o: %.c
	g++ $(CXXFLAGS) -o $@ -c $<

############# Main application #################
$(CLIENT):	$(CLIENT_OBJECTS)
	g++ -o $@ $(CLIENT_OBJECTS) $(LIBSFLAGS)

################### Clean ######################
clean:
	find . -name '*~' -delete
	rm -f $(CLIENT) $(CLIENT_OBJECTS)
