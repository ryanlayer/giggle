BIN=bin
OBJ=obj

all: 
	@mkdir -p $(OBJ)
	@mkdir -p $(BIN)
	cd src; $(MAKE)

clean:
	rm -rf $(BIN)/*
	rm -rf $(OBJ)/*
