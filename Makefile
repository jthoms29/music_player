CC = gcc
CFLAGS = -g
CPPFLAGS = -Wall -pedantic -Wextra

TAGLIB = /usr/local/lib/
#libtag_c.a
TAGINCLUDE = /usr/local/include/taglib/


OBJ_DIR=build/$(PLATFORM)/obj
LIB_DIR=build/$(PLATFORM)/lib
BIN_DIR=build/$(PLATFORM)/bin

BINARIES = music_player

all: $(BINARIES)

clean: 
	rm -rf build $(BINARIES)


# directories
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(LIB_DIR):
	mkdir -p $(LIB_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)


$(OBJ_DIR)/main.o : main.c | $(OBJ_DIR)
	$(CC) -c $(CFLAGS) $(CPPFLAGS) main.c -I. -I$(TAGINCLUDE) -o $(OBJ_DIR)/main.o

$(BIN_DIR)/music_player : $(OBJ_DIR)/main.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -L$(TAGLIB) $(OBJ_DIR)/main.o -ldl -lpthread -lm -ltag_c -ltag -lstdc++ -lz -o \
	$(BIN_DIR)/music_player

music_player : $(BIN_DIR)/music_player
	ln -sf $(BIN_DIR)/music_player music_player

