AIC_LIB_PATH ?= ../lib/libaic.a
AIC_INCLUDE_PATH ?= ../include/
MODEL_DIR ?= models/v5
QUAIL_MODEL_URL := https://artifacts.ai-coustics.io/models/quail-vf-2-1-s-16khz/v5/quail_vf_2_1_s_16khz_5i8jb8of_v12.aicmodel
DEFAULT_BASIC_MODEL_PATH := $(MODEL_DIR)/quail_vf_2_1_s_16khz_5i8jb8of_v12.aicmodel
TYTO_MODEL_URL := https://artifacts.ai-coustics.io/models/tyto-l-16khz/v5/tyto_l_16khz_yhlek4hc_v43.aicmodel
DEFAULT_ANALYZER_MODEL_PATH := $(MODEL_DIR)/tyto_l_16khz_yhlek4hc_v43.aicmodel

ifeq ($(origin MODEL_PATH),undefined)
BASIC_MODEL_PATH := $(DEFAULT_BASIC_MODEL_PATH)
ANALYZER_MODEL_PATH := $(DEFAULT_ANALYZER_MODEL_PATH)
MODEL_PATH_PROVIDED := false
else
BASIC_MODEL_PATH := $(MODEL_PATH)
ANALYZER_MODEL_PATH := $(MODEL_PATH)
MODEL_PATH_PROVIDED := true
endif

all: basic analyzer

basic:
	gcc -I$(AIC_INCLUDE_PATH) -o ./aic-sdk-test basic.c $(AIC_LIB_PATH) -lm -lpthread -ldl

analyzer:
	gcc -I$(AIC_INCLUDE_PATH) -o ./aic-sdk-analyzer analyzer.c $(AIC_LIB_PATH) -lm -lpthread -ldl

$(DEFAULT_BASIC_MODEL_PATH):
	@echo "Ensuring model present at $(DEFAULT_BASIC_MODEL_PATH)"
	@if [ ! -f "$(DEFAULT_BASIC_MODEL_PATH)" ]; then \
		mkdir -p "$(@D)"; \
		curl -fL -o "$(DEFAULT_BASIC_MODEL_PATH)" "$(QUAIL_MODEL_URL)"; \
	else \
		echo "Model already exists, skipping download"; \
	fi

$(DEFAULT_ANALYZER_MODEL_PATH):
	@echo "Ensuring model present at $(DEFAULT_ANALYZER_MODEL_PATH)"
	@if [ ! -f "$(DEFAULT_ANALYZER_MODEL_PATH)" ]; then \
		mkdir -p "$(@D)"; \
		curl -fL -o "$(DEFAULT_ANALYZER_MODEL_PATH)" "$(TYTO_MODEL_URL)"; \
	else \
		echo "Model already exists, skipping download"; \
	fi

ifeq ($(MODEL_PATH_PROVIDED),false)
run-basic: basic $(DEFAULT_BASIC_MODEL_PATH)
else
run-basic: basic
endif
	./aic-sdk-test $(BASIC_MODEL_PATH)

run: run-basic

ifeq ($(MODEL_PATH_PROVIDED),false)
run-analyzer: analyzer $(DEFAULT_ANALYZER_MODEL_PATH)
else
run-analyzer: analyzer
endif
	./aic-sdk-analyzer $(ANALYZER_MODEL_PATH)

ifeq ($(MODEL_PATH_PROVIDED),true)
run-examples:
	@echo "MODEL_PATH can only be used with run-basic or run-analyzer, because the examples require different model types." >&2
	@echo "Run 'make run-basic MODEL_PATH=...' or 'make run-analyzer MODEL_PATH=...' instead." >&2
	@exit 2
else
run-examples: run-basic run-analyzer
endif

clean:
	rm -f ./aic-sdk-test ./aic-sdk-analyzer

.PHONY: all basic analyzer run run-basic run-analyzer run-examples clean
