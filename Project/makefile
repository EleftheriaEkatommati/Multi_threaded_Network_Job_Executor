# CC = g++
# CFLAGS = -Wall -std=c++11
# #INSTALL_DIR = /usr/local/bin

# all: jobCommander jobExecutorServer

# jobCommander: jobCommander.cpp
# 	$(CC) $(CFLAGS) -o jobCommander jobCommander.cpp

# jobExecutorServer: jobExecutorServer.cpp Threads.o
# 	$(CC) $(CFLAGS) -o jobExecutorServer jobExecutorServer.cpp Threads.o -pthread

# Threads.o: Threads.cpp thread_functions.h
# 	$(CC) $(CFLAGS) -c Threads.cpp -pthread

# install: all
# 	cp jobCommander $(INSTALL_DIR)
# 	cp jobExecutorServer $(INSTALL_DIR)

# clean:
# 	rm -f jobCommander jobExecutorServer Threads.o
CC = g++
CFLAGS = -Wall -std=c++11
INSTALL_DIR = /usr/local/bin

# Directories
BIN_DIR = bin
INCLUDE_DIR = include
SRC_DIR = src

# Targets
all: $(BIN_DIR)/jobCommander $(BIN_DIR)/jobExecutorServer $(BIN_DIR)/progDelay

$(BIN_DIR)/jobCommander: $(SRC_DIR)/jobCommander.cpp
	$(CC) $(CFLAGS) -o $@ $<

$(BIN_DIR)/progDelay: $(SRC_DIR)/progDelay.cpp
	$(CC) $(CFLAGS) -o $@ $<

$(BIN_DIR)/jobExecutorServer: $(SRC_DIR)/jobExecutorServer.cpp $(BIN_DIR)/Threads.o
	$(CC) $(CFLAGS) -o $@ $< $(BIN_DIR)/Threads.o -pthread

$(BIN_DIR)/Threads.o: $(SRC_DIR)/Threads.cpp $(INCLUDE_DIR)/thread_functions.h
	$(CC) $(CFLAGS) -c -o $@ $< -pthread

install: all
	cp $(BIN_DIR)/jobCommander $(INSTALL_DIR)
	cp $(BIN_DIR)/jobExecutorServer $(INSTALL_DIR)

clean:
	rm -f $(BIN_DIR)/jobCommander $(BIN_DIR)/jobExecutorServer $(BIN_DIR)/Threads.o $(BIN_DIR)/progDelay
