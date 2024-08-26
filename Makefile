CC = gcc
CFLAGS = -g -Wall -Wextra -Werror -pedantic

BUILD_DIR = build

$(shell mkdir -p $(BUILD_DIR))

SERVER_BINARY = server
CLIENT_BINARY = client

all: $(BUILD_DIR)/$(SERVER_BINARY) $(BUILD_DIR)/$(CLIENT_BINARY)

$(BUILD_DIR)/$(CLIENT_BINARY): $(BUILD_DIR)/client.o
	$(CC) $(CFLAGS) $^ -o $@
	rm -f $(BUILD_DIR)/*.o

$(BUILD_DIR)/client.o: ./src/client.c
	$(CC) $(CFLAGS) -c $^ -o $@

$(BUILD_DIR)/$(SERVER_BINARY): $(BUILD_DIR)/server.o
	$(CC) $(CFLAGS) $^ -o $@
	rm -f $(BUILD_DIR)/*.o

$(BUILD_DIR)/server.o: ./src/server.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: clean
