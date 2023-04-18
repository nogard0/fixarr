#
# 'make'        build executable file 'main'
# 'make clean'  removes all .o and executable files
#

# define the C compiler to use
CC = gcc

# define any compile-time flags
CFLAGS	:= -Wall -Wextra -g -std=gnu99 -D_XOPEN_SOURCE -D_DEFAULT_SOURCE

ifdef NO_DEBUG
CFLAGS += -DNO_DEBUG
endif


# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
LFLAGS =

# define output directory
OUTPUT	:= .

# define source directory
SRC		:= src

# define include directory
INCLUDE	:= include

ifeq ($(OS),Windows_NT)
MAIN	:= main.exe
SOURCEDIRS	:= $(SRC)
INCLUDEDIRS	:= $(INCLUDE)
FIXPATH = $(subst /,\,$1)
RM			:= del /q /f
MD	:= mkdir
else
MAIN	:= fixarr
SOURCEDIRS	:= $(shell find $(SRC) -type d)
INCLUDEDIRS	:= $(shell find $(INCLUDE) -type d)
FIXPATH = $1
RM = rm -f
MD	:= mkdir -p
endif

# define any directories containing header files other than /usr/include
INCLUDES	:= $(patsubst %,-I%, $(INCLUDEDIRS:%/=%))

# define the C libs
LDLIBS = -lulfius -ljansson

# define the C source files
SOURCES		:= $(wildcard $(patsubst %,%/*.c, $(SOURCEDIRS)))

# define the C object files 
OBJECTS		:= $(SOURCES:.c=.o)

# define the dependency output files
DEPS		:= $(OBJECTS:.o=.d)

#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

OUTPUTMAIN	:= $(call FIXPATH,$(OUTPUT)/$(MAIN))

all: $(OUTPUT) $(MAIN)
	@echo Executing 'all' complete!

$(OUTPUT):
	$(MD) $(OUTPUT)

$(MAIN): $(OBJECTS) 
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUTPUTMAIN) $(OBJECTS) $(LFLAGS) $(LDLIBS)

# include all .d files
-include $(DEPS)

# this is a suffix replacement rule for building .o's and .d's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file) 
# -MMD generates dependency output files same name as the .o file
# (see the gnu make manual section about automatic variables)
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c -MMD $<  -o $@

.PHONY: clean
clean:
	$(RM) $(OUTPUTMAIN)
	$(RM) $(call FIXPATH,$(OBJECTS))
	$(RM) $(call FIXPATH,$(DEPS))
	@echo Cleanup complete!

run: all
	./$(OUTPUTMAIN)
	@echo Executing 'run: all' complete!

install:
	@if [ "`whoami`" = "root" ] ; \
	then \
		echo "Install in /usr/local/bin"; \
		install -m 755 $(OUTPUTMAIN) /usr/local/bin/; \
		echo "Config file: /etc/fixarr.json"; \
		if [ ! -f /etc/fixarr.json ]; then cp fixarr.json /etc/; chmod 600 /etc/fixarr.json; fi; \
	else \
		echo "Install in ~/.local/bin"; \
		install -m 755 $(OUTPUTMAIN) ~/.local/bin; \
		echo "Config file: ~/fixarr.json"; \
		if [ ! -f ~/fixarr.json ]; then cp fixarr.json ~; chmod 600 ~/fixarr.json; fi; \
	fi

install_service:
	mkdir -p /etc/systemd/system
	cp fixarr.service /etc/systemd/system/
	systemctl daemon-reload
	systemctl enable fixarr.service