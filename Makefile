
CC=g++ 
CFLAGS=-Wall
TARGET=voiceCommand
SRC=./source
SAMPLES=./samples/*.cpp
BIN=bin
INCLUDE=
LIBS= -lcurl -lboost_regex -lasound -lopenal -lFLAC -ljsoncpp -lUnitTest++
TMP=tmp
INC= -Isource/ -Itarget/include/
LDPATH= -Ltarget/lib/

#list of files containing main() function, to prevent conflicts while compiling
MAINFILES:=source/console_command.cpp    \
           samples/sample-record.cpp     \
           source/voiceCommand-old.cpp   \
           samples/curlpost.cpp          \
           source/tests/unittests.cpp    \
           source/tests/test_flac.cpp    \
           source/tests/test_curl.cpp
           
OBJS:=$(patsubst %.cpp, %.o, $(filter-out $(MAINFILES),$(wildcard source/*.cpp)))

.PHONY: all
all: bin $(OBJS) $(TARGET) sample
	@cp $(SRC)/scripts/* $(BIN)/
	@echo "Build successful"
bin: 
	@mkdir -p $@
	
$(TARGET):source/console_command.o $(OBJS) 
	$(CC) $(CFLAGS) $(LDPATH) $^ -o $(BIN)/$@ $(LIBS)

.PHONY: sample
sample: sample-record     \
        voiceCommand-old  \
        curlpost          \
        curlpost_callback      

sample-record:samples/sample-record.o $(OBJS)
	     $(CC) $(CFLAGS) $(LDPATH) $^ -o $(BIN)/$@ $(LIBS)

voiceCommand-old: source/voiceCommand-old.o $(OBJS)
		$(CC) $(CFLAGS) $(LDPATH) $^ -o $(BIN)/$@ $(LIBS)

curlpost: samples/curlpost.o $(OBJS)
		$(CC) $(CFLAGS) $(LDPATH) $^ -o $(BIN)/$@ $(LIBS)
		
curlpost_callback: samples/curlpost_callback.o $(OBJS)
		$(CC) $(CFLAGS) $(LDPATH) $^ -o $(BIN)/$@ $(LIBS)
						   	
%.o: %.cpp
	$(CC) $(CFLAGS) $(INC) -c $< -o $@
			
############ ----- building unit tests ----- ##############

tests: unittests      \
	   test_flac      \
	   test_curl

test_curl: source/tests/test_curl.o $(OBJS)
	   	$(CC) $(CFLAGS) $(LDPATH) $^ -o $(BIN)/$@ $(LIBS)
	   	
test_flac: source/tests/test_flac.o $(OBJS)
	   	$(CC) $(CFLAGS) $(LDPATH) $^ -o $(BIN)/$@ $(LIBS)
	   	
unittests: source/tests/unittests.o $(OBJS)
	   	$(CC) $(CFLAGS) $(LDPATH) $^ -o $(BIN)/$@ $(LIBS)

.PHONY: clean
clean:
	 @echo "Cleaning files..."
	 @rm -f source/*.o samples/*.o source/tests/*.o
