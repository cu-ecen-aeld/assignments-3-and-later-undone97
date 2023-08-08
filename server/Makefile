#CROSS_COMPILE=/home/bruno/Desktop/gcc-arm-10.2-2020.11-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-
CROSS_COMPILE=
#GCC=$(CROSS_COMPILE)gcc
ifndef CROSS_COMPILE
	GCC=gcc
else
	GCC=$(CROSS_COMPILE)gcc
endif

CFLAGS	= -Wall -Werror -g0 -pthread
#CFLAGS=
INPUT 	= aesdsocket.c
OUTPUT	= aesdsocket
.SILENT: clean
default: $(INPUT)
	echo "Building server"
	$(GCC) $(CFLAGS) $(INPUT) -o $(OUTPUT)

clean:
	if [ ! -e $(OUTPUT) ]; \
	then \
		echo "Nothing to be done"; \
	else \
		echo "Cleaning objects"; \
		rm $(OUTPUT); \
	fi
	
all:
	$(GCC) $(CFLAGS) $(INPUT) -o $(OUTPUT) 
