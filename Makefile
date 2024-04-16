OBJ_DIR := lib
BIN_DIR := bin

all: $(BIN_DIR)/client $(BIN_DIR)/server

# Add instructions for building to client and server executables

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# create object files for dependencies and then link them to create the executable

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)/* $(BIN_DIR)/*