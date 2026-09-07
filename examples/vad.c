#include "aic.h"

#include <stdio.h>
#include <stdlib.h>

static int print_error(enum AicErrorCode result) {
  switch (result) {
  case AIC_ERROR_CODE_LICENSE_FORMAT_INVALID:
    printf("Invalid license key: check AIC_SDK_LICENSE.\n");
    return 1;
  case AIC_ERROR_CODE_LICENSE_VERSION_UNSUPPORTED:
    printf("Unsupported license version: update the SDK or license key.\n");
    return 1;
  case AIC_ERROR_CODE_LICENSE_EXPIRED:
    printf("License expired: renew AIC_SDK_LICENSE.\n");
    return 1;
  case AIC_ERROR_CODE_PROCESSING_NOT_ALLOWED:
    printf("Processing not allowed: check AIC_SDK_LICENSE/network.\n");
    return 1;
  default:
    return 0;
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s <path/to/vad-model.aicmodel>\n", argv[0]);
    return 1;
  }

  const char *model_path = argv[1];
  printf("Using VAD model: %s\n", model_path);

  printf("Library version: %s\n", aic_get_sdk_version());
  printf("Compatible model version: %u\n", aic_get_compatible_model_version());

  const char *license = getenv("AIC_SDK_LICENSE");
  if (!license) {
    printf("Error: AIC_SDK_LICENSE environment variable not set\n");
    printf("Please set it with: export AIC_SDK_LICENSE=your_license_key\n");
    return 1;
  }

  struct AicModel  *model        = NULL;
  enum AicErrorCode model_result = aic_model_create_from_file(&model, model_path);
  if (model_result != AIC_ERROR_CODE_SUCCESS) {
    printf("Model creation failed with error: %d\n", model_result);
    return 1;
  }

  struct AicVad    *vad        = NULL;
  enum AicErrorCode vad_result = aic_vad_create(&vad, model, license, NULL);
  if (vad_result != AIC_ERROR_CODE_SUCCESS) {
    if (vad_result == AIC_ERROR_CODE_MODEL_TYPE_UNSUPPORTED) {
      printf("Model is not a dedicated VAD model\n");
    } else if (!print_error(vad_result)) {
      printf("VAD creation failed with error: %d\n", vad_result);
    }
    aic_model_destroy(model);
    return 1;
  }

  uint32_t sample_rate;
  size_t   block_size;
  if (aic_model_get_optimal_sample_rate(model, &sample_rate) != AIC_ERROR_CODE_SUCCESS) {
    printf("Failed to read optimal sample rate\n");
    aic_vad_destroy(vad);
    aic_model_destroy(model);
    return 1;
  }
  if (aic_model_get_optimal_block_size(model, sample_rate, &block_size) != AIC_ERROR_CODE_SUCCESS) {
    printf("Failed to read optimal block size\n");
    aic_vad_destroy(vad);
    aic_model_destroy(model);
    return 1;
  }
  printf("Optimal sample rate: %u Hz\n", sample_rate);
  printf("Optimal block size: %zu\n", block_size);

  if (aic_vad_initialize(vad, sample_rate, block_size, false) != AIC_ERROR_CODE_SUCCESS) {
    printf("VAD initialization failed\n");
    aic_vad_destroy(vad);
    aic_model_destroy(model);
    return 1;
  }

  struct AicVadContext *context        = NULL;
  enum AicErrorCode     context_result = aic_vad_context_create(&context, vad);
  if (context_result != AIC_ERROR_CODE_SUCCESS) {
    printf("VAD context creation failed with error: %d\n", context_result);
    aic_vad_destroy(vad);
    aic_model_destroy(model);
    return 1;
  }

  if (aic_vad_context_set_parameter(context, AIC_VAD_PARAMETER_SENSITIVITY, 0.8f) ==
      AIC_ERROR_CODE_SUCCESS) {
    float sensitivity;
    if (aic_vad_context_get_parameter(context, AIC_VAD_PARAMETER_SENSITIVITY, &sensitivity) ==
        AIC_ERROR_CODE_SUCCESS) {
      printf("VAD sensitivity: %f\n", sensitivity);
    }
  }

  uintptr_t delay;
  if (aic_vad_context_get_prediction_delay(context, &delay) == AIC_ERROR_CODE_SUCCESS) {
    printf("VAD prediction delay: %zu samples\n", delay);
  }

  float *audio_buffer = (float *) calloc(block_size, sizeof(float));
  if (!audio_buffer) {
    printf("Audio buffer allocation failed\n");
    aic_vad_context_destroy(context);
    aic_vad_destroy(vad);
    aic_model_destroy(model);
    return 1;
  }

  enum AicErrorCode process_result = aic_vad_process(vad, audio_buffer, block_size);
  if (process_result != AIC_ERROR_CODE_SUCCESS) {
    if (!print_error(process_result)) {
      printf("VAD processing failed with error: %d\n", process_result);
    }
    free(audio_buffer);
    aic_vad_context_destroy(context);
    aic_vad_destroy(vad);
    aic_model_destroy(model);
    return 1;
  }

  bool is_speech_detected = false;
  if (aic_vad_context_is_speech_detected(context, &is_speech_detected) == AIC_ERROR_CODE_SUCCESS) {
    printf("Speech detected: %d\n", is_speech_detected);
  }

  float raw_probability = 0.0f;
  if (aic_vad_context_get_raw_vad_probability(context, &raw_probability) ==
      AIC_ERROR_CODE_SUCCESS) {
    printf("Raw VAD probability: %f\n", raw_probability);
  }

  if (aic_vad_context_reset(context) == AIC_ERROR_CODE_SUCCESS) {
    printf("VAD reset succeeded\n");
  }

  free(audio_buffer);
  aic_vad_context_destroy(context);
  aic_vad_destroy(vad);
  aic_model_destroy(model);
  printf("VAD example completed\n");
  return 0;
}
