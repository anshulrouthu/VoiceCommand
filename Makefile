
CC=g++ 
CFLAGS=-Wall
TARGET=voiceCommand
SRC=./source
SAMPLES=./samples/*.cpp
BIN=bin
INCLUDE=
LIBS= -ljson_spirit -lcurl -lboost_regex -lasound -lopenal -lFLAC
TMP=tmp

SRCS=$(wildcard ./samples/*.c)
OBJS=$(SRCS:.c=.o)

.PHONY: all
all: $(TARGET) sample
	@cp $(SRC)/scripts/* $(BIN)/
	@echo "Build successful"

#$(TARGET):$(TARGET).o
#		$(CC) $(CFLAGS) $^ -o $(BIN)/$@ $(LIBS)

$(TARGET): $(patsubst %.cpp, %.o, $(wildcard source/*.cpp))
		$(CC) $(CFLAGS) $^ -o $(BIN)/$@ $(LIBS)

.PHONY: sample
sample: sample-record

sample-record:samples/sample-record.o
	          $(CC) $(CFLAGS) $^ -o $(BIN)/$@ $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	 @echo "Cleaning files..."
	 @rm -f source/*.o samples/*.o
