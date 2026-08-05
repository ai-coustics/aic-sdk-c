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
    printf("Usage: %s <path/to/model.aicmodel>\n", argv[0]);
    return 1;
  }

  const char *model_path = argv[1];
  printf("Using model: %s\n", model_path);

  // Display library version
  printf("Library version: %s\n", aic_get_sdk_version());
  printf("Compatible model version: %u\n", aic_get_compatible_model_version());

  // Get license key from environment variable
  const char *license = getenv("AIC_SDK_LICENSE");
  if (!license) {
    printf("Error: AIC_SDK_LICENSE environment variable not set\n");
    printf("Please set it with: export AIC_SDK_LICENSE=your_license_key\n");
    return 1;
  }

  // Model creation from file
  struct AicModel  *model        = NULL;
  enum AicErrorCode model_result = aic_model_create_from_file(&model, model_path);
  if (model_result != AIC_ERROR_CODE_SUCCESS) {
    printf("Model creation failed with error: %d\n", model_result);
    return 1;
  }

  // Processor creation with license key
  struct AicProcessor *processor        = NULL;
  enum AicErrorCode    processor_result = aic_processor_create(&processor, model, license, NULL);
  if (processor_result != AIC_ERROR_CODE_SUCCESS) {
    if (!print_error(processor_result)) {
      printf("Processor creation failed with error: %d\n", processor_result);
    }
    aic_model_destroy(model);
    return 1;
  }

  // Get optimal settings
  uint32_t sample_rate;
  size_t   block_size;
  if (aic_model_get_optimal_sample_rate(model, &sample_rate) != AIC_ERROR_CODE_SUCCESS) {
    printf("Failed to read optimal sample rate\n");
    aic_processor_destroy(processor);
    aic_model_destroy(model);
    return 1;
  }
  if (aic_model_get_optimal_block_size(model, sample_rate, &block_size) != AIC_ERROR_CODE_SUCCESS) {
    printf("Failed to read optimal block size\n");
    aic_processor_destroy(processor);
    aic_model_destroy(model);
    return 1;
  }
  printf("Optimal sample rate: %u Hz\n", sample_rate);
  printf("Optimal block size: %zu\n", block_size);

  // Initialize with basic audio config
  if (aic_processor_initialize(processor, sample_rate, block_size, false) !=
      AIC_ERROR_CODE_SUCCESS) {
    printf("Processor initialization failed\n");
    aic_processor_destroy(processor);
    aic_model_destroy(model);
    return 1;
  }

  // Test audio processing
  float *audio_buffer = (float *) calloc(block_size, sizeof(float));
  if (!audio_buffer) {
    printf("Audio buffer allocation failed\n");
    aic_processor_destroy(processor);
    aic_model_destroy(model);
    return 1;
  }

  enum AicErrorCode process_result = aic_processor_process(processor, audio_buffer, block_size);

  if (process_result != AIC_ERROR_CODE_SUCCESS) {
    if (!print_error(process_result)) {
      printf("Audio processing failed with error: %d\n", process_result);
    }
    free(audio_buffer);
    aic_processor_destroy(processor);
    aic_model_destroy(model);
    return 1;
  } else {
    printf("Audio processing succeeded\n");
  }

  // Processor context handle for thread-safe control APIs
  struct AicProcessorContext *proc_context = NULL;
  enum AicErrorCode proc_context_result    = aic_processor_context_create(&proc_context, processor);
  if (proc_context_result != AIC_ERROR_CODE_SUCCESS) {
    printf("Processor context creation failed with error: %d\n", proc_context_result);
    free(audio_buffer);
    aic_processor_destroy(processor);
    aic_model_destroy(model);
    return 1;
  }

  // Test reset functionality
  if (aic_processor_context_reset(proc_context) == AIC_ERROR_CODE_SUCCESS) {
    printf("Processor reset succeeded\n");
  }

  // Test parameter setting and getting
  if (aic_processor_context_set_parameter(proc_context, AIC_PROCESSOR_PARAMETER_ENHANCEMENT_LEVEL,
                                          0.7f) == AIC_ERROR_CODE_SUCCESS) {
    printf("Enhancement parameter set successfully\n");

    float value;
    if (aic_processor_context_get_parameter(proc_context, AIC_PROCESSOR_PARAMETER_ENHANCEMENT_LEVEL,
                                            &value) == AIC_ERROR_CODE_SUCCESS) {
      printf("Enhancement level: %f\n", value);
    }
  }

  // Get the delay applied to the audio
  uintptr_t delay;
  if (aic_processor_context_get_audio_delay(proc_context, &delay) == AIC_ERROR_CODE_SUCCESS) {
    printf("Audio delay: %zu samples\n", delay);
  }

  // Clean up
  free(audio_buffer);
  aic_processor_context_destroy(proc_context);
  aic_processor_destroy(processor);
  aic_model_destroy(model);
  printf("All tests completed\n");
  return 0;
}
