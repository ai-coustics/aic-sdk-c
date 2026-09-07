/**
 * This file contains the definitions and declarations for the ai-coustics
 * SDK, including initialization, processing, and configuration functions. The
 * ai-coustics SDK provides advanced machine learning models that can be used
 * in audio streaming contexts.
 *
 * Copyright (C) ai-coustics GmbH - All Rights Reserved
 *
 * Unauthorized copying, distribution, or modification of this file,
 * via any medium, is strictly prohibited.
 *
 * For inquiries, please contact: systems@ai-coustics.com
 */


#ifndef AIC_H
#define AIC_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum AicErrorCode {
  /**
   * Operation completed successfully
   */
  AIC_ERROR_CODE_SUCCESS = 0,
  /**
   * Required pointer argument was NULL. Check all pointer parameters.
   */
  AIC_ERROR_CODE_NULL_POINTER = 1,
  /**
   * Parameter value is outside the acceptable range. Check documentation for valid values.
   */
  AIC_ERROR_CODE_PARAMETER_OUT_OF_RANGE = 2,
  /**
   * Handle must be initialized before calling this operation.
   */
  AIC_ERROR_CODE_NOT_INITIALIZED = 3,
  /**
   * Audio configuration (sample_rate, block_size) is not supported by the model
   */
  AIC_ERROR_CODE_AUDIO_CONFIG_UNSUPPORTED = 4,
  /**
   * Audio block configuration differs from the one provided during initialization
   */
  AIC_ERROR_CODE_AUDIO_CONFIG_MISMATCH = 5,
  /**
   * Processing is not allowed because the SDK key was not authorized or usage reporting failed.
   */
  AIC_ERROR_CODE_PROCESSING_NOT_ALLOWED = 6,
  /**
   * Internal error occurred. Contact support.
   */
  AIC_ERROR_CODE_INTERNAL_ERROR = 7,
  /**
   * License key format is invalid or corrupted. Verify the key was copied correctly.
   */
  AIC_ERROR_CODE_LICENSE_FORMAT_INVALID = 50,
  /**
   * License version is not compatible with the SDK version. Update SDK or contact support.
   */
  AIC_ERROR_CODE_LICENSE_VERSION_UNSUPPORTED = 51,
  /**
   * License key has expired. Renew your license to continue.
   */
  AIC_ERROR_CODE_LICENSE_EXPIRED = 52,
  /**
   * Updating the token is only supported when both the original and new keys are JWT-form licenses.
   */
  AIC_ERROR_CODE_TOKEN_UPDATE_UNSUPPORTED = 53,
  /**
   * The model file is invalid or corrupted. Verify the file is correct.
   */
  AIC_ERROR_CODE_MODEL_INVALID = 100,
  /**
   * The model file version is not compatible with this SDK version.
   */
  AIC_ERROR_CODE_MODEL_VERSION_UNSUPPORTED = 101,
  /**
   * The file path is invalid.
   */
  AIC_ERROR_CODE_FILE_PATH_INVALID = 102,
  /**
   * The model file cannot be opened due to a filesystem error. Verify that the file exists.
   */
  AIC_ERROR_CODE_FILE_SYSTEM_ERROR = 103,
  /**
   * The model data is not aligned to 64 bytes.
   */
  AIC_ERROR_CODE_MODEL_DATA_UNALIGNED = 104,
  /**
   * The model type is not supported by the requested API.
   */
  AIC_ERROR_CODE_MODEL_TYPE_UNSUPPORTED = 105,
} AicErrorCode;

/**
 * Configurable parameters for audio processing.
 */
typedef enum AicProcessorParameter {
  /**
   * Controls whether audio processing is bypassed while preserving algorithmic delay.
   *
   * When enabled, the input audio passes through unmodified, but the output is still
   * delayed by the same amount as during normal processing. This ensures seamless
   * transitions when toggling enhancement on/off without audible clicks or timing shifts.
   *
   * **Range:** 0.0 to 1.0
   * - **0.0:** Enhancement active (normal processing)
   * - **1.0:** Bypass enabled (latency-compensated passthrough)
   *
   * **Default:** 0.0
   */
  AIC_PROCESSOR_PARAMETER_BYPASS = 0,
  /**
   * A tunable parameter to optimize for specific STT engines, deployment environments,
   * and user experience requirements.
   *
   * The exact behavior depends on the active model:
   * - **Quail Models:** Controls how aggressively the model suppresses noise. When used
   *   with Quail Voice Focus, it also suppresses background and competing speech.
   * - **Rook Models:** Controls the mixback and therefore the intensity of the
   *   enhancement.
   *
   * **Range:** 0.0 to 1.0
   */
  AIC_PROCESSOR_PARAMETER_ENHANCEMENT_LEVEL = 1,
} AicProcessorParameter;

/**
 * Configurable parameters for Voice Activity Detection.
 */
typedef enum AicVadParameter {
  /**
   * Controls for how long the VAD continues to detect speech after the audio signal
   * no longer contains speech.
   *
   * This affects the stability of speech detected -> not detected transitions.
   *
   * The VAD reports speech detected if the audio signal contained speech in at least 50%
   * of the blocks processed in the last `speech_hold_duration * 2` seconds.
   *
   * For example, if `speech_hold_duration` is set to 0.5 seconds and the VAD stops detecting speech
   * in the audio signal, the VAD will continue to report speech for 0.5 seconds assuming the
   * VAD does not detect speech again during that period. If a few blocks of speech are detected
   * during that period, those blocks will be included in the 50% calculation, which will extend
   * the speech detection period until the 50% threshold is no longer met.
   *
   * NOTE: The VAD returns a value per processed audio block, so this duration is rounded
   * to the closest model window length. For example, if the model has a processing window
   * length of 10 ms, the VAD will round up/down to the closest multiple of 10 ms.
   * Because of this, this parameter may return a different value than the one it was last set to.
   *
   * **Range:** 0.0 to 300x model window length (value in seconds)
   *
   * **Default:** model-specific
   */
  AIC_VAD_PARAMETER_SPEECH_HOLD_DURATION = 0,
  /**
   * Controls the sensitivity of the VAD.
   *
   * VAD models output a probability of speech presence for each processed
   * audio block, 1.0 being the model is certain speech is present and 0.0 being the
   * model is certain speech is not present. The probability is compared against the
   * sensitivity threshold to determine if speech is detected.
   *
   * A value above the threshold will trigger a speech detected decision.
   *
   * **Range:** 0.0 to 1.0
   *
   * **Default:** model-specific
   */
  AIC_VAD_PARAMETER_SENSITIVITY = 1,
  /**
   * Controls for how long speech needs to be present in the audio signal before
   * the VAD considers it speech.
   *
   * This affects the stability of speech not detected -> detected transitions.
   *
   * NOTE: The VAD returns a value per processed audio block, so this duration is rounded
   * to the closest model window length. For example, if the model has a processing window
   * length of 10 ms, the VAD will round up/down to the closest multiple of 10 ms.
   * Because of this, this parameter may return a different value than the one it was last set to.
   *
   * **Range:** 0.0 to 1.0 (value in seconds)
   *
   * **Default:** model-specific
   */
  AIC_VAD_PARAMETER_MINIMUM_SPEECH_DURATION = 2,
} AicVadParameter;

typedef struct AicAnalyzer AicAnalyzer;

typedef struct AicCollector AicCollector;

typedef struct AicModel AicModel;

typedef struct AicProcessor AicProcessor;

typedef struct AicProcessorContext AicProcessorContext;

typedef struct AicVad AicVad;

typedef struct AicVadContext AicVadContext;

typedef struct AicOtelConfig {
  /**
   * Whether to enable OpenTelemetry telemetry (overrides the `AIC_SDK_OTEL_ENABLE` environment variable).
   */
  bool enable;
  /**
   * Optional session ID for telemetry. If NULL, a random session ID will be generated.
   */
  const char *session_id;
  /**
   * OTel metric export interval in milliseconds. 0 uses the default (60 000 ms).
   */
  uint32_t export_interval_ms;
} AicOtelConfig;

/**
 * The result of analyzing a signal with an [`AicAnalyzer`].
 */
typedef struct AicAnalysisResult {
  /**
   * Headline audio score.
   *
   * Predicts likelihood of failure of downstream models including speech-to-text,
   * voice activity detection or turn-taking or speech-to-speech models.
   * Lower indicates less problematic audio.
   *
   * **Range:** 0.0 to 1.0
   */
  float risk_score;
  /**
   * Measure of speaker distance and reverberance.
   * Lower indicates less problematic audio.
   *
   * **Range:** 0.0 to 1.0
   */
  float speaker_reverb;
  /**
   * Measure of speaker loudness.
   *
   * **Range:** 0.0 to 1.0
   */
  float speaker_loudness;
  /**
   * Measure of interfering speech from sources other than the main speaker.
   * Lower indicates less problematic audio.
   *
   * **Range:** 0.0 to 1.0
   */
  float interfering_speech;
  /**
   * Measure of ambient or environmental noise.
   * Lower indicates less problematic audio.
   *
   * **Range:** 0.0 to 1.0
   */
  float noise;
  /**
   * Measure of artifacts introduced by lossy speech codecs,
   * e.g. from a low bitrate or a narrowband codec.
   * Lower indicates less problematic audio.
   *
   * **Range:** 0.0 to 1.0
   */
  float codec_degradation;
  /**
   * Measure of audio dropouts or discontinuities in the stream,
   * e.g. from packet loss, frame erasure, jitter or CPU overload.
   * Lower indicates less problematic audio.
   *
   * **Range:** 0.0 to 1.0
   */
  float packet_loss;
} AicAnalysisResult;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * Returns the version of the SDK.
 *
 * # Returns
 * A null-terminated C string containing the version (e.g., "1.2.3")
 *
 * # Safety
 * - The returned pointer points to a static string and remains valid
 *   for the lifetime of the program. The caller should NOT free this pointer.
 * - Real-time safe: Can be called from audio processing threads.
 * - Thread-safe: Can be called from any thread.
 */
const char *aic_get_sdk_version(void);

/**
 * Returns the model version compatible with the SDK.
 *
 * # Returns
 * Model version compatible with this version of the SDK.
 *
 * # Safety
 * - Real-time safe: Can be called from audio processing threads.
 * - Thread-safe: Can be called from any thread.
 */
uint32_t aic_get_compatible_model_version(void);

/**
 * Creates a new model instance from a model file.
 *
 * A single model instance can be used to create multiple processors, VADs or analyzers,
 * according to the model type.
 *
 * # Lifetime and ownership
 * Each processor, VAD or analyzer created with a given model handle keeps the underlying model
 * alive through internal reference counting. When the reference count reaches zero the model is destroyed.
 * You may therefore destroy the model handle before those objects, in any order.
 *
 * The model data is memory-mapped from the file, not copied into the process. It is the caller's
 * responsibility to make sure the file is not modified while the model exists.
 *
 * # Parameters
 * - `model`: Receives the handle to the newly created model. Must not be NULL.
 * - `file_path`: Null-terminated UTF-8 path to the model file. Must not be NULL.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Model created successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `model` or `file_path` is NULL
 * - `AIC_ERROR_CODE_MODEL_INVALID`: Model file is invalid or corrupted.
 * - `AIC_ERROR_CODE_MODEL_VERSION_UNSUPPORTED`: Model version is not compatible with the SDK version.
 * - `AIC_ERROR_CODE_FILE_PATH_INVALID`: Path to model file is invalid.
 * - `AIC_ERROR_CODE_FILE_SYSTEM_ERROR`: Model file could not be opened due to a file system error.
 * - `AIC_ERROR_CODE_MODEL_DATA_UNALIGNED`: Model data is not aligned to 64 bytes.
 *
 * # Safety
 * - The `model` output pointer must be valid, writable, and not aliased.
 * - The file at `file_path` must not be modified or deleted while the model, or any
 *   object created from it, is alive. Accessing the model or a derived object after the
 *   file is removed or changed is undefined behavior.
 */
enum AicErrorCode aic_model_create_from_file(struct AicModel **model,
                                             const char *file_path);

/**
 * Creates a new model instance from a memory buffer.
 *
 * A single model instance can be used to create multiple processors, VADs or analyzers,
 * according to the model type.
 *
 * # Lifetime and ownership
 * Each processor, VAD or analyzer created with a given model handle keeps the underlying model
 * alive through internal reference counting. When the reference count reaches zero the model is destroyed.
 * You may therefore destroy the model handle before those objects, in any order.
 *
 * The model data is only referenced, not copied. It is the caller's responsibility to make sure the buffer
 * is not modified while the model exists. Destroying the model does not free the memory `buffer` points to.
 *
 * # Parameters
 * - `model`: Receives the handle to the newly created model. Must not be NULL.
 * - `buffer`: Pointer to the model bytes. Must not be NULL and must be aligned to 64 bytes.
 * - `buffer_len`: Length of the model buffer in bytes.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Model created successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `model` or `buffer` is NULL
 * - `AIC_ERROR_CODE_MODEL_INVALID`: Model buffer is invalid or corrupted.
 * - `AIC_ERROR_CODE_MODEL_VERSION_UNSUPPORTED`: Model version is not compatible with the SDK version.
 * - `AIC_ERROR_CODE_MODEL_DATA_UNALIGNED`: Model data is not aligned to 64 bytes.
 *
 * # Safety
 * - The `model` output pointer must be valid, writable, and not aliased.
 * - The memory `buffer` points to must not be modified until the model handle and every object
 *   derived from it have all been destroyed.
 */
enum AicErrorCode aic_model_create_from_buffer(struct AicModel **model,
                                               const uint8_t *buffer,
                                               size_t buffer_len);

/**
 * Releases all resources associated with a model instance.
 *
 * After calling this function, the model handle becomes invalid.
 * This function is safe to call with NULL.
 *
 * # Lifetime and ownership
 * Each processor, VAD or analyzer created with a given model handle keeps the underlying model
 * alive through internal reference counting. When the reference count reaches zero the model is destroyed.
 * You may therefore destroy the model handle before those objects, in any order.
 *
 * The model data is only referenced, not copied. Destroying the `model` handle does not free its
 * backing data.
 *
 * If any objects derived from the model still exist after this call, the backing data of the model
 * being destroyed must remain valid until all the objects are destroyed too.
 *
 * # Parameters
 * - `model`: Model instance to destroy. Can be NULL.
 *
 * # Safety
 * - This function is not thread-safe. Ensure no other threads are using the model handle.
 * - The `model` pointer must have been created by
 *   `aic_model_create_from_file` or `aic_model_create_from_buffer` when non-NULL.
 * - The `model` pointer must not be used to create further objects after this call.
 */
void aic_model_destroy(struct AicModel *model);

/**
 * Returns a pointer to the model identifier.
 *
 * The returned string is UTF-8 encoded and null-terminated.
 *
 * # Parameters
 * - `model`: Model instance. Must not be NULL.
 *
 * # Returns
 * - Pointer to the null-terminated model ID string. Returns NULL if `model` is NULL.
 *
 * # Safety
 * - The pointer is only valid while the `AicModel` remains alive. Do not use it
 *   after calling `aic_model_destroy`.
 * - Read-only: do not modify or free the returned pointer.
 */
const char *aic_model_get_id(const struct AicModel *model);

/**
 * Retrieves the optimal sample rate of the model.
 *
 * Each model is optimized for a specific sample rate, which determines the frequency
 * range of the enhanced audio output. While you can process audio at any sample rate,
 * understanding the model's native rate helps predict the enhancement quality.
 *
 * **How sample rate affects enhancement:**
 *
 * - Models trained at lower sample rates (e.g., 8 kHz) can only enhance frequencies
 *   up to their Nyquist limit (4 kHz for 8 kHz models)
 * - When processing higher sample rate input (e.g., 48 kHz) with a lower-rate model,
 *   only the lower frequency components will be enhanced
 *
 * **Enhancement blending:**
 *
 * When enhancement strength is set below 1.0, the enhanced signal is blended with
 * the original, maintaining the full frequency spectrum of your input while adding
 * the model's noise reduction capabilities to the lower frequencies.
 *
 * **Sample rate and optimal block size relationship:**
 *
 * When using different sample rates than the model's native rate, the optimal samples
 * per block (returned by `aic_model_get_optimal_block_size`) will change. The processor's output
 * delay remains constant regardless of sample rate as long as you use the optimal samples
 * per block for that rate.
 *
 * **Recommendation:**
 *
 * For maximum enhancement quality across the full frequency spectrum, match your
 * input sample rate to the model's native rate when possible.
 *
 * # Parameters
 * - `model`: Model instance. Must not be NULL.
 * - `sample_rate`: Receives the optimal sample rate in Hz. Must not be NULL.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Sample rate retrieved successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `model` or `sample_rate` is NULL
 *
 * # Safety
 * - Real-time safe: Can be called from audio processing threads.
 * - Thread-safe: Can be called from any thread.
 */
enum AicErrorCode aic_model_get_optimal_sample_rate(const struct AicModel *model,
                                                    uint32_t *sample_rate);

/**
 * Retrieves the optimal block size for the model at a given sample rate.
 *
 * Using the optimal block size minimizes latency by avoiding internal buffering.
 *
 * **When you use a different block size than the optimal value, the processor will
 * introduce additional buffering latency on top of its base processing delay.**
 *
 * The optimal block size varies based on the sample rate. Each model operates on a
 * fixed time window length, so the required number of samples changes with sample rate.
 * For example, a model designed for 10 ms processing windows requires 480 samples at
 * 48 kHz, but only 160 samples at 16 kHz to capture the same duration of audio.
 *
 * Call this function with your intended sample rate before calling `aic_processor_initialize`
 * to determine the best block size for minimal latency.
 *
 * # Parameters
 * - `model`: Model instance. Must not be NULL.
 * - `sample_rate`: The sample rate in Hz for which to calculate the optimal block size.
 * - `block_size`: Receives the optimal block size. Must not be NULL.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Samples per block retrieved successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `model` or `block_size` is NULL
 *
 * # Safety
 * - Real-time safe: Can be called from audio processing threads.
 * - Thread-safe: Can be called from any thread.
 */
enum AicErrorCode aic_model_get_optimal_block_size(const struct AicModel *model,
                                                   uint32_t sample_rate,
                                                   size_t *block_size);

/**
 * Creates a new audio processor instance from an enhancement or bypass model.
 *
 * Multiple processors can be created to process different audio streams simultaneously
 * or to switch between different enhancement algorithms during runtime.
 *
 * The same `AicModel` handle may be passed to this function more than once:
 * each call creates an independent processor that shares the underlying model
 * data internally.
 *
 * # Parameters
 * - `processor`: Receives the handle to the newly created processor. Must not be NULL.
 * - `model`: Handle to the model instance to process. Must not be NULL.
 * - `license_key`: Null-terminated license key. Must not be NULL.
 * - `otel_config`: Optional pointer to OpenTelemetry configuration.
 *    If non-NULL, telemetry will be sent according to the provided configuration.
 *    Otherwise it will be configured according to the runtime environment.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Processor created successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `processor` or `model` or `license_key` is NULL
 * - `AIC_ERROR_CODE_LICENSE_FORMAT_INVALID`: License key format is incorrect
 * - `AIC_ERROR_CODE_LICENSE_VERSION_UNSUPPORTED`: License version is not compatible with the SDK version
 * - `AIC_ERROR_CODE_LICENSE_EXPIRED`: License key has expired
 * - `AIC_ERROR_CODE_MODEL_TYPE_UNSUPPORTED`: The model is not an enhancement or bypass model
 *
 * # Safety
 * - The `processor` output pointer must be valid, writable, and not aliased.
 * - The `model`'s backing buffer/file must outlive the `processor` object.
 */
enum AicErrorCode aic_processor_create(struct AicProcessor **processor,
                                       const struct AicModel *model,
                                       const char *license_key,
                                       const struct AicOtelConfig *otel_config);

/**
 * Releases all resources associated with a processor instance.
 *
 * After calling this function, the processor handle becomes invalid.
 * This function is safe to call with NULL.
 *
 * # Lifetime and ownership
 * The `processor` holds a shared reference to the model data, so it can be destroyed
 * independently of the model handle, in any order.
 * Destroying it releases that reference.
 *
 * # Parameters
 * - `processor`: Processor instance to destroy. Can be NULL.
 *
 * # Safety
 * - This function is not thread-safe. Ensure no other threads are using the
 *   processor while it is being destroyed.
 * - The `processor` pointer must have been created by `aic_processor_create` when non-NULL.
 */
void aic_processor_destroy(struct AicProcessor *processor);

/**
 * Configures the processor for a specific audio format.
 *
 * This function must be called before processing any audio.
 * For the lowest delay use the sample rate and block size returned by
 * `aic_model_get_optimal_sample_rate` and `aic_model_get_optimal_block_size`.
 *
 * # Parameters
 * - `processor`: Processor instance to configure. Must not be NULL.
 * - `sample_rate`: Audio sample rate in Hz (8000 - 192000).
 * - `block_size`: Number of samples per process call (the maximum, if `variable_block_size` is `true`).
 * - `variable_block_size`: If `true`, permits shorter calls at the cost of added delay;
 *   calls larger than `block_size` are always rejected.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Configuration accepted
 * - `AIC_ERROR_CODE_NULL_POINTER`: `processor` is NULL
 * - `AIC_ERROR_CODE_AUDIO_CONFIG_UNSUPPORTED`: Configuration is not supported
 *
 * # Safety
 * - This function allocates memory. Avoid calling it from real-time audio threads.
 * - This function is not thread-safe. Ensure no other threads are using the
 *   processor during initialization.
 */
enum AicErrorCode aic_processor_initialize(struct AicProcessor *processor,
                                           uint32_t sample_rate,
                                           size_t block_size,
                                           bool variable_block_size);

/**
 * Enhances speech in the provided audio block in-place.
 *
 * # Parameters
 * - `processor`: Initialized processor instance. Must not be NULL.
 * - `audio_ptr`: Pointer to a mono audio block of `audio_len` samples. Must not be NULL.
 * - `audio_len`: Number of samples in the block (must match `block_size` from initialization, or if `variable_block_size` was enabled, must be ≤ `block_size`).
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Audio processed successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `processor` or `audio_ptr` is NULL
 * - `AIC_ERROR_CODE_NOT_INITIALIZED`: Processor has not been initialized
 * - `AIC_ERROR_CODE_AUDIO_CONFIG_MISMATCH`: Block size mismatch
 * - `AIC_ERROR_CODE_PROCESSING_NOT_ALLOWED`: Processing is not allowed because the SDK key was not authorized or usage reporting failed.
 *
 * # Safety
 * - Real-time safe: Can be called from audio processing threads.
 * - This function is not thread-safe. Do not use the processor from any other
 *   thread while this call is active.
 */
enum AicErrorCode aic_processor_process(struct AicProcessor *processor,
                                        float *audio_ptr,
                                        size_t audio_len);

/**
 * Terminates the telemetry session associated with this processor.
 *
 * Once the request has been handled, the processor is no longer allowed to
 * process audio.
 *
 * This function is meant to be used in lifecycle management events.
 * A telemetry session is automatically stopped when a processor is destroyed.
 *
 * However, in cases where this SDK is integrated with languages with automatic
 * memory management, object deallocation could be delayed - for example, until the
 * garbage collector runs. Use this function to start termination on demand.
 *
 * This function blocks until the telemetry session is terminated, unless another
 * session is still alive. In that case, this function returns early and termination
 * happens asynchronously. This keeps lifecycle management smooth while ensuring
 * all sessions are closed when the last processor is terminated.
 *
 * # Parameters
 * - `processor`: Processor instance. Must not be NULL.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Termination requested successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `processor` is NULL
 *
 * # Safety
 * - This function is not real-time safe. It may block until the session is terminated.
 * - This function is not thread-safe. Ensure no other threads are using the
 *   processor while this call is active.
 * - The `processor` pointer must have been created by `aic_processor_create`.
 */
enum AicErrorCode aic_processor_terminate_session(struct AicProcessor *processor);

/**
 * Creates a processor context handle for thread-safe control APIs.
 *
 * Use the returned handle to reset the processor, parameter APIs,
 * and other thread-safe functions that operate on `AicProcessorContext`.
 *
 * # Parameters
 * - `context`: Receives the handle to the processor context. Must not be NULL.
 * - `processor`: Processor instance. Must not be NULL.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Context handle created successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `processor` or `context` is NULL
 *
 * # Safety
 * - The `context` output pointer must be valid, writable, and not aliased.
 */
enum AicErrorCode aic_processor_context_create(struct AicProcessorContext **context,
                                               const struct AicProcessor *processor);

/**
 * Releases a processor context handle.
 *
 * After calling this function, the context handle becomes invalid.
 * This function is safe to call with NULL.
 * Destroying the context does not destroy the associated processor.
 *
 * # Parameters
 * - `context`: Context instance to destroy. Can be NULL.
 *
 * # Safety
 * - Thread-safe with calls that use other context handles.
 * - Do not use the same context handle from another thread while this call is
 *   active.
 * - The `context` pointer must have been created by `aic_processor_context_create` when non-NULL.
 */
void aic_processor_context_destroy(struct AicProcessorContext *context);

/**
 * Clears all internal state and buffers.
 *
 * Call this when the audio stream is interrupted or when seeking
 * to prevent artifacts from previous audio content.
 *
 * This operates on the processor associated with the provided context handle.
 *
 * The processor stays initialized to the configured settings.
 *
 * # Parameters
 * - `context`: Processor context instance to reset. Must not be NULL.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: State cleared successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `context` is NULL
 *
 * # Safety
 * - Real-time safe: Can be called from audio processing threads.
 * - Thread-safe: Can be called from any thread.
 */
enum AicErrorCode aic_processor_context_reset(const struct AicProcessorContext *context);

/**
 * Modifies an enhancement parameter.
 *
 * All parameters can be changed during audio processing.
 * This function can be called from any thread.
 *
 * This operates on the processor associated with the provided context handle.
 *
 * # Parameters
 * - `context`: Processor context instance. Must not be NULL.
 * - `parameter`: Parameter to modify.
 * - `value`: New parameter value. See parameter documentation for ranges.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Parameter updated successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `context` is NULL
 * - `AIC_ERROR_CODE_PARAMETER_OUT_OF_RANGE`: Value outside valid range
 *
 * # Safety
 * - Real-time safe: Can be called from audio processing threads.
 * - Thread-safe: Can be called from any thread.
 */
enum AicErrorCode aic_processor_context_set_parameter(const struct AicProcessorContext *context,
                                                      enum AicProcessorParameter parameter,
                                                      float value);

/**
 * Retrieves the current value of a parameter.
 *
 * This function can be called from any thread.
 *
 * This queries the processor associated with the provided context handle.
 *
 * # Parameters
 * - `context`: Processor context instance. Must not be NULL.
 * - `parameter`: Parameter to query.
 * - `value`: Receives the current parameter value. Must not be NULL.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Parameter retrieved successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `context` or `value` is NULL
 *
 * # Safety
 * - Real-time safe: Can be called from audio processing threads.
 * - Thread-safe: Can be called from any thread.
 */
enum AicErrorCode aic_processor_context_get_parameter(const struct AicProcessorContext *context,
                                                      enum AicProcessorParameter parameter,
                                                      float *value);

/**
 * Returns the delay applied to the audio in samples for the current audio configuration.
 *
 * This function provides the complete end-to-end latency introduced by the processor,
 * which includes both algorithmic processing delay and any buffering overhead.
 * The processed audio leaves `aic_processor_process` this many samples behind its input.
 * Use this value to synchronize enhanced audio with other streams or to implement
 * delay compensation in your application.
 *
 * This queries the processor associated with the provided context handle.
 *
 * **Delay behavior:**
 * - **Before initialization:** Returns the base processing delay using the processor's
 *   optimal block size at its native sample rate
 * - **After initialization:** Returns the actual delay for your specific configuration,
 *   including any additional buffering introduced by a non-optimal block size
 *
 * **Important:** The delay value is always expressed in samples at the sample rate
 * you configured during `aic_processor_initialize`. To convert to time units:
 * `delay_ms = (delay_samples * 1000) / sample_rate`
 *
 * **Note:** Using a block size different from the optimal value returned by
 * `aic_model_get_optimal_block_size` will increase the delay beyond the processor's base latency.
 *
 * # Parameters
 * - `context`: Processor context instance. Must not be NULL.
 * - `delay`: Receives the delay in samples. Must not be NULL.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Delay retrieved successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `context` or `delay` is NULL
 *
 * # Safety
 * - Real-time safe: Can be called from audio processing threads.
 * - Thread-safe: Can be called from any thread.
 */
enum AicErrorCode aic_processor_context_get_audio_delay(const struct AicProcessorContext *context,
                                                        size_t *delay);

/**
 * Replaces the bearer token on a running processor.
 *
 * Use this when your license key is a JWT and needs to be refreshed
 * before it expires. Calling this with a renewed token lets you stay authenticated
 * without tearing down and recreating the processor: audio processing continues
 * uninterrupted, the context handle stays valid, and the new token is used for all
 * subsequent authentication against the ai-coustics backend.
 *
 * In-place updates are only supported when both the originally configured key and
 * the new token are JWTs. Other license types cannot be swapped in this way.
 *
 * On any error the call is a no-op: the previously active token remains in use and
 * the telemetry session is unaffected (no backoff, no interruption to processing).
 *
 * On success the swap is applied immediately and is **not** gated on backend
 * acceptance. The token is validated locally for format only; if the backend later
 * rejects it (e.g. expired or revoked), the SDK retries it under backoff rather than
 * rolling back to the prior token, and audio processing is eventually disabled if no
 * accepted token arrives in time. Supplying a known-good token via this call during
 * that window recovers the session.
 *
 * Safe to call concurrently with `aic_processor_process` on the originating
 * processor.
 *
 * # Parameters
 * - `context`: Processor context instance. Must not be NULL.
 * - `token`: Null-terminated string containing the new JWT. Must not be NULL.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Token replaced successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `context` or `token` is NULL
 * - `AIC_ERROR_CODE_LICENSE_FORMAT_INVALID`: New token could not be parsed; the existing token stays in use
 * - `AIC_ERROR_CODE_TOKEN_UPDATE_UNSUPPORTED`: The original or new key does not support in-place updates; the existing token stays in use
 *
 * # Safety
 * - This function is not real-time safe. It locks a mutex and allocates memory.
 * - Thread-safe: Can be called from any thread.
 * - The `context` pointer must have been created by `aic_processor_context_create`.
 * - `token` must point to a valid null-terminated string.
 */
enum AicErrorCode aic_processor_context_update_bearer_token(const struct AicProcessorContext *context,
                                                            const char *token);

/**
 * Creates a new voice activity detector instance from a VAD model.
 *
 * Multiple VAD instances can be created to process different audio streams simultaneously.
 *
 * The same `AicModel` handle may be passed to this function more than once:
 * each call creates an independent VAD that shares the underlying model data
 * internally.
 *
 * # Parameters
 * - `vad`: Receives the handle to the newly created VAD. Must not be NULL.
 * - `model`: Handle to the model instance to process. Must not be NULL.
 * - `license_key`: Null-terminated license key. Must not be NULL.
 * - `otel_config`: Optional pointer to OpenTelemetry configuration.
 *    If non-NULL, telemetry will be sent according to the provided configuration.
 *    Otherwise it will be configured according to the runtime environment.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: VAD created successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `vad` or `model` or `license_key` is NULL
 * - `AIC_ERROR_CODE_LICENSE_FORMAT_INVALID`: License key format is incorrect
 * - `AIC_ERROR_CODE_LICENSE_VERSION_UNSUPPORTED`: License version is not compatible with the SDK version
 * - `AIC_ERROR_CODE_LICENSE_EXPIRED`: License key has expired
 * - `AIC_ERROR_CODE_MODEL_TYPE_UNSUPPORTED`: The model is not a VAD model
 *
 * # Safety
 * - The `vad` output pointer must be valid, writable, and not aliased.
 * - The `model`'s backing buffer/file must outlive the `vad` object.
 */
enum AicErrorCode aic_vad_create(struct AicVad **vad,
                                 const struct AicModel *model,
                                 const char *license_key,
                                 const struct AicOtelConfig *otel_config);

/**
 * Releases all resources associated with a VAD instance.
 *
 * After calling this function, the VAD handle becomes invalid.
 * This function is safe to call with NULL.
 *
 * # Lifetime and ownership
 * The `vad` holds a shared reference to the model data, so it can be destroyed
 * independently of the model handle, in any order.
 * Destroying it releases that reference.
 *
 * # Parameters
 * - `vad`: VAD instance to destroy. Can be NULL.
 *
 * # Safety
 * - This function is not thread-safe. Ensure no other threads are using the
 *   VAD while it is being destroyed.
 * - The `vad` pointer must have been created by `aic_vad_create` when non-NULL.
 */
void aic_vad_destroy(struct AicVad *vad);

/**
 * Configures the VAD for a specific audio format.
 *
 * This function must be called before processing any audio.
 * For the most frequent prediction updates, use the sample rate and block size returned by
 * `aic_model_get_optimal_sample_rate` and `aic_model_get_optimal_block_size`.
 *
 * # Parameters
 * - `vad`: VAD instance to configure. Must not be NULL.
 * - `sample_rate`: Audio sample rate in Hz (8000 - 192000).
 * - `block_size`: Number of samples per process call (the maximum, if `variable_block_size` is `true`).
 * - `variable_block_size`: If `true`, permits shorter calls at the cost of extra buffering
 *   before new predictions are published; calls larger than `block_size` are always rejected.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Configuration accepted
 * - `AIC_ERROR_CODE_NULL_POINTER`: `vad` is NULL
 * - `AIC_ERROR_CODE_AUDIO_CONFIG_UNSUPPORTED`: Configuration is not supported
 *
 * # Safety
 * - This function allocates memory. Avoid calling it from real-time audio threads.
 * - This function is not thread-safe. Ensure no other threads are using the
 *   VAD during initialization.
 */
enum AicErrorCode aic_vad_initialize(struct AicVad *vad,
                                     uint32_t sample_rate,
                                     size_t block_size,
                                     bool variable_block_size);

/**
 * Processes the provided mono audio block and updates the VAD prediction.
 *
 * This function does not modify the input audio block.
 *
 * **Recommendation:** When enhancement and VAD run together, pass the original input audio
 * here, not the output of `aic_processor_process`. Enhancement is designed to change the
 * signal, so running the VAD on its output means detecting speech in audio that no longer
 * matches what the VAD model expects, and it stacks the processor's audio delay on top of the
 * VAD's prediction delay. Because this function does not modify its input, calling it on the
 * same buffer before `aic_processor_process` is enough.
 *
 * # Parameters
 * - `vad`: Initialized VAD instance. Must not be NULL.
 * - `audio_ptr`: Pointer to a mono audio block of `audio_len` samples. Must not be NULL.
 * - `audio_len`: Number of samples in the block (must match `block_size` from initialization, or if `variable_block_size` was enabled, must be ≤ `block_size`).
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Audio processed successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `vad` or `audio_ptr` is NULL
 * - `AIC_ERROR_CODE_NOT_INITIALIZED`: VAD has not been initialized
 * - `AIC_ERROR_CODE_AUDIO_CONFIG_MISMATCH`: Block size mismatch
 * - `AIC_ERROR_CODE_PROCESSING_NOT_ALLOWED`: Processing is not allowed because the SDK key was not authorized or usage reporting failed.
 *
 * # Safety
 * - Real-time safe: Can be called from audio processing threads.
 * - This function is not thread-safe. Do not use the VAD from any other
 *   thread while this call is active.
 */
enum AicErrorCode aic_vad_process(struct AicVad *vad,
                                  const float *audio_ptr,
                                  size_t audio_len);

/**
 * Terminates the telemetry session associated with this VAD.
 *
 * Once the request has been handled, the VAD is no longer allowed to process audio.
 *
 * This function is meant to be used in lifecycle management events.
 * A telemetry session is automatically stopped when a VAD is destroyed.
 *
 * However, in cases where this SDK is integrated with languages with automatic
 * memory management, object deallocation could be delayed - for example, until the
 * garbage collector runs. Use this function to start termination on demand.
 *
 * This function blocks until the telemetry session is terminated, unless another
 * session is still alive. In that case, this function returns early and termination
 * happens asynchronously. This keeps lifecycle management smooth while ensuring
 * all sessions are closed when the last VAD is terminated.
 *
 * # Parameters
 * - `vad`: VAD instance. Must not be NULL.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Termination requested successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `vad` is NULL
 *
 * # Safety
 * - This function is not real-time safe. It may block until the session is terminated.
 * - This function is not thread-safe. Ensure no other threads are using the
 *   VAD while this call is active.
 * - The `vad` pointer must have been created by `aic_vad_create`.
 */
enum AicErrorCode aic_vad_terminate_session(struct AicVad *vad);

/**
 * Creates a VAD context handle for thread-safe control APIs.
 *
 * The voice activity detection works automatically as `aic_vad_process` processes audio.
 *
 * This uses the VAD associated with the provided VAD handle.
 * All handles created from a given VAD reference the same VAD instance.
 *
 * **Important:** If the backing VAD is destroyed, the VAD context will stop
 * producing new data. It is safe to destroy the VAD without destroying the context.
 *
 * # Parameters
 * - `context`: VAD context instance. Must not be NULL.
 * - `vad`: VAD instance to use as data source. Must not be NULL.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Context handle created successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `context` or `vad` is NULL
 *
 * # Safety
 * - The `context` output pointer must be valid, writable, and not aliased.
 */
enum AicErrorCode aic_vad_context_create(struct AicVadContext **context, const struct AicVad *vad);

/**
 * Releases a VAD context handle.
 *
 * **Important:** This does **NOT** destroy the backing VAD.
 * `aic_vad_destroy` must be called separately.
 *
 * After calling this function, the context handle becomes invalid.
 * This function is safe to call with NULL.
 *
 * # Parameters
 * - `context`: VAD context instance. Can be NULL.
 *
 * # Safety
 * - Thread-safe with calls that use other VAD context handles.
 * - Do not use the same context handle from another thread while this call is
 *   active.
 * - The `context` pointer must have been created by `aic_vad_context_create` when non-NULL.
 */
void aic_vad_context_destroy(struct AicVadContext *context);

/**
 * Clears all internal state and buffers. This also resets the VAD state.
 *
 * Call this when the audio stream is interrupted or when seeking
 * to prevent mispredictions from previous audio content.
 *
 * This operates on the VAD associated with the provided context handle.
 *
 * The VAD stays initialized to the configured settings.
 *
 * # Parameters
 * - `context`: VAD context instance to reset. Must not be NULL.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: State cleared successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `context` is NULL
 *
 * # Safety
 * - Real-time safe: Can be called from audio processing threads.
 * - Thread-safe: Can be called from any thread.
 */
enum AicErrorCode aic_vad_context_reset(const struct AicVadContext *context);

/**
 * Returns the total VAD prediction delay in samples for the current audio configuration.
 *
 * This function provides the complete end-to-end latency of the VAD prediction,
 * which includes input reblocking, STFT, and model processing delay. Use this value
 * to line up VAD decisions with the input timeline.
 *
 * This delay is **not** applied to the audio: `aic_vad_process` leaves its input buffer
 * untouched. The value only describes how far behind its input the published prediction is.
 *
 * When enhancement and VAD run together, feed the VAD the original input audio rather than the
 * processor's output. This value is then the prediction's delay relative to that input, and it is
 * independent of the processor's `aic_processor_context_get_audio_delay`.
 *
 * This queries the VAD associated with the provided context handle.
 *
 * **Delay behavior:**
 * - **Before initialization:** Returns the base processing delay using the VAD's
 *   optimal block size at its native sample rate
 * - **After initialization:** Returns the end-to-end VAD prediction delay at the
 *   initialized sample rate, including adapter input-buffering latency for the
 *   configured block size
 *
 * **Important:** The delay value is always expressed in samples at the sample rate
 * you configured during `aic_vad_initialize`. To convert to time units:
 * `delay_ms = (delay_samples * 1000) / sample_rate`
 *
 * **Note:** Using a block size different from the optimal value returned by
 * `aic_model_get_optimal_block_size`, or enabling variable block sizes, can add
 * input-buffering latency before a new VAD prediction is published. That
 * latency is included in the reported delay.
 *
 * # Parameters
 * - `context`: VAD context instance. Must not be NULL.
 * - `delay`: Receives the delay in samples. Must not be NULL.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Delay retrieved successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `context` or `delay` is NULL
 *
 * # Safety
 * - Real-time safe: Can be called from audio processing threads.
 * - Thread-safe: Can be called from any thread.
 */
enum AicErrorCode aic_vad_context_get_prediction_delay(const struct AicVadContext *context,
                                                       size_t *delay);

/**
 * Replaces the bearer token on a running VAD.
 *
 * Use this when your license key is a JWT and needs to be refreshed
 * before it expires. Calling this with a renewed token lets you stay authenticated
 * without tearing down and recreating the VAD: audio processing continues
 * uninterrupted, the context handle stays valid, and the new token is used for all
 * subsequent authentication against the ai-coustics backend.
 *
 * In-place updates are only supported when both the originally configured key and
 * the new token are JWTs. Other license types cannot be swapped in this way.
 *
 * On any error the call is a no-op: the previously active token remains in use and
 * the telemetry session is unaffected (no backoff, no interruption to processing).
 *
 * On success the swap is applied immediately and is **not** gated on backend
 * acceptance. The token is validated locally for format only; if the backend later
 * rejects it (e.g. expired or revoked), the SDK retries it under backoff rather than
 * rolling back to the prior token, and audio processing is eventually disabled if no
 * accepted token arrives in time. Supplying a known-good token via this call during
 * that window recovers the session.
 *
 * Safe to call concurrently with `aic_vad_process` on the originating VAD.
 *
 * # Parameters
 * - `context`: VAD context instance. Must not be NULL.
 * - `token`: Null-terminated string containing the new JWT. Must not be NULL.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Token replaced successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `context` or `token` is NULL
 * - `AIC_ERROR_CODE_LICENSE_FORMAT_INVALID`: New token could not be parsed; the existing token stays in use
 * - `AIC_ERROR_CODE_TOKEN_UPDATE_UNSUPPORTED`: The original or new key does not support in-place updates; the existing token stays in use
 *
 * # Safety
 * - This function is not real-time safe. It locks a mutex and allocates memory.
 * - Thread-safe: Can be called from any thread.
 * - The `context` pointer must have been created by `aic_vad_context_create`.
 * - `token` must point to a valid null-terminated string.
 */
enum AicErrorCode aic_vad_context_update_bearer_token(const struct AicVadContext *context,
                                                      const char *token);

/**
 * Returns the VAD's prediction.
 *
 * # Latency
 * The latency of the VAD prediction is equal to the backing VAD's processing latency,
 * reported by `aic_vad_context_get_prediction_delay`. The prediction lags its input by
 * that many samples. Align speech decisions to the input timeline using that delay.
 *
 * If the backing VAD stops being processed, the VAD will not update its prediction.
 *
 * # Parameters
 * - `context`: VAD context instance. Must not be NULL.
 * - `value`: Receives the VAD prediction. Must not be NULL.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Prediction retrieved successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `context` or `value` is NULL
 *
 * # Safety
 * - Real-time safe: Can be called from audio processing threads.
 * - Thread-safe: Can be called from any thread.
 */
enum AicErrorCode aic_vad_context_is_speech_detected(const struct AicVadContext *context,
                                                     bool *value);

/**
 * Returns the raw prediction of the VAD, without any processing.
 *
 * In contrast to the output of `aic_vad_context_is_speech_detected`,
 * the output of this function is the model's direct prediction without
 * going through the SDK's VAD post-processing (i.e. speech hold duration,
 * sensitivity thresholding, etc.).
 *
 * This value may be used to build other abstractions on top of this data.
 *
 * # Latency
 * The latency of the VAD prediction is equal to the backing VAD's processing latency,
 * reported by `aic_vad_context_get_prediction_delay`. The prediction lags its input by
 * that many samples. Align speech decisions to the input timeline using that delay.
 *
 * If the backing VAD stops being processed, the VAD will not update its prediction.
 *
 * # Parameters
 * - `context`: VAD context instance. Must not be NULL.
 * - `value`: Receives the VAD prediction. Must not be NULL.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Prediction retrieved successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `context` or `value` is NULL
 *
 * # Safety
 * - Real-time safe: Can be called from audio processing threads.
 * - Thread-safe: Can be called from any thread.
 */
enum AicErrorCode aic_vad_context_get_raw_vad_probability(const struct AicVadContext *context,
                                                          float *value);

/**
 * Modifies a VAD parameter.
 *
 * All parameters can be changed during audio processing.
 * This function can be called from any thread.
 *
 * # Parameters
 * - `context`: VAD context instance. Must not be NULL.
 * - `parameter`: Parameter to modify.
 * - `value`: New parameter value. See parameter documentation for ranges.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Parameter updated successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `context` is NULL
 * - `AIC_ERROR_CODE_PARAMETER_OUT_OF_RANGE`: Value outside valid range
 *
 * # Safety
 * - Real-time safe: Can be called from audio processing threads.
 * - Thread-safe: Can be called from any thread.
 */
enum AicErrorCode aic_vad_context_set_parameter(const struct AicVadContext *context,
                                                enum AicVadParameter parameter,
                                                float value);

/**
 * Retrieves the current value of a parameter.
 *
 * This function can be called from any thread.
 *
 * # Parameters
 * - `context`: VAD context instance. Must not be NULL.
 * - `parameter`: Parameter to query.
 * - `value`: Receives the current parameter value. Must not be NULL.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Parameter retrieved successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `context` or `value` is NULL
 *
 * # Safety
 * - Real-time safe: Can be called from audio processing threads.
 * - Thread-safe: Can be called from any thread.
 */
enum AicErrorCode aic_vad_context_get_parameter(const struct AicVadContext *context,
                                                enum AicVadParameter parameter,
                                                float *value);

/**
 * Creates a collector/analyzer pair for non-real-time analysis.
 *
 * The collector is designed to be placed in the audio thread, buffering audio chunks for
 * later analysis.
 *
 * The analyzer is designed to be run separately. Analysis models are computationally expensive
 * and cannot run in the audio thread. The analyzer has access to the audio buffered by the
 * collector, and it can access it safely across threads.
 *
 * The collector retains a span of audio determined by the analysis model. As more samples
 * get collected, old audio is discarded.
 *
 * The same `AicModel` handle may be passed to this function more than once:
 * each call creates an independent collector/analyzer pair whose analyzer
 * shares the underlying model data internally.
 *
 * # Lifetime and ownership
 * The analyzer shares ownership of the model data through internal reference
 * counting, so `model` need not outlive it: you may destroy the model handle (via
 * `aic_model_destroy`) before the analyzer, in any order.
 * The collector holds no reference to `model` at all.
 *
 * # Parameters
 * - `collector`: Out-pointer that receives the created collector handle. Must not be NULL.
 * - `analyzer`: Out-pointer that receives the created analyzer handle. Must not be NULL.
 * - `model`: Model to analyze with. Must not be NULL.
 * - `license_key`: Null-terminated license key. Must not be NULL.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Collector and analyzer created
 * - `AIC_ERROR_CODE_NULL_POINTER`: `collector`, `analyzer`, `model`, or `license_key` is NULL
 * - `AIC_ERROR_CODE_MODEL_TYPE_UNSUPPORTED`: `model` is not an analysis model
 * - license/model errors as for `aic_processor_create`
 *
 * # Safety
 * - This function allocates memory. Avoid calling it from real-time audio threads.
 * - The `collector` and `analyzer` output pointers must be valid, writable,
 *   and not aliased.
 * - The `model`'s backing buffer/file must outlive the `collector`/`analyzer` objects.
 */
enum AicErrorCode aic_analyzer_pair_create(struct AicCollector **collector,
                                           struct AicAnalyzer **analyzer,
                                           const struct AicModel *model,
                                           const char *license_key);

/**
 * Configures the collector for a specific audio format.
 *
 * This function must be called before buffering any audio.
 * Using the sample rate and block size returned by
 * `aic_model_get_optimal_sample_rate` and `aic_model_get_optimal_block_size`
 * avoids internal resampling and rebuffering.
 *
 * # Parameters
 * - `collector`: Collector instance to configure. Must not be NULL.
 * - `sample_rate`: Audio sample rate in Hz (8000 - 192000).
 * - `block_size`: Number of samples per call to `aic_collector_buffer` (the maximum, if `variable_block_size` is `true`).
 * - `variable_block_size`: If `true`, permits shorter calls at the cost of added delay;
 *   calls larger than `block_size` are always rejected.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Configuration accepted
 * - `AIC_ERROR_CODE_NULL_POINTER`: `collector` is NULL
 * - `AIC_ERROR_CODE_AUDIO_CONFIG_UNSUPPORTED`: Configuration is not supported
 *
 * # Safety
 * - This function allocates memory. Avoid calling it from real-time audio threads.
 * - This function is not thread-safe. Ensure no other threads are using the
 *   collector during initialization.
 */
enum AicErrorCode aic_collector_initialize(struct AicCollector *collector,
                                           uint32_t sample_rate,
                                           size_t block_size,
                                           bool variable_block_size);

/**
 * Buffers audio for later offline use.
 *
 * # Parameters
 * - `collector`: Initialized collector instance. Must not be NULL.
 * - `audio_ptr`: Pointer to a mono audio block of `audio_len` samples. Must not be NULL.
 * - `audio_len`: Number of samples in the block (must match `block_size` from initialization, or if `variable_block_size` was enabled, must be ≤ `block_size`).
 *
 * # Note
 * Input audio is read-only and is not modified.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Audio buffered successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `collector` or `audio_ptr` is NULL
 * - `AIC_ERROR_CODE_NOT_INITIALIZED`: Collector has not been initialized
 * - `AIC_ERROR_CODE_AUDIO_CONFIG_MISMATCH`: Block size mismatch
 *
 * # Safety
 * - Real-time safe: Can be called from audio processing threads.
 * - This function is not thread-safe. Do not use the collector from any other
 *   thread while this call is active.
 */
enum AicErrorCode aic_collector_buffer(struct AicCollector *collector,
                                       const float *audio_ptr,
                                       size_t audio_len);

/**
 * Releases all resources associated with a collector instance.
 *
 * After calling this function, the collector handle becomes invalid.
 * This function is safe to call with NULL.
 *
 * # Lifetime and ownership
 * The collector holds no reference to its model, so it can be destroyed
 * independently of the model and its paired analyzer, in any order.
 *
 * # Parameters
 * - `collector`: Collector instance to destroy. Can be NULL.
 *
 * # Safety
 * - This function is not thread-safe. Ensure no other threads are using the
 *   collector while this call is active.
 * - The `collector` pointer must have been created by `aic_analyzer_pair_create` when non-NULL.
 */
void aic_collector_destroy(struct AicCollector *collector);

/**
 * Clears all internal state and buffers.
 *
 * Call this when the audio stream is interrupted or when seeking
 * to prevent mispredictions from previous audio content.
 *
 * This operates on both the analyzer and its collector.
 *
 * The collector stays initialized to the configured settings.
 *
 * # Parameters
 * - `analyzer`: Analyzer instance to reset. Must not be NULL.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: State cleared successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `analyzer` is NULL
 *
 * # Safety
 * - Real-time safe: Can be called from audio processing threads.
 * - Thread-safe: Can be called from any thread.
 */
enum AicErrorCode aic_analyzer_reset(const struct AicAnalyzer *analyzer);

/**
 * Analyze the buffered signal.
 *
 * The analyzer runs a forward-pass of the analysis model with a fixed length of audio,
 * determined by the model.
 *
 * If this function is called before the collector has buffered that length of audio,
 * the analyzer will run the analysis with silence (zeros) in the tail of the input.
 *
 * # Parameters
 * - `analyzer`: Analyzer instance. Must not be NULL.
 * - `result`: Receives the analysis scores. Must not be NULL.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Analysis completed successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `analyzer` or `result` is NULL
 * - `AIC_ERROR_CODE_PROCESSING_NOT_ALLOWED`: Processing is not allowed because the SDK key was not authorized or usage reporting failed.
 *
 * # Safety
 * - This function is not real-time safe. Avoid calling it from real-time audio threads.
 * - This function is not thread-safe. Do not use the analyzer from any other
 *   thread while this call is active. `aic_collector_buffer` may run
 *   concurrently on the paired collector.
 */
enum AicErrorCode aic_analyzer_analyze_buffered(struct AicAnalyzer *analyzer,
                                                struct AicAnalysisResult *result);

/**
 * Terminates the telemetry session associated with this analyzer.
 *
 * Once the request has been handled, the analyzer is no longer allowed to
 * analyze buffered audio.
 *
 * This function is meant to be used in lifecycle management events.
 * A telemetry session is automatically stopped when an analyzer is destroyed.
 *
 * However, in cases where this SDK is integrated with languages with automatic
 * memory management, object deallocation could be delayed - for example, until the
 * garbage collector runs. Use this function to start termination on demand.
 *
 * This function blocks until the telemetry session is terminated, unless another
 * session is still alive. In that case, this function returns early and termination
 * happens asynchronously. This keeps lifecycle management smooth while ensuring
 * all sessions are closed when the last telemetry session is terminated.
 *
 * # Parameters
 * - `analyzer`: Analyzer instance. Must not be NULL.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Termination requested successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `analyzer` is NULL
 *
 * # Safety
 * - This function is not real-time safe. It may block until the session is terminated.
 * - This function is not thread-safe. Ensure no other threads are using the
 *   analyzer while this call is active.
 * - The `analyzer` pointer must have been created by `aic_analyzer_pair_create`.
 */
enum AicErrorCode aic_analyzer_terminate_session(struct AicAnalyzer *analyzer);

/**
 * Replaces the bearer token on a running analyzer.
 *
 * Use this when your license key is a JWT and needs to be refreshed
 * before it expires. Calling this with a renewed token lets you stay authenticated
 * without tearing down and recreating the analyzer: the analyzer handle stays valid,
 * buffered audio remains available, and the new token is used for all
 * subsequent authentication against the ai-coustics backend.
 *
 * In-place updates are only supported when both the originally configured key and
 * the new token are JWTs. Other license types cannot be swapped in this way.
 *
 * On any error the call is a no-op: the previously active token remains in use and
 * the telemetry session is unaffected (no backoff, no interruption to processing).
 *
 * On success the swap is applied immediately and is **not** gated on backend
 * acceptance. The token is validated locally for format only; if the backend later
 * rejects it (e.g. expired or revoked), the SDK retries it under backoff rather than
 * rolling back to the prior token, and analysis calls may be rejected if no
 * accepted token arrives in time. Supplying a known-good token via this call
 * during that window recovers the session.
 *
 * Safe to call concurrently with collector buffering.
 *
 * # Parameters
 * - `analyzer`: Analyzer instance. Must not be NULL.
 * - `token`: Null-terminated string containing the new JWT. Must not be NULL.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Token replaced successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `analyzer` or `token` is NULL
 * - `AIC_ERROR_CODE_LICENSE_FORMAT_INVALID`: New token could not be parsed; the existing token stays in use
 * - `AIC_ERROR_CODE_TOKEN_UPDATE_UNSUPPORTED`: The original or new key does not support in-place updates; the existing token stays in use
 *
 * # Safety
 * - This function is not real-time safe. It locks a mutex and allocates memory.
 * - The `analyzer` pointer must have been created by `aic_analyzer_pair_create`.
 * - `token` must point to a valid null-terminated string.
 */
enum AicErrorCode aic_analyzer_update_bearer_token(const struct AicAnalyzer *analyzer,
                                                   const char *token);

/**
 * Releases all resources associated with an analyzer instance.
 *
 * After calling this function, the analyzer handle becomes invalid.
 * This function is safe to call with NULL.
 *
 * # Lifetime and ownership
 * The `analyzer` holds a shared reference to the model data, so it can be destroyed
 * independently of the model handle and its paired collector, in any order.
 * Destroying it releases that reference.
 *
 * # Parameters
 * - `analyzer`: Analyzer instance to destroy. Can be NULL.
 *
 * # Safety
 * - This function is not thread-safe. Ensure no other threads are using the
 *   analyzer while this call is active.
 * - The `analyzer` pointer must have been created by `aic_analyzer_pair_create` when non-NULL.
 */
void aic_analyzer_destroy(struct AicAnalyzer *analyzer);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  /* AIC_H */
