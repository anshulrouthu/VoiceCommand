
CC=g++ 
CFLAGS=-Wall
TARGET=voiceCommand
SRC=./source
SAMPLES=./samples/*.cpp
BIN=bin
INCLUDE=
LIBS= -ljson_spirit -lcurl -lboost_regex -lasound -lopenal -lFLAC
TMP=tmp
INC= -Isource/
SRCS=$(wildcard ./samples/*.c)
OBJS=$(SRCS:.c=.o)
MAINFILES:=source/voiceCommand.cpp samples/sample-record.cpp source/voiceCommand-old.cpp
OBJS:=$(patsubst %.cpp, %.o, $(filter-out $(MAINFILES),$(wildcard source/*.cpp)))

.PHONY: all
all: clean $(OBJS) $(TARGET) sample
	@cp $(SRC)/scripts/* $(BIN)/
	@echo "Build successful $(OBJS)"

#$(TARGET):$(TARGET).o
#		$(CC) $(CFLAGS) $^ -o $(BIN)/$@ $(LIBS)

$(TARGET): $(OBJS) source/$(TARGET).o
		$(CC) $(CFLAGS) $^ -o $(BIN)/$@ $(LIBS)

.PHONY: sample
sample: sample-record

sample-record:samples/sample-record.o $(OBJS)
	          $(CC) $(CFLAGS) $^ -o $(BIN)/$@ $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

.PHONY: clean
clean:
	 @echo "Cleaning files..."
	 @rm -f source/*.o samples/*.o
