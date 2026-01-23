AIC_LIB_PATH ?= ../lib/libaic.a
AIC_INCLUDE_PATH ?= ../include/
MODEL_URL := https://artifacts.ai-coustics.io/models/sparrow-xxs-48khz/v1/sparrow_xxs_48khz_wsur2zkw_v7.aicmodel
MODEL_PATH := sparrow_xxs_48khz_wsur2zkw_v7.aicmodel

all: $(MODEL_PATH)
	gcc -I$(AIC_INCLUDE_PATH) -o ./aic-sdk-test basic.c $(AIC_LIB_PATH) -lm -lpthread -ldl

$(MODEL_PATH):
	@echo "Ensuring model present at $(MODEL_PATH)"
	@if [ ! -f "$(MODEL_PATH)" ]; then \
		curl -fL -o "$(MODEL_PATH)" "$(MODEL_URL)"; \
	else \
		echo "Model already exists, skipping download"; \
	fi

run: all
	./aic-sdk-test $(MODEL_PATH)

clean:
	rm ./aic-sdk-test

.PHONY: all run clean
