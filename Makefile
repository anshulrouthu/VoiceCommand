
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

#list of files containing main() function, to prevent conflicts while compiling
MAINFILES:=source/console_command.cpp samples/sample-record.cpp source/voiceCommand-old.cpp
OBJS:=$(patsubst %.cpp, %.o, $(filter-out $(MAINFILES),$(wildcard source/*.cpp)))

.PHONY: all
all: bin $(OBJS) $(TARGET) sample
	@cp $(SRC)/scripts/* $(BIN)/
	@echo "Build successful"
bin: 
	@mkdir -p $@
	
$(TARGET):source/console_command.o $(OBJS) 
	$(CC) $(CFLAGS) $^ -o $(BIN)/$@ $(LIBS)

.PHONY: sample
sample: sample-record voiceCommand-old

sample-record:samples/sample-record.o $(OBJS)
	     $(CC) $(CFLAGS) $^ -o $(BIN)/$@ $(LIBS)

voiceCommand-old: source/voiceCommand-old.o $(OBJS)
				$(CC) $(CFLAGS) $^ -o $(BIN)/$@ $(LIBS)
%.o: %.cpp
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

.PHONY: clean
clean:
	 @echo "Cleaning files..."
	 @rm -f source/*.o samples/*.o
