AIC_LIB_PATH ?= ../lib/libaic.a
AIC_INCLUDE_PATH ?= ../include/

all:
	gcc -I$(AIC_INCLUDE_PATH) -o ./aic-sdk-test basic.c $(AIC_LIB_PATH) -lm -lpthread -ldl

run: all
	./aic-sdk-test

clean:
	rm ./aic-sdk-test

.PHONY: all run clean
