#include <stdio.h>
#include <stdlib.h>
#include "aic.h"

#define NUM_CHANNELS 2

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <path/to/model.aicmodel>\n", argv[0]);
        return 1;
    }

    const char* model_path = argv[1];
    printf("Using model: %s\n", model_path);

    // Display library version
    printf("Library version: %s\n", aic_get_sdk_version());
    printf("Compatible model version: %u\n", aic_get_compatible_model_version());

    // Get license key from environment variable
    const char* license = getenv("AIC_SDK_LICENSE");
    if (!license) {
        printf("Error: AIC_SDK_LICENSE environment variable not set\n");
        printf("Please set it with: export AIC_SDK_LICENSE=your_license_key\n");
        return 1;
    }

    // Model creation from file
    struct AicModel* model = NULL;
    enum AicErrorCode model_result =
        aic_model_create_from_file(&model, model_path);
    if (model_result != AIC_ERROR_CODE_SUCCESS)
    {
        printf("Model creation failed with error: %d\n", model_result);
        return 1;
    }

    // Processor creation with license key
    struct AicProcessor* processor = NULL;
    enum AicErrorCode processor_result =
        aic_processor_create(&processor, model, license, NULL);
    if (processor_result != AIC_ERROR_CODE_SUCCESS)
    {
        printf("Processor creation failed with error: %d\n", processor_result);
        aic_model_destroy(model);
        return 1;
    }

    // Get optimal settings
    uint32_t optimal_sample_rate;
    size_t optimal_num_frames;
    if (aic_model_get_optimal_sample_rate(model, &optimal_sample_rate) == AIC_ERROR_CODE_SUCCESS)
    {
        printf("Optimal sample rate: %u Hz\n", optimal_sample_rate);
    }
    if (aic_model_get_optimal_num_frames(model, optimal_sample_rate, &optimal_num_frames) == AIC_ERROR_CODE_SUCCESS)
    {
        printf("Optimal frame count: %zu\n", optimal_num_frames);
    }

    // Initialize with basic audio config
    if (aic_processor_initialize(processor, optimal_sample_rate, NUM_CHANNELS, optimal_num_frames, false) != AIC_ERROR_CODE_SUCCESS)
    {
        printf("Processor initialization failed\n");
        aic_processor_destroy(processor);
        aic_model_destroy(model);
        return 1;
    }

    // Create minimal test audio
    float* audio_buffer_left = (float*) calloc(optimal_num_frames, sizeof(float));
    float* audio_buffer_right = (float*) calloc(optimal_num_frames, sizeof(float));
    if (!audio_buffer_left || !audio_buffer_right)
    {
        printf("Memory allocation failed\n");
        free(audio_buffer_left);
        free(audio_buffer_right);
        aic_processor_destroy(processor);
        aic_model_destroy(model);
        return 1;
    }
    float* audio_buffer_planar[2] = {audio_buffer_left, audio_buffer_right};

    // Test planar audio processing
    if (aic_processor_process_planar(processor, audio_buffer_planar, 2, optimal_num_frames) != AIC_ERROR_CODE_SUCCESS)
    {
        printf("Planar processing failed\n");
    }
    else
    {
        printf("Planar processing succeeded\n");
    }

    // Test interleaved audio processing (frames first)
    float* audio_buffer = (float*) calloc(2 * optimal_num_frames, sizeof(float));
    if (aic_processor_process_interleaved(processor, audio_buffer, 2, optimal_num_frames)
        != AIC_ERROR_CODE_SUCCESS)
    {
        printf("Interleaved processing failed\n");
    }
    else
    {
        printf("Interleaved processing succeeded\n");
    }

    // Test sequential audio processing (channels first)
    if (aic_processor_process_sequential(processor, audio_buffer, 2, optimal_num_frames)
        != AIC_ERROR_CODE_SUCCESS)
    {
        printf("Sequential processing failed\n");
    }
    else
    {
        printf("Sequential processing succeeded\n");
    }

    // Processor context handle for thread-safe control APIs
    struct AicProcessorContext* proc_context = NULL;
    enum AicErrorCode proc_context_result = aic_processor_context_create(&proc_context, processor);
    if (proc_context_result != AIC_ERROR_CODE_SUCCESS)
    {
        printf("Processor context creation failed with error: %d\n", proc_context_result);
        aic_processor_destroy(processor);
        aic_model_destroy(model);
        return 1;
    }

    // Test reset functionality
    if (aic_processor_context_reset(proc_context) == AIC_ERROR_CODE_SUCCESS)
    {
        printf("Processor reset succeeded\n");
    }

    // Test parameter setting and getting
    if (aic_processor_context_set_parameter(proc_context, AIC_PROCESSOR_PARAMETER_ENHANCEMENT_LEVEL, 0.7f)
        == AIC_ERROR_CODE_SUCCESS)
    {
        printf("Enhancement parameter set successfully\n");
        float value;
        if (aic_processor_context_get_parameter(proc_context, AIC_PROCESSOR_PARAMETER_ENHANCEMENT_LEVEL, &value)
            == AIC_ERROR_CODE_SUCCESS)
        {
            printf("Enhancement level: %f\n", value);
        }
    }

    // Get output delay
    uintptr_t delay;
    if (aic_processor_context_get_output_delay(proc_context, &delay) == AIC_ERROR_CODE_SUCCESS)
    {
        printf("Output delay: %zu samples\n", delay);
    }

    // VAD context handle for thread-safe control APIs
    struct AicVadContext* vad_context = NULL;
    enum AicErrorCode vad_result = aic_vad_context_create(&vad_context, processor);
    if (vad_result != AIC_ERROR_CODE_SUCCESS)
    {
        printf("VAD context creation failed with error: %d\n", vad_result);
        aic_processor_context_destroy(proc_context);
        aic_processor_destroy(processor);
        aic_model_destroy(model);
        return 1;
    }

    // Test VAD speech detection
    bool is_speech_detected = false;
    if (aic_vad_context_is_speech_detected(vad_context, &is_speech_detected) == AIC_ERROR_CODE_SUCCESS)
    {
        printf("Speech detected: %d\n", is_speech_detected);
    }

    // Test VAD parameter setting and getting
    if (aic_vad_context_set_parameter(vad_context, AIC_VAD_PARAMETER_SENSITIVITY, 5.0f)
        == AIC_ERROR_CODE_SUCCESS)
    {
        printf("VAD parameter set successfully\n");
        float value;
        if (aic_vad_context_get_parameter(vad_context, AIC_VAD_PARAMETER_SENSITIVITY, &value)
            == AIC_ERROR_CODE_SUCCESS)
        {
            printf("VAD sensitivity: %f\n", value);
        }
    }

    // Clean up
    free(audio_buffer_left);
    free(audio_buffer_right);
    free(audio_buffer);
    aic_vad_context_destroy(vad_context);
    aic_processor_context_destroy(proc_context);
    aic_processor_destroy(processor);
    aic_model_destroy(model);
    printf("All tests completed\n");
    return 0;
}
