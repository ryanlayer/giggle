BIN=bin
OBJ=obj
LIBD=lib

all: 
	@mkdir -p $(OBJ)
	@mkdir -p $(BIN)
	@mkdir -p $(LIBD)
	cd src; $(MAKE)

server:
	@mkdir -p $(OBJ)
	@mkdir -p $(BIN)
	cd src; $(MAKE) server

clean:
	rm -rf $(BIN)/*
	rm -rf $(OBJ)/*
	rm -rf $(LIBD)/*
