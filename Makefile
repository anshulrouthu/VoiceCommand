
CFLAGS=-Wall -g -O2
TARGET=voiceCommand
SRC=./source
SAMPLES=./samples/*.cpp
BIN=bin
INCLUDE=
LIBS= -lcurl -lboost_regex -lasound -lopenal -lFLAC -ljsoncpp -lUnitTest++
TMP=tmp
INC=-Itarget/include/ -Isource/components/ -Isource/framework/ -Isource/osapi/
LDPATH= -Ltarget/lib/

#list of files containing main() function, to prevent conflicts while linking
MAINFILES:=source/main/console_command.cpp    \
           samples/sample-record.cpp          \
           samples/curlpost.cpp               \
           source/tests/unittests.cpp         \
           source/tests/test_flac.cpp         \
           source/tests/test_curl.cpp
           
OBJS:=$(patsubst %.cpp, %.o, $(filter-out $(MAINFILES),$(wildcard source/components/*.cpp) $(wildcard source/framework/*.cpp) $(wildcard source/osapi/*.cpp)))

############ ----- build main application ----- ##############

.PHONY: all
all: bin $(OBJS) $(TARGET) sample tests
	@cp $(SRC)/scripts/* $(BIN)/
	@echo "Build successful"

bin: 
	@mkdir -p $@
	
$(TARGET):source/main/console_command.o $(OBJS) 
	@echo "Linking... $@"
	@$(CXX) $(CFLAGS) -O2 $(LDPATH) $^ -o $(BIN)/$@ $(LIBS)

%.o: %.cpp
	@echo "[CXX] $@"
	@$(CXX) $(CFLAGS) $(INC) -c $< -o $@

%.o: %.c
	@echo "[CC] $@"
	@$(CC) $(CFLAGS) $(INC) -c $< -o $@

############ ----- build samples ----- ##############

.PHONY: sample
sample: sample-record     \
        voiceCommand-old  \
        curlpost          \
        curlpost_callback      

sample-record:samples/sample-record.o $(OBJS)
	     $(CXX) $(CFLAGS) $(LDPATH) $^ -o $(BIN)/$@ $(LIBS)

voiceCommand-old: samples/voiceCommand-old.o $(OBJS)
		$(CXX) $(CFLAGS) $(LDPATH) $^ -o $(BIN)/$@ $(LIBS)

curlpost: samples/curlpost.o $(OBJS)
		$(CXX) $(CFLAGS) $(LDPATH) $^ -o $(BIN)/$@ $(LIBS)
		
curlpost_callback: samples/curlpost_callback.o $(OBJS)
		$(CXX) $(CFLAGS) $(LDPATH) $^ -o $(BIN)/$@ $(LIBS)
			
############ ----- build tests ----- ##############

.PHONY: tests
tests: unittests      \
       test_flac      \
       test_curl      \
       test_osapi

test_curl: source/tests/test_curl.o $(OBJS)
	   @echo "Linking... $@"
	   @$(CC) $(CFLAGS) $(LDPATH) $^ -o $(BIN)/$@ $(LIBS)
	   	
test_flac: source/tests/test_flac.o $(OBJS)
	   @echo "Linking... $@"
	   @$(CC) $(CFLAGS) $(LDPATH) $^ -o $(BIN)/$@ $(LIBS)
	   	
unittests: source/tests/unittests.o $(OBJS)
	   @echo "Linking... $@"
	   @$(CC) $(CFLAGS) $(LDPATH) $^ -o $(BIN)/$@ $(LIBS)

test_osapi: source/tests/test_osapi.o $(OBJS)
	    @echo "Linking... $@"
	    @$(CC) $(CFLAGS) $(LDPATH) $^ -o $(BIN)/$@ $(LIBS)

.PHONY: clean
clean:
	 @echo "Cleaning files..."
	 @rm -f source/components/*.o           \
	        source/framework/*.o            \
	        source/main/*.o                 \
	        samples/*.o                     \
	        source/tests/*.o  
