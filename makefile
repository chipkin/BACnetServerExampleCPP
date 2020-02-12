# Make file for building the CASModbusDLL as a Shared library
# This makes it so that it acts similar to the CASModbusDLL in Windows

# The Compiler: gcc for C, g++ for C++
CC := g++

NAME := BACnetServerExampleCPP_x64_Release

# Compiler flags:
# -m32 for 32bit, -m64 for 64bit
# -Wall turns on most, but not all, compiler warnings
#
CFLAGS := -m64 -Wall

DEBUGFLAGS = -O0 -g3 -DCAS_BACNET_STACK_LIB_TYPE_LIB
RELEASEFLAGS = -O3 -DCAS_BACNET_STACK_LIB_TYPE_LIB
OBJECTFLAGS = -c -fmessage-length=0 -fPIC -MMD -MP
LDFLAGS = -static

SOURCES = $(wildcard ../../*.cpp) $(wildcard ../../../BACnetStackSimpleUDP/*.cpp) $(wildcard ../../../../adapters/cpp/*.cpp)
OBJECTS = $(addprefix obj/,$(notdir $(SOURCES:.cpp=.o)))
INCLUDES = -I../../../../source -I../../../BACnetStackSimpleUDP -I../../../../adapters/cpp
LIBPATH = -L../../../../bin -L../../../../submodules/cas-common/bin
LIB = -ldl -lCASBACnetStack_x64_Release -lCASCommon_x64_Release

# Build Target
TARGET = $(NAME)

all: $(NAME)

$(NAME): $(OBJECTS)
	@echo 'Building target: $@'
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(NAME) $(OBJECTS) $(LIBPATH) $(LIB)
	@echo 'Finished building target: $@'
	@echo ' '

obj/%.o: ../../../../adapters/cpp/%.cpp
	@mkdir -p obj
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	$(CC) $(RELEASEFLAGS) $(CFLAGS) $(OBJECTFLAGS) $(INCLUDES) $(LIBPATH) -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ $<
	@echo 'Finished building: $<'
	@echo ' '

obj/%.o: ../../../BACnetStackSimpleUDP/%.cpp
	@mkdir -p obj
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	$(CC) $(RELEASEFLAGS) $(CFLAGS) $(OBJECTFLAGS) $(INCLUDES) $(LIBPATH) -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ $<
	@echo 'Finished building: $<'
	@echo ' '

# General source files for the API
obj/%.o: ../../%.cpp
	@mkdir -p obj
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	$(CC) $(RELEASEFLAGS) $(CFLAGS) $(OBJECTFLAGS) $(INCLUDES) $(LIBPATH) -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ $<
	@echo 'Finished building: $<'
	@echo ' '


install:
	install -D $(NAME) ../../../../bin/$(NAME)

# make clean
# Removes target file and any .o object files, 
# .d dependency files, or ~ backup files
clean:
	$(RM) $(NAME) obj/* *~
