
CC=g++ 
CFLAGS=-Wall
TARGET=voiceCommand
SRC=./src
BIN=bin
INCLUDE=
LIBS= -ljson_spirit -lcurl -lboost_regex
TMP=tmp

.PHONY: all
all: $(TARGET)
	@cp $(SRC)/scripts/* $(BIN)/
	@echo "Build successful"

$(TARGET):$(TARGET).o
		$(CC) $(CFLAGS) $^ -o $(BIN)/$@ $(LIBS)
 
$(TARGET).o: $(SRC)/$(TARGET).cpp
			$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	 @echo "Cleaning files..."
	 @rm -f *.o
