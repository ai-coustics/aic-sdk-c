/**
 * This file contains the definitions and declarations for the ai-coustics
 * speech enhancement SDK, including initialization, processing, and configuration
 * functions. The ai-coustics SDK provides advanced machine learning models for
 * speech enhancement, that can be used in audio streaming contexts.
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
   * Model must be initialized before calling this operation. Call `aic_processor_initialize` first.
   */
  AIC_ERROR_CODE_MODEL_NOT_INITIALIZED = 3,
  /**
   * Audio configuration (samplerate, num_channels, num_frames) is not supported by the model
   */
  AIC_ERROR_CODE_AUDIO_CONFIG_UNSUPPORTED = 4,
  /**
   * Audio buffer configuration differs from the one provided during initialization
   */
  AIC_ERROR_CODE_AUDIO_CONFIG_MISMATCH = 5,
  /**
   * SDK key was not authorized or process failed to report usage. Check if you have internet connection.
   */
  AIC_ERROR_CODE_ENHANCEMENT_NOT_ALLOWED = 6,
  /**
   * Internal error occurred. Contact support.
   */
  AIC_ERROR_CODE_INTERNAL_ERROR = 7,
  /**
   * The requested parameter is read-only for this model type and cannot be modified.
   */
  AIC_ERROR_CODE_PARAMETER_FIXED = 8,
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
   * The model file is invalid or corrupted. Verify the file is correct.
   */
  AIC_ERROR_CODE_MODEL_INVALID = 100,
  /**
   * The model file version is not compatible with this SDK version.
   */
  AIC_ERROR_CODE_MODEL_VERSION_UNSUPPORTED = 101,
  /**
   * The path to the model file is invalid.
   */
  AIC_ERROR_CODE_MODEL_FILE_PATH_INVALID = 102,
  /**
   * The model file cannot be opened due to a filesystem error. Verify that the file exists.
   */
  AIC_ERROR_CODE_FILE_SYSTEM_ERROR = 103,
  /**
   * The model data is not aligned to 64 bytes.
   */
  AIC_ERROR_CODE_MODEL_DATA_UNALIGNED = 104,
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
   * Controls the intensity of speech enhancement processing.
   *
   * **Range:** 0.0 to 1.0
   * - **0.0:** No enhancement - original signal passes through unchanged
   * - **1.0:** Full enhancement - maximum noise reduction but also more audible artifacts
   *
   * **Default:** 1.0
   */
  AIC_PROCESSOR_PARAMETER_ENHANCEMENT_LEVEL = 1,
  /**
   * Compensates for perceived volume reduction after noise removal.
   *
   * **Range:** 0.1 to 4.0 (linear amplitude multiplier)
   * - **0.1:** Significant volume reduction (-20 dB)
   * - **1.0:** No gain change (0 dB, default)
   * - **2.0:** Double amplitude (+6 dB)
   * - **4.0:** Maximum boost (+12 dB)
   *
   * **Formula:** Gain (dB) = 20 × log₁₀(value)
   * **Default:** 1.0
   */
  AIC_PROCESSOR_PARAMETER_VOICE_GAIN = 2,
} AicProcessorParameter;

/**
 * Configurable parameters for Voice Activity Detection.
 */
typedef enum AicVadParameter {
  /**
   * Controls for how long the VAD continues to detect speech after the audio signal
   * no longer contains speech.
   *
   * The VAD reports speech detected if the audio signal contained speech in at least 50%
   * of the frames processed in the last `speech_hold_duration` seconds.
   *
   * This affects the stability of speech detected -> not detected transitions.
   *
   * NOTE: The VAD returns a value per processed buffer, so this duration is rounded
   * to the closest model window length. For example, if the model has a processing window
   * length of 10 ms, the VAD will round up/down to the closest multiple of 10 ms.
   * Because of this, this parameter may return a different value than the one it was last set to.
   *
   * **Range:** 0.0 to 20x model window length (value in seconds)
   *
   * **Default:** 0.05 (50 ms)
   */
  AIC_VAD_PARAMETER_SPEECH_HOLD_DURATION = 0,
  /**
   * Controls the sensitivity (energy threshold) of the VAD.
   *
   * This value is used by the VAD as the threshold a
   * speech audio signal's energy has to exceed in order to be
   * considered speech.
   *
   * **Range:** 1.0 to 15.0
   *
   * **Formula:** Energy threshold = 10 ^ (-sensitivity)
   *
   * **Default:** 6.0
   */
  AIC_VAD_PARAMETER_SENSITIVITY = 1,
  /**
   * Controls for how long speech needs to be present in the audio signal before
   * the VAD considers it speech.
   *
   * This affects the stability of speech not detected -> detected transitions.
   *
   * NOTE: The VAD returns a value per processed buffer, so this duration is rounded
   * to the closest model window length. For example, if the model has a processing window
   * length of 10 ms, the VAD will round up/down to the closest multiple of 10 ms.
   * Because of this, this parameter may return a different value than the one it was last set to.
   *
   * **Range:** 0.0 to 1.0 (value in seconds)
   *
   * **Default:** 0.0
   */
  AIC_VAD_PARAMETER_MINIMUM_SPEECH_DURATION = 2,
} AicVadParameter;

typedef struct AicModel AicModel;

typedef struct AicProcessor AicProcessor;

typedef struct AicProcessorContext AicProcessorContext;

typedef struct AicVadContext AicVadContext;

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
 * Creates a new model instance.
 *
 * A single model instance can be used to create multiple processors.
 *
 * # Note
 * Processor instances retain a shared reference to the model data.
 * It is safe to destroy the model handle after creating the desired processors.
 * The memory used by the model will be automatically freed after all processors
 * using that model have been destroyed.
 *
 * # Parameters
 * - `model`: Receives the handle to the newly created model. Must not be NULL.
 * - `file_path`: NULL-terminated string containing the path to the model file. Must not be NULL.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Model created successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `model` or `file_path` is NULL
 * - `AIC_ERROR_CODE_MODEL_INVALID`: Model file is invalid or corrupted.
 * - `AIC_ERROR_CODE_MODEL_VERSION_UNSUPPORTED`: Model version is not compatible with the SDK version.
 * - `AIC_ERROR_CODE_MODEL_FILE_PATH_INVALID`: Path to model file is invalid.
 * - `AIC_ERROR_CODE_FILE_SYSTEM_ERROR`: Model file could not be opened due to a file system error.
 * - `AIC_ERROR_CODE_MODEL_DATA_UNALIGNED`: Model data is not aligned to 64 bytes.
 *
 * # Safety
 * - This function is not thread-safe. Ensure no other threads are using the model handle or the same file path.
 */
enum AicErrorCode aic_model_create_from_file(struct AicModel **model,
                                             const char *file_path);

/**
 * Creates a new model instance from a memory buffer.
 *
 * The buffer must remain valid and unchanged for the lifetime of the model.
 *
 * # Note
 * Processor instances retain a shared reference to the model data.
 * It is safe to destroy the model handle after creating the desired processors.
 * The memory used by the model will be automatically freed after all processors
 * using that model have been destroyed.
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
 * - This function is not thread-safe. Ensure no other threads are using the model handle.
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
 * # Note
 * Processor instances retain a shared reference to the model data.
 * It is safe to destroy the model handle after creating the desired processors.
 *
 * The memory used by the model will be automatically freed after all processors
 * using that model have been destroyed. If all processors using this model handle
 * have already been destroyed, calling this function frees the memory used by the model.
 *
 * # Parameters
 * - `model`: Model instance to destroy. Can be NULL.
 *
 * # Safety
 * - This function is not thread-safe. Ensure no other threads are using the model handle.
 * - The `model` pointer must have been created by
 *   `aic_model_create_from_file` or `aic_model_create_from_buffer` when non-NULL.
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
 * - Not thread-safe with concurrent model destruction. Ensure no other thread can
 *   destroy the model while this pointer is in use.
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
 * **Sample rate and optimal frames relationship:**
 *
 * When using different sample rates than the model's native rate, the optimal number
 * of frames (returned by `aic_model_get_optimal_num_frames`) will change. The processor's output
 * delay remains constant regardless of sample rate as long as you use the optimal frame
 * count for that rate.
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
 * Retrieves the optimal number of frames for the model at a given sample rate.
 *
 * Using the optimal number of frames minimizes latency by avoiding internal buffering.
 *
 * **When you use a different frame count than the optimal value, the processor will
 * introduce additional buffering latency on top of its base processing delay.**
 *
 * The optimal frame count varies based on the sample rate. Each model operates on a
 * fixed time window length, so the required number of frames changes with sample rate.
 * For example, a model designed for 10 ms processing windows requires 480 frames at
 * 48 kHz, but only 160 frames at 16 kHz to capture the same duration of audio.
 *
 * Call this function with your intended sample rate before calling `aic_processor_initialize`
 * to determine the best frame count for minimal latency.
 *
 * # Parameters
 * - `model`: Model instance. Must not be NULL.
 * - `sample_rate`: The sample rate in Hz for which to calculate the optimal frame count.
 * - `num_frames`: Receives the optimal frame count. Must not be NULL.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Frame count retrieved successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `model` or `num_frames` is NULL
 *
 * # Safety
 * - Real-time safe: Can be called from audio processing threads.
 * - Thread-safe: Can be called from any thread.
 */
enum AicErrorCode aic_model_get_optimal_num_frames(const struct AicModel *model,
                                                   uint32_t sample_rate,
                                                   size_t *num_frames);

/**
 * Creates a new audio processor instance.
 *
 * Multiple processors can be created to process different audio streams simultaneously
 * or to switch between different enhancement algorithms during runtime.
 *
 * # Parameters
 * - `processor`: Receives the handle to the newly created processor. Must not be NULL.
 * - `model`: Handle to the model instance to process. Must not be NULL.
 * - `license_key`: NULL-terminated string containing your license key. Must not be NULL.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Processor created successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `processor` or `model` or `license_key` is NULL
 * - `AIC_ERROR_CODE_LICENSE_INVALID`: License key format is incorrect
 * - `AIC_ERROR_CODE_LICENSE_VERSION_UNSUPPORTED`: License version is not compatible with the SDK version
 * - `AIC_ERROR_CODE_LICENSE_EXPIRED`: License key has expired
 *
 * # Safety
 * - This function is not thread-safe. Ensure no other threads are using the processor handle.
 */
enum AicErrorCode aic_processor_create(struct AicProcessor **processor,
                                       const struct AicModel *model,
                                       const char *license_key);

/**
 * Releases all resources associated with a processor instance.
 *
 * After calling this function, the processor handle becomes invalid.
 * This function is safe to call with NULL.
 *
 * # Parameters
 * - `processor`: Processor instance to destroy. Can be NULL.
 *
 * # Safety
 * - This function is not thread-safe. Ensure no other threads are using the processor during initialization.
 * - The `processor` pointer must have been created by `aic_processor_create` when non-NULL.
 */
void aic_processor_destroy(struct AicProcessor *processor);

/**
 * Configures the processor for a specific audio format.
 *
 * This function must be called before processing any audio.
 * For the lowest delay use the sample rate and frame size returned by
 * `aic_processor_get_optimal_sample_rate` and `aic_processor_get_optimal_num_frames`.
 *
 * # Parameters
 * - `processor`: Processor instance to configure. Must not be NULL.
 * - `sample_rate`: Audio sample rate in Hz (8000 - 192000).
 * - `num_channels`: Number of audio channels (1 for mono, 2 for stereo, etc.).
 * - `num_frames`: Number of samples per channel in each process call.
 * - `allow_variable_frames`: Allows varying frame counts per process call (up to `num_frames`), but increases delay.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Configuration accepted
 * - `AIC_ERROR_CODE_NULL_POINTER`: `processor` is NULL
 * - `AIC_ERROR_CODE_UNSUPPORTED_AUDIO_CONFIG`: Configuration is not supported
 *
 * # Note
 * All channels are mixed to mono for processing. To process channels
 * independently, create separate processor instances.
 *
 * # Safety
 * - This function allocates memory. Avoid calling it from real-time audio threads.
 * - This function is not thread-safe. Ensure no other threads are using the processor during initialization.
 */
enum AicErrorCode aic_processor_initialize(struct AicProcessor *processor,
                                           uint32_t sample_rate,
                                           uint16_t num_channels,
                                           size_t num_frames,
                                           bool allow_variable_frames);

/**
 * Processes audio with separate buffers for each channel (planar layout).
 *
 * Enhances speech in the provided audio buffers in-place.
 *
 * **Memory Layout:**
 * - `audio` is an array of pointers, one pointer per channel
 * - Each pointer points to a separate buffer containing `num_frames` samples for that channel
 * - Example for 2 channels, 4 frames:
 *   ```
 *   audio[0] -> [ch0_f0, ch0_f1, ch0_f2, ch0_f3]
 *   audio[1] -> [ch1_f0, ch1_f1, ch1_f2, ch1_f3]
 *   ```
 *
 * The planar function allows a maximum of 16 channels.
 *
 * # Parameters
 * - `processor`: Initialized processor instance. Must not be NULL.
 * - `audio`: Array of `num_channels` pointers, each pointing to a buffer of `num_frames` floats. Must not be NULL.
 * - `num_channels`: Number of channels (must match initialization).
 * - `num_frames`: Number of samples per channel (must match initialization value, or if `allow_variable_frames` was enabled, must be ≤ initialization value).
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Audio processed successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `processor` or `audio` is NULL
 * - `AIC_ERROR_CODE_NOT_INITIALIZED`: Processor has not been initialized
 * - `AIC_ERROR_CODE_AUDIO_CONFIG_MISMATCH`: Channel or frame count mismatch
 * - `AIC_ERROR_CODE_ENHANCEMENT_NOT_ALLOWED`: SDK key was not authorized or process failed to report usage. Check if you have internet connection.
 *
 * # Safety
 * - Real-time safe: Can be called from audio processing threads.
 * - This function is not thread-safe. Do not call this function from multiple threads.
 */
enum AicErrorCode aic_processor_process_planar(struct AicProcessor *processor,
                                               float *const *audio,
                                               uint16_t num_channels,
                                               size_t num_frames);

/**
 * Processes audio with interleaved channels in a single buffer.
 *
 * Enhances speech in the provided audio buffer in-place.
 *
 * **Memory Layout:**
 * - Single contiguous buffer with channels interleaved
 * - Buffer size: `num_channels` * `num_frames` floats
 * - Example for 2 channels, 4 frames:
 *   ```
 *   audio -> [ch0_f0, ch1_f0, ch0_f1, ch1_f1, ch0_f2, ch1_f2, ch0_f3, ch1_f3]
 *   ```
 *
 * # Parameters
 * - `processor`: Initialized processor instance. Must not be NULL.
 * - `audio`: Single buffer containing interleaved audio data of size `num_channels` * `num_frames`. Must not be NULL.
 * - `num_channels`: Number of channels (must match initialization).
 * - `num_frames`: Number of samples per channel (must match initialization value, or if `allow_variable_frames` was enabled, must be ≤ initialization value).
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Audio processed successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `processor` or `audio` is NULL
 * - `AIC_ERROR_CODE_NOT_INITIALIZED`: Processor has not been initialized
 * - `AIC_ERROR_CODE_AUDIO_CONFIG_MISMATCH`: Channel or frame count mismatch
 * - `AIC_ERROR_CODE_ENHANCEMENT_NOT_ALLOWED`: SDK key was not authorized or process failed to report usage. Check if you have internet connection.
 *
 * # Safety
 * - Real-time safe: Can be called from audio processing threads.
 * - This function is not thread-safe. Do not call this function from multiple threads.
 */
enum AicErrorCode aic_processor_process_interleaved(struct AicProcessor *processor,
                                                    float *audio,
                                                    uint16_t num_channels,
                                                    size_t num_frames);

/**
 * Processes audio with sequential channel data in a single buffer.
 *
 * Enhances speech in the provided audio buffer in-place.
 *
 * **Memory Layout:**
 * - Single contiguous buffer with all samples for each channel stored sequentially
 * - Buffer size: `num_channels` * `num_frames` floats
 * - Example for 2 channels, 4 frames:
 *   ```
 *   audio -> [ch0_f0, ch0_f1, ch0_f2, ch0_f3, ch1_f0, ch1_f1, ch1_f2, ch1_f3]
 *   ```
 *
 * # Parameters
 * - `processor`: Initialized processor instance. Must not be NULL.
 * - `audio`: Single buffer containing sequential audio data of size `num_channels` * `num_frames`. Must not be NULL.
 * - `num_channels`: Number of channels (must match initialization).
 * - `num_frames`: Number of samples per channel (must match initialization value, or if `allow_variable_frames` was enabled, must be ≤ initialization value).
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Audio processed successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `processor` or `audio` is NULL
 * - `AIC_ERROR_CODE_NOT_INITIALIZED`: Processor has not been initialized
 * - `AIC_ERROR_CODE_AUDIO_CONFIG_MISMATCH`: Channel or frame count mismatch
 * - `AIC_ERROR_CODE_ENHANCEMENT_NOT_ALLOWED`: SDK key was not authorized or process failed to report usage. Check if you have internet connection.
 *
 * # Safety
 * - Real-time safe: Can be called from audio processing threads.
 * - This function is not thread-safe. Do not call this function from multiple threads.
 */
enum AicErrorCode aic_processor_process_sequential(struct AicProcessor *processor,
                                                   float *audio,
                                                   uint16_t num_channels,
                                                   size_t num_frames);

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
 * - Thread-safe: Can be called from any thread.
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
 * - Thread-safe: Can be called from any thread.
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
 * Returns the total output delay in samples for the current audio configuration.
 *
 * This function provides the complete end-to-end latency introduced by the processor,
 * which includes both algorithmic processing delay and any buffering overhead.
 * Use this value to synchronize enhanced audio with other streams or to implement
 * delay compensation in your application.
 *
 * This queries the processor associated with the provided context handle.
 *
 * **Delay behavior:**
 * - **Before initialization:** Returns the base processing delay using the processor's
 *   optimal frame size at its native sample rate
 * - **After initialization:** Returns the actual delay for your specific configuration,
 *   including any additional buffering introduced by non-optimal frame sizes
 *
 * **Important:** The delay value is always expressed in samples at the sample rate
 * you configured during `aic_processor_initialize`. To convert to time units:
 * `delay_ms = (delay_samples * 1000) / sample_rate`
 *
 * **Note:** Using frame sizes different from the optimal value returned by
 * `aic_processor_get_optimal_num_frames` will increase the delay beyond the processor's base latency.
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
enum AicErrorCode aic_processor_context_get_output_delay(const struct AicProcessorContext *context,
                                                         size_t *delay);

/**
 * Creates a VAD context handle for thread-safe control APIs.
 *
 * The voice activity detection works automatically using the enhanced audio output
 * of a given processor.
 *
 * This uses the processor associated with the provided processor handle.
 *
 * **Important:** If the backing processor is destroyed, the VAD instance will stop
 * producing new data. It is safe to destroy the processor without destroying the VAD.
 *
 * # Parameters
 * - `context`: VAD context instance. Must not be NULL.
 * - `processor`: Processor instance to use as data source for the VAD.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: VAD created successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `context` or `processor` is NULL
 *
 * # Safety
 * - Real-time safe: Can be called from audio processing threads.
 * - Thread-safe: Can be called from any thread.
 * - It is safe for the processor handle to be currently in use by other threads.
 */
enum AicErrorCode aic_vad_context_create(struct AicVadContext **context,
                                         const struct AicProcessor *processor);

/**
 * Releases a VAD context handle.
 *
 * **Important:** This does **NOT** destroy the backing processor.
 * `aic_processor_destroy` must be called separately.
 *
 * After calling this function, the VAD handle becomes invalid.
 * This function is safe to call with NULL.
 *
 * # Parameters
 * - `context`: VAD context instance.
 *
 * # Safety
 * - Thread-safe: Can be called from any thread.
 * - The `context` pointer must have been created by `aic_vad_context_create` when non-NULL.
 */
void aic_vad_context_destroy(struct AicVadContext *context);

/**
 * Returns the VAD's prediction.
 *
 * **Important:**
 * - The latency of the VAD prediction is equal to
 *   the backing processor's processing latency.
 * - If the backing processor stops being processed,
 *   the VAD will not update its speech detection prediction.
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

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  /* AIC_H */
