LIB_PATH ?= ../lib/libaic.a
INCLUDE_PATH ?= ../include/

all:
	gcc -I$(INCLUDE_PATH) -o ./aic-sdk-test basic.c $(LIB_PATH) -lm -lpthread -ldl

run: all
	./aic-sdk-test

.PHONY: all run
