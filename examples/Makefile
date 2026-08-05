AIC_LIB_PATH ?= ../lib/libaic.a
AIC_INCLUDE_PATH ?= ../include/
MODEL_DIR ?= models
QUAIL_MODEL_URL := https://artifacts.ai-coustics.io/models/quail-vf-2-2-s-16khz/v6/quail_vf_2_2_s_16khz_gf70x7zf_v14.aicmodel
DEFAULT_ENHANCEMENT_MODEL_PATH := $(MODEL_DIR)/v6/quail_vf_2_2_s_16khz_gf70x7zf_v14.aicmodel
TYTO_MODEL_URL := https://artifacts.ai-coustics.io/models/tyto-l-16khz/v6/tyto_l_16khz_yhlek4hc_v43.aicmodel
DEFAULT_ANALYZER_MODEL_PATH := $(MODEL_DIR)/v6/tyto_l_16khz_yhlek4hc_v43.aicmodel

ifeq ($(origin MODEL_PATH),undefined)
ENHANCEMENT_MODEL_PATH := $(DEFAULT_ENHANCEMENT_MODEL_PATH)
ANALYZER_MODEL_PATH := $(DEFAULT_ANALYZER_MODEL_PATH)
MODEL_PATH_PROVIDED := false
else
ENHANCEMENT_MODEL_PATH := $(MODEL_PATH)
ANALYZER_MODEL_PATH := $(MODEL_PATH)
VAD_MODEL_PATH := $(MODEL_PATH)
MODEL_PATH_PROVIDED := true
endif

all: enhancement vad analyzer

enhancement:
	gcc -I$(AIC_INCLUDE_PATH) -o ./aic-sdk-enhancement enhancement.c $(AIC_LIB_PATH) -lm -lpthread -ldl

vad:
	gcc -I$(AIC_INCLUDE_PATH) -o ./aic-sdk-vad vad.c $(AIC_LIB_PATH) -lm -lpthread -ldl

analyzer:
	gcc -I$(AIC_INCLUDE_PATH) -o ./aic-sdk-analyzer analyzer.c $(AIC_LIB_PATH) -lm -lpthread -ldl

$(DEFAULT_ENHANCEMENT_MODEL_PATH):
	@echo "Ensuring model present at $(DEFAULT_ENHANCEMENT_MODEL_PATH)"
	@if [ ! -f "$(DEFAULT_ENHANCEMENT_MODEL_PATH)" ]; then \
		mkdir -p "$(@D)"; \
		curl -fL -o "$(DEFAULT_ENHANCEMENT_MODEL_PATH)" "$(QUAIL_MODEL_URL)"; \
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
run-enhancement: enhancement $(DEFAULT_ENHANCEMENT_MODEL_PATH)
else
run-enhancement: enhancement
endif
	./aic-sdk-enhancement $(ENHANCEMENT_MODEL_PATH)

run: run-enhancement

ifeq ($(MODEL_PATH_PROVIDED),false)
run-analyzer: analyzer $(DEFAULT_ANALYZER_MODEL_PATH)
else
run-analyzer: analyzer
endif
	./aic-sdk-analyzer $(ANALYZER_MODEL_PATH)

ifeq ($(MODEL_PATH_PROVIDED),true)
run-vad: vad
	./aic-sdk-vad $(VAD_MODEL_PATH)
else
run-vad:
	@echo "run-vad requires MODEL_PATH because no default VAD model URL is configured." >&2
	@echo "Run 'make run-vad MODEL_PATH=path/to/your/vad-model.aicmodel'." >&2
	@exit 2
endif

ifeq ($(MODEL_PATH_PROVIDED),true)
run-examples:
	@echo "MODEL_PATH can only be used with run-enhancement, run-vad, or run-analyzer, because the examples require different model types." >&2
	@echo "Run 'make run-enhancement MODEL_PATH=...', 'make run-vad MODEL_PATH=...', or 'make run-analyzer MODEL_PATH=...' instead." >&2
	@exit 2
else
run-examples: run-enhancement run-analyzer
endif

clean:
	rm -f ./aic-sdk-enhancement ./aic-sdk-vad ./aic-sdk-analyzer

.PHONY: all enhancement vad analyzer run run-enhancement run-vad run-analyzer run-examples clean
