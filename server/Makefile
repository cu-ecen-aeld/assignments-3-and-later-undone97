CROSS_COMPILE=/home/bruno/Desktop/gcc-arm-10.2-2020.11-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-
#CROSS_COMPILE=
CC ?= $(CROSS_COMPILE)gcc
ifndef CROSS_COMPILE
	CC?=gcc
else
	CC?=$(CROSS_COMPILE)gcc
endif

CFLAGS	?= -Wall -Werror -g0 -pthread
LDFLAGS?=-lrt -pthread
INPUT 	= aesdsocket.c
TARGET	= aesdsocket
.SILENT: clean
default: $(INPUT)
	echo "Building server"
	$(CC) $(CFLAGS) $(INPUT) -o $(TARGET)

clean:
	if [ ! -e $(TARGET) ]; \
	then \
		echo "Nothing to be done"; \
	else \
		echo "Cleaning objects"; \
		rm $(TARGET); \
	fi
	
all:
	$(CC) $(CFLAGS) $(INPUT) -o $(OUTPUT) 
