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
  printf("Library version: %s\n", aic_get_sdk_version());

  const char *license = getenv("AIC_SDK_LICENSE");
  if (!license) {
    printf("Error: AIC_SDK_LICENSE environment variable not set\n");
    return 1;
  }

  // Model creation from file
  struct AicModel  *model        = NULL;
  enum AicErrorCode model_result = aic_model_create_from_file(&model, model_path);
  if (model_result != AIC_ERROR_CODE_SUCCESS) {
    printf("Model creation failed with error: %d\n", model_result);
    return 1;
  }

  uint32_t          sample_rate        = 0;
  enum AicErrorCode sample_rate_result = aic_model_get_optimal_sample_rate(model, &sample_rate);
  if (sample_rate_result != AIC_ERROR_CODE_SUCCESS) {
    printf("Failed to get optimal sample rate with error: %d\n", sample_rate_result);
    aic_model_destroy(model);
    return 1;
  }

  size_t            block_size = 0;
  enum AicErrorCode block_size_result =
      aic_model_get_optimal_block_size(model, sample_rate, &block_size);
  if (block_size_result != AIC_ERROR_CODE_SUCCESS || block_size == 0) {
    printf("Failed to get optimal block size with error: %d\n", block_size_result);
    aic_model_destroy(model);
    return 1;
  }

  // Analyzer creation: the collector buffers audio, the analyzer processes the
  // latest buffer.
  struct AicCollector *collector = NULL;
  struct AicAnalyzer  *analyzer  = NULL;
  enum AicErrorCode    analyzer_result =
      aic_analyzer_pair_create(&collector, &analyzer, model, license);
  if (analyzer_result != AIC_ERROR_CODE_SUCCESS) {
    if (!print_error(analyzer_result)) {
      printf("Analyzer creation failed with error: %d\n", analyzer_result);
    }
    aic_model_destroy(model);
    return 1;
  }

  enum AicErrorCode initialize_result =
      aic_collector_initialize(collector, sample_rate, block_size, true);
  if (initialize_result != AIC_ERROR_CODE_SUCCESS) {
    printf("Analyzer collector initialization failed with error: %d\n", initialize_result);
    aic_analyzer_destroy(analyzer);
    aic_collector_destroy(collector);
    aic_model_destroy(model);
    return 1;
  }

  // A one-second mono signal to analyze (silence here, for demonstration).
  size_t signal_length = sample_rate;
  float *signal        = (float *) calloc(signal_length, sizeof(float));
  if (!signal) {
    printf("Allocation failed\n");
    aic_analyzer_destroy(analyzer);
    aic_collector_destroy(collector);
    aic_model_destroy(model);
    return 1;
  }

  for (size_t offset = 0; offset < signal_length; offset += block_size) {
    size_t            remaining          = signal_length - offset;
    size_t            samples_this_block = remaining < block_size ? remaining : block_size;
    enum AicErrorCode buffer_result =
        aic_collector_buffer(collector, signal + offset, samples_this_block);
    if (buffer_result != AIC_ERROR_CODE_SUCCESS) {
      printf("Audio buffering failed with error: %d\n", buffer_result);
      free(signal);
      aic_analyzer_destroy(analyzer);
      aic_collector_destroy(collector);
      aic_model_destroy(model);
      return 1;
    }
  }

  struct AicAnalysisResult result         = {0};
  enum AicErrorCode        analyze_result = aic_analyzer_analyze_buffered(analyzer, &result);
  if (analyze_result != AIC_ERROR_CODE_SUCCESS) {
    if (!print_error(analyze_result)) {
      printf("Analysis failed with error: %d\n", analyze_result);
    }
    free(signal);
    aic_analyzer_destroy(analyzer);
    aic_collector_destroy(collector);
    aic_model_destroy(model);
    return 1;
  }

  printf("risk_score:           %f\n", result.risk_score);
  printf("speaker_reverb:       %f\n", result.speaker_reverb);
  printf("speaker_loudness:     %f\n", result.speaker_loudness);
  printf("interfering_speech:   %f\n", result.interfering_speech);
  printf("noise:                %f\n", result.noise);
  printf("codec_degradation:    %f\n", result.codec_degradation);
  printf("packet_loss:          %f\n", result.packet_loss);

  // Clean up
  free(signal);
  aic_analyzer_destroy(analyzer);
  aic_collector_destroy(collector);
  aic_model_destroy(model);
  printf("Analysis completed\n");
  return 0;
}
