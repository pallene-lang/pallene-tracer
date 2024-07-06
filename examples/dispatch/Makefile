CC := gcc
CURRENT_DIR := $(shell realpath --relative-to=$(shell pwd) $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST)))))

.PHONY :
.SILENT:

all: 
	$(CC) -fPIC -O2 -shared $(CURRENT_DIR)/module.c -o $(CURRENT_DIR)/module.so
