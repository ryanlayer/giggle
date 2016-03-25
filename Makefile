BIN=bin
OBJ=obj

all: 
	@mkdir -p $(OBJ)
	@mkdir -p $(BIN)
	cd src; $(MAKE)

server:
	@mkdir -p $(OBJ)
	@mkdir -p $(BIN)
	cd src; $(MAKE) server

clean:
	rm -rf $(BIN)/*
	rm -rf $(OBJ)/*
