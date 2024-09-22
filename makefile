
# Compiler to use
CC = gcc

# Compiler flags
# CFLAGS = -Wall -Wextra -std=c99 -g
CFLAGS = -g -O0  -gdwarf-2 \
	-I/mnt/c/101_coding/binutils/install/include/elf \
	-I/mnt/c/101_coding/binutils/install/include \
	-I/mnt/c/101_coding/binutils/install/bfd \
	-I/mnt/c/101_coding/binutils/install/gas/config
 
LDFLAGS = -L/mnt/c/101_coding/binutils/install/lib -lbfd -liberty -lz

# Name of the executable
TARGET = go

# Source files
SRCS = main.c 
# SRCS = runtime-static-linking-with-libbfd2.c 

# Object files (automatically generated from the source files)
OBJS = $(SRCS:.c=.o)

# Default target: Build the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)  $(LDFLAGS) 

# Rule to build the object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up the build (remove the executable and object files)
clean:
	rm -f $(OBJS) $(TARGET)
# clean:
# 	del /Q $(subst /,\,$(OBJS)) $(subst /,\,$(TARGET))

# Phony targets (these targets do not correspond to actual files)
.PHONY: clean
