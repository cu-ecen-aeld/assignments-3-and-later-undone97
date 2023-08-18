CROSS_COMPILE=/home/bruno/Desktop/gcc-arm-10.2-2020.11-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-
#CROSS_COMPILE=
CC ?= $(CROSS_COMPILE)gcc
ifndef CROSS_COMPILE
	CC?=gcc
else
	CC?=$(CROSS_COMPILE)gcc
endif

CFLAGS	?= -Wall -Werror -g0
LDFLAGS?=-lrt -pthread
TARGET	?= aesdsocket
INPUT = $(TARGET).c

.SILENT: clean

.PHONY: all clean

$(TARGET): $(INPUT)
	echo "Building server"
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)



clean:
	if [ ! -e $(TARGET) ]; \
	then \
		echo "Nothing to be done"; \
	else \
		echo "Cleaning objects"; \
		rm $(TARGET); \
	fi
	
all:
	$(TARGET) 
