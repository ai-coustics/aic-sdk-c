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
   * Model must be initialized before calling this operation. Call `aic_model_initialize` first.
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
} AicErrorCode;

/**
 * Available model types for audio enhancement.
 */
typedef enum AicModelType {
  /**
   * **Specifications:**
   * - Window length: 10 ms
   * - Native sample rate: 48 kHz
   * - Native num frames: 480
   * - Processing latency: 30ms
   */
  AIC_MODEL_TYPE_QUAIL_L48 = 0,
  /**
   * **Specifications:**
   * - Window length: 10 ms
   * - Native sample rate: 16 kHz
   * - Native num frames: 160
   * - Processing latency: 30 ms
   */
  AIC_MODEL_TYPE_QUAIL_L16 = 1,
  /**
   * **Specifications:**
   * - Window length: 10 ms
   * - Native sample rate: 8 kHz
   * - Native num frames: 80
   * - Processing latency: 30 ms
   */
  AIC_MODEL_TYPE_QUAIL_L8 = 2,
  /**
   * **Specifications:**
   * - Window length: 10 ms
   * - Native sample rate: 48 kHz
   * - Native num frames: 480
   * - Processing latency: 30 ms
   */
  AIC_MODEL_TYPE_QUAIL_S48 = 3,
  /**
   * **Specifications:**
   * - Window length: 10 ms
   * - Native sample rate: 16 kHz
   * - Native num frames: 160
   * - Processing latency: 30 ms
   */
  AIC_MODEL_TYPE_QUAIL_S16 = 4,
  /**
   * **Specifications:**
   * - Window length: 10 ms
   * - Native sample rate: 8 kHz
   * - Native num frames: 80
   * - Processing latency: 30 ms
   */
  AIC_MODEL_TYPE_QUAIL_S8 = 5,
  /**
   * **Specifications:**
   * - Window length: 10 ms
   * - Native sample rate: 48 kHz
   * - Native num frames: 480
   * - Processing latency: 10 ms
   */
  AIC_MODEL_TYPE_QUAIL_XS = 6,
  /**
   * **Specifications:**
   * - Window length: 10 ms
   * - Native sample rate: 48 kHz
   * - Native num frames: 480
   * - Processing latency: 10 ms
   */
  AIC_MODEL_TYPE_QUAIL_XXS = 7,
  /**
   * Special model optimized for human-to-machine interaction (e.g., voice agents, speech-to-text)
   * designed specifically to improve STT accuracy across unpredictable, diverse and challenging environments.
   *
   * **Specifications:**
   * - Window length: 10 ms
   * - Native sample rate: 16 kHz
   * - Native num frames: 160
   * - Processing latency: 30 ms
   */
  AIC_MODEL_TYPE_QUAIL_STT_L16 = 8,
  /**
   * Special model optimized for human-to-machine interaction (e.g., voice agents, speech-to-text)
   * designed specifically to improve STT accuracy across unpredictable, diverse and challenging environments.
   *
   * **Specifications:**
   * - Window length: 10 ms
   * - Native sample rate: 8 kHz
   * - Native num frames: 80
   * - Processing latency: 30 ms
   */
  AIC_MODEL_TYPE_QUAIL_STT_L8 = 9,
  /**
   * Special model optimized for human-to-machine interaction (e.g., voice agents, speech-to-text)
   * designed specifically to improve STT accuracy across unpredictable, diverse and challenging environments.
   *
   * **Specifications:**
   * - Window length: 10 ms
   * - Native sample rate: 16 kHz
   * - Native num frames: 160
   * - Processing latency: 30 ms
   */
  AIC_MODEL_TYPE_QUAIL_STT_S16 = 10,
  /**
   * Special model optimized for human-to-machine interaction (e.g., voice agents, speech-to-text)
   * designed specifically to improve STT accuracy across unpredictable, diverse and challenging environments.
   *
   * **Specifications:**
   * - Window length: 10 ms
   * - Native sample rate: 8 kHz
   * - Native num frames: 80
   * - Processing latency: 30 ms
   */
  AIC_MODEL_TYPE_QUAIL_STT_S8 = 11,
  /**
   * Special model optimized for human-to-machine interaction (e.g., voice agents, speech-to-text)
   * purpose-built to isolate and elevate the foreground speaker while suppressing both
   * interfering speech and background noise.
   *
   * **Specifications:**
   * - Window length: 10 ms
   * - Native sample rate: 16 kHz
   * - Native num frames: 160
   * - Processing latency: 30 ms
   */
  AIC_MODEL_TYPE_QUAIL_VF_STT_L16 = 12,
} AicModelType;

/**
 * Configurable parameters for audio enhancement
 */
typedef enum AicEnhancementParameter {
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
  AIC_ENHANCEMENT_PARAMETER_BYPASS = 0,
  /**
   * Controls the intensity of speech enhancement processing.
   *
   * **Range:** 0.0 to 1.0
   * - **0.0:** No enhancement - original signal passes through unchanged
   * - **1.0:** Full enhancement - maximum noise reduction but also more audible artifacts
   *
   * **Default:** 1.0
   */
  AIC_ENHANCEMENT_PARAMETER_ENHANCEMENT_LEVEL = 1,
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
  AIC_ENHANCEMENT_PARAMETER_VOICE_GAIN = 2,
} AicEnhancementParameter;

/**
 * Configurable parameters for Voice Activity Detection.
 */
typedef enum AicVadParameter {
  /**
   * Controls the lookback buffer size used in the Voice Activity Detector.
   *
   * The lookback buffer size is the number of window-length audio buffers
   * the VAD has available as a lookback buffer.
   *
   * The stability of the prediction increases with the buffer size,
   * at the cost of higher latency.
   *
   * **Range:** 1.0 to 20.0 (rounded up/down to the closest integer)
   *
   * **Default:** 6.0
   */
  AIC_VAD_PARAMETER_LOOKBACK_BUFFER_SIZE = 0,
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
} AicVadParameter;

typedef struct AicModel AicModel;

typedef struct AicVad AicVad;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * Creates a new audio enhancement model instance.
 *
 * Multiple models can be created to process different audio streams simultaneously
 * or to switch between different enhancement algorithms during runtime.
 *
 * # Parameters
 * - `model`: Receives the handle to the newly created model. Must not be NULL.
 * - `model_type`: Selects the enhancement algorithm variant.
 * - `license_key`: NULL-terminated string containing your license key. Must not be NULL.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Model created successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `model` or `license_key` is NULL
 * - `AIC_ERROR_CODE_LICENSE_INVALID`: License key format is incorrect
 * - `AIC_ERROR_CODE_LICENSE_VERSION_UNSUPPORTED`: License version is not compatible with the SDK version
 * - `AIC_ERROR_CODE_LICENSE_EXPIRED`: License key has expired
 */
enum AicErrorCode aic_model_create(struct AicModel **model,
                                   enum AicModelType model_type,
                                   const char *license_key);

/**
 * Releases all resources associated with a model instance.
 *
 * After calling this function, the model handle becomes invalid.
 * This function is safe to call with NULL.
 *
 * # Parameters
 * - `model`: Model instance to destroy. Can be NULL.
 */
void aic_model_destroy(struct AicModel *model);

/**
 * Configures the model for a specific audio format.
 *
 * This function must be called before processing any audio.
 * For the lowest delay use the sample rate and frame size returned by
 * `aic_get_optimal_sample_rate` and `aic_get_optimal_num_frames`.
 *
 * # Parameters
 * - `model`: Model instance to configure. Must not be NULL.
 * - `sample_rate`: Audio sample rate in Hz (8000 - 192000).
 * - `num_channels`: Number of audio channels (1 for mono, 2 for stereo, etc.).
 * - `num_frames`: Number of samples per channel in each process call.
 * - `allow_variable_frames`: Allows varying frame counts per process call (up to `num_frames`), but increases delay.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Configuration accepted
 * - `AIC_ERROR_CODE_NULL_POINTER`: `model` is NULL
 * - `AIC_ERROR_CODE_UNSUPPORTED_AUDIO_CONFIG`: Configuration is not supported
 *
 * # Warning
 * Do not call from audio processing threads as this allocates memory.
 *
 * # Note
 * All channels are mixed to mono for processing. To process channels
 * independently, create separate model instances.
 */
enum AicErrorCode aic_model_initialize(struct AicModel *model,
                                       uint32_t sample_rate,
                                       uint16_t num_channels,
                                       size_t num_frames,
                                       bool allow_variable_frames);

/**
 * Clears all internal state and buffers.
 *
 * Call this when the audio stream is interrupted or when seeking
 * to prevent artifacts from previous audio content.
 *
 * The model stays initialized to the configured settings.
 *
 * # Parameters
 * - `model`: Model instance to reset. Must not be NULL.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: State cleared successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `model` is NULL
 *
 * # Thread Safety
 * Real-time safe. Can be called from audio processing threads.
 */
enum AicErrorCode aic_model_reset(struct AicModel *model);

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
 * - `model`: Initialized model instance. Must not be NULL.
 * - `audio`: Array of `num_channels` pointers, each pointing to a buffer of `num_frames` floats. Must not be NULL.
 * - `num_channels`: Number of channels (must match initialization).
 * - `num_frames`: Number of samples per channel (must match initialization value, or if `allow_variable_frames` was enabled, must be ≤ initialization value).
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Audio processed successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `model` or `audio` is NULL
 * - `AIC_ERROR_CODE_NOT_INITIALIZED`: Model has not been initialized
 * - `AIC_ERROR_CODE_AUDIO_CONFIG_MISMATCH`: Channel or frame count mismatch
 * - `AIC_ERROR_CODE_ENHANCEMENT_NOT_ALLOWED`: SDK key was not authorized or process failed to report usage. Check if you have internet connection.
 */
enum AicErrorCode aic_model_process_planar(struct AicModel *model,
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
 * - `model`: Initialized model instance. Must not be NULL.
 * - `audio`: Single buffer containing interleaved audio data of size `num_channels` * `num_frames`. Must not be NULL.
 * - `num_channels`: Number of channels (must match initialization).
 * - `num_frames`: Number of samples per channel (must match initialization value, or if `allow_variable_frames` was enabled, must be ≤ initialization value).
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Audio processed successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `model` or `audio` is NULL
 * - `AIC_ERROR_CODE_NOT_INITIALIZED`: Model has not been initialized
 * - `AIC_ERROR_CODE_AUDIO_CONFIG_MISMATCH`: Channel or frame count mismatch
 * - `AIC_ERROR_CODE_ENHANCEMENT_NOT_ALLOWED`: SDK key was not authorized or process failed to report usage. Check if you have internet connection.
 */
enum AicErrorCode aic_model_process_interleaved(struct AicModel *model,
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
 * - `model`: Initialized model instance. Must not be NULL.
 * - `audio`: Single buffer containing sequential audio data of size `num_channels` * `num_frames`. Must not be NULL.
 * - `num_channels`: Number of channels (must match initialization).
 * - `num_frames`: Number of samples per channel (must match initialization value, or if `allow_variable_frames` was enabled, must be ≤ initialization value).
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Audio processed successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `model` or `audio` is NULL
 * - `AIC_ERROR_CODE_NOT_INITIALIZED`: Model has not been initialized
 * - `AIC_ERROR_CODE_AUDIO_CONFIG_MISMATCH`: Channel or frame count mismatch
 * - `AIC_ERROR_CODE_ENHANCEMENT_NOT_ALLOWED`: SDK key was not authorized or process failed to report usage. Check if you have internet connection.
 */
enum AicErrorCode aic_model_process_sequential(struct AicModel *model,
                                               float *audio,
                                               uint16_t num_channels,
                                               size_t num_frames);

/**
 * Modifies a model parameter.
 *
 * All parameters can be changed during audio processing.
 * This function can be called from any thread.
 *
 * # Parameters
 * - `model`: Model instance. Must not be NULL.
 * - `parameter`: Parameter to modify.
 * - `value`: New parameter value. See parameter documentation for ranges.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Parameter updated successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `model` is NULL
 * - `AIC_ERROR_CODE_PARAMETER_OUT_OF_RANGE`: Value outside valid range
 */
enum AicErrorCode aic_model_set_parameter(struct AicModel *model,
                                          enum AicEnhancementParameter parameter,
                                          float value);

/**
 * Retrieves the current value of a parameter.
 *
 * This function can be called from any thread.
 *
 * # Parameters
 * - `model`: Model instance. Must not be NULL.
 * - `parameter`: Parameter to query.
 * - `value`: Receives the current parameter value. Must not be NULL.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Parameter retrieved successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `model` or `value` is NULL
 */
enum AicErrorCode aic_model_get_parameter(const struct AicModel *model,
                                          enum AicEnhancementParameter parameter,
                                          float *value);

/**
 * Returns the total output delay in samples for the current audio configuration.
 *
 * This function provides the complete end-to-end latency introduced by the model,
 * which includes both algorithmic processing delay and any buffering overhead.
 * Use this value to synchronize enhanced audio with other streams or to implement
 * delay compensation in your application.
 *
 * **Delay behavior:**
 * - **Before initialization:** Returns the base processing delay using the model's
 *   optimal frame size at its native sample rate
 * - **After initialization:** Returns the actual delay for your specific configuration,
 *   including any additional buffering introduced by non-optimal frame sizes
 *
 * **Important:** The delay value is always expressed in samples at the sample rate
 * you configured during `aic_model_initialize`. To convert to time units:
 * `delay_ms = (delay_samples * 1000) / sample_rate`
 *
 * **Note:** Using frame sizes different from the optimal value returned by
 * `aic_get_optimal_num_frames` will increase the delay beyond the model's base latency.
 *
 * # Parameters
 * - `model`: Initialized model instance. Must not be NULL.
 * - `delay`: Receives the delay in samples. Must not be NULL.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Latency retrieved successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `model` or `latency` is NULL
 */
enum AicErrorCode aic_get_output_delay(const struct AicModel *model, size_t *delay);

/**
 * Retrieves the native sample rate of the selected model.
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
 * of frames (returned by `aic_get_optimal_num_frames`) will change. The model's output
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
 */
enum AicErrorCode aic_get_optimal_sample_rate(const struct AicModel *model, uint32_t *sample_rate);

/**
 * Retrieves the optimal number of frames for the selected model at a given sample rate.
 *
 * Using the optimal number of frames minimizes latency by avoiding internal buffering.
 *
 * **When you use a different frame count than the optimal value, the model will
 * introduce additional buffering latency on top of its base processing delay.**
 *
 * The optimal frame count varies based on the sample rate. Each model operates on a
 * fixed time window length, so the required number of frames changes with sample rate.
 * For example, a model designed for 10 ms processing windows requires 480 frames at
 * 48 kHz, but only 160 frames at 16 kHz to capture the same duration of audio.
 *
 * Call this function with your intended sample rate before calling `aic_model_initialize`
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
 */
enum AicErrorCode aic_get_optimal_num_frames(const struct AicModel *model,
                                             uint32_t sample_rate,
                                             size_t *num_frames);

/**
 * Creates a new Voice Activity Detector instance.
 *
 * The VAD works automatically using the enhanced audio output of a given model.
 *
 * **Important:** If the backing model is destroyed, the VAD instance will stop
 * producing new data. It is safe to destroy the model without destroying the VAD.
 *
 * # Parameters
 * - `vad`: Receives the handle to the newly created VAD. Must not be NULL.
 * - `model`: Model instance to use as data source for the VAD.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: VAD created successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `vad` or `model` is NULL
 */
enum AicErrorCode aic_vad_create(struct AicVad **vad, struct AicModel *model);

/**
 * Releases the VAD instance.
 *
 * **Important:** This does **NOT** destroy the backing model.
 * `aic_model_destroy` must be called separately.
 *
 * After calling this function, the VAD handle becomes invalid.
 * This function is safe to call with NULL.
 *
 * # Parameters
 * - `vad`: VAD instance to destroy. Can be NULL.
 */
void aic_vad_destroy(struct AicVad *vad);

/**
 * Returns the VAD's prediction.
 *
 * **Important:**
 * - The latency of the VAD prediction is equal to
 *   the backing model's processing latency.
 * - If the backing model stops being processed,
 *   the VAD will not update its speech detection prediction.
 *
 * # Parameters
 * - `vad`: VAD instance. Must not be NULL.
 * - `value`: Receives the VAD prediction. Must not be NULL.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Prediction retrieved successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `vad` or `value` is NULL
 */
enum AicErrorCode aic_vad_is_speech_detected(struct AicVad *vad, bool *value);

/**
 * Modifies a VAD parameter.
 *
 * All parameters can be changed during audio processing.
 * This function can be called from any thread.
 *
 * # Parameters
 * - `model`: Model instance. Must not be NULL.
 * - `parameter`: Parameter to modify.
 * - `value`: New parameter value. See parameter documentation for ranges.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Parameter updated successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `vad` is NULL
 * - `AIC_ERROR_CODE_PARAMETER_OUT_OF_RANGE`: Value outside valid range
 */
enum AicErrorCode aic_vad_set_parameter(struct AicVad *vad,
                                        enum AicVadParameter parameter,
                                        float value);

/**
 * Retrieves the current value of a parameter.
 *
 * This function can be called from any thread.
 *
 * # Parameters
 * - `model`: Model instance. Must not be NULL.
 * - `parameter`: Parameter to query.
 * - `value`: Receives the current parameter value. Must not be NULL.
 *
 * # Returns
 * - `AIC_ERROR_CODE_SUCCESS`: Parameter retrieved successfully
 * - `AIC_ERROR_CODE_NULL_POINTER`: `vad` or `value` is NULL
 */
enum AicErrorCode aic_vad_get_parameter(const struct AicVad *vad,
                                        enum AicVadParameter parameter,
                                        float *value);

/**
 * Returns the version of the SDK.
 *
 * # Safety
 * The returned pointer points to a static string and remains valid
 * for the lifetime of the program. The caller should NOT free this pointer.
 *
 * # Returns
 * A null-terminated C string containing the version (e.g., "1.2.3")
 */
const char *aic_get_sdk_version(void);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  /* AIC_H */
