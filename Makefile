OBJ_DIR := lib
BIN_DIR := bin

all: $(BIN_DIR)/server

# Add instructions for building to client and server executables
$(BIN_DIR)/server : $(OBJ_DIR)/server.o $(OBJ_DIR)/socket.o $(OBJ_DIR)/socketserver.o $(OBJ_DIR)/Blockable.o | $(BIN_DIR)
	g++ -o $(BIN_DIR)/server $(OBJ_DIR)/server.o $(OBJ_DIR)/socket.o $(OBJ_DIR)/socketserver.o $(OBJ_DIR)/Blockable.o -pthread -l rt -std=c++11

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# create object files for dependencies and then link them to create the executable

$(OBJ_DIR)/server.o : server/server.cpp include/socketserver.h include/Semaphore.h | $(OBJ_DIR)
	g++ -c server/server.cpp -o $(OBJ_DIR)/server.o -std=c++11

$(OBJ_DIR)/Blockable.o : include/Blockable.h src/Blockable.cpp | $(OBJ_DIR)
	g++ -c src/Blockable.cpp -o $(OBJ_DIR)/Blockable.o -std=c++11

$(OBJ_DIR)/socket.o : src/socket.cpp include/socket.h | $(OBJ_DIR)
	g++ -c src/socket.cpp -o $(OBJ_DIR)/socket.o -std=c++11

$(OBJ_DIR)/socketserver.o : src/socketserver.cpp include/socket.h include/socketserver.h | $(OBJ_DIR)
	g++ -c src/socketserver.cpp -o $(OBJ_DIR)/socketserver.o -std=c++11

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)/* $(BIN_DIR)/*