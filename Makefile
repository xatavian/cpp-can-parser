CC=g++
CCFLAGS=-Wall -std=c++11 -I.
TARGET=cpp-can-parser
TARGET_DIR=bin
MODELS_SRC=$(wildcard models/*.cpp)
PARSING_SRC=$(wildcard parsing/*.cpp)
MAIN_FOLDER_SRC=$(wildcard *.cpp)

OBJS=$(MODELS_SRC:.cpp=.o)
OBJS+=$(PARSING_SRC:.cpp=.o)
OBJS+=$(MAIN_FOLDER_SRC:.cpp=.o)

all: $(TARGET)

cpp-can-parser: $(OBJS)
	$(CC) -o $(TARGET_DIR)/$@ $(addprefix $(TARGET_DIR)/, $(notdir $^))

%.o: %.cpp
	$(CC) $(CCFLAGS) -o $(TARGET_DIR)/$(notdir $*).o -c ./$<
