#include <stdio.h>
#include <stdlib.h>
#include "aic.h"

#define NUM_CHANNELS 2

int main()
{
    // Display library version
    printf("Library version: %s\n", aic_get_sdk_version());

    // Get license key from environment variable
    const char* license = getenv("AIC_SDK_LICENSE");
    if (!license) {
        printf("Error: AIC_SDK_LICENSE environment variable not set\n");
        printf("Please set it with: export AIC_SDK_LICENSE=your_license_key\n");
        return 1;
    }

    // Model creation with license key
    struct AicModel* model = NULL;
    enum AicErrorCode create_result =
        aic_model_create(&model, AIC_MODEL_TYPE_QUAIL_S48, license);
    if (create_result != AIC_ERROR_CODE_SUCCESS)
    {
        printf("Model creation failed with error: %d\n", create_result);
        return 1;
    }

    // Get optimal settings
    uint32_t optimal_sample_rate;
    size_t optimal_num_frames;
    if (aic_get_optimal_sample_rate(model, &optimal_sample_rate) == AIC_ERROR_CODE_SUCCESS)
    {
        printf("Optimal sample rate: %u Hz\n", optimal_sample_rate);
    }
    if (aic_get_optimal_num_frames(model, &optimal_num_frames) == AIC_ERROR_CODE_SUCCESS)
    {
        printf("Optimal frame count: %zu\n", optimal_num_frames);
    }

    // Initialize with basic audio config
    if (aic_model_initialize(model, optimal_sample_rate, NUM_CHANNELS, optimal_num_frames) != AIC_ERROR_CODE_SUCCESS)
    {
        printf("Model initialization failed\n");
        aic_model_destroy(model);
        return 1;
    }

    // Get output delay
    uintptr_t delay;
    if (aic_get_output_delay(model, &delay) == AIC_ERROR_CODE_SUCCESS)
    {
        printf("Output delay: %zu samples\n", delay);
    }

    // Test parameter setting and getting
    if (aic_model_set_parameter(model, AIC_PARAMETER_ENHANCEMENT_LEVEL, 0.7f)
        == AIC_ERROR_CODE_SUCCESS)
    {
        printf("Parameter set successfully\n");
        float value;
        if (aic_model_get_parameter(model, AIC_PARAMETER_ENHANCEMENT_LEVEL, &value)
            == AIC_ERROR_CODE_SUCCESS)
        {
            printf("Enhancement level: %f\n", value);
        }
    }

    // Create minimal test audio
    float* audio_buffer_left = (float*) calloc(optimal_num_frames, sizeof(float));
    float* audio_buffer_right = (float*) calloc(optimal_num_frames, sizeof(float));
    if (!audio_buffer_left || !audio_buffer_right)
    {
        printf("Memory allocation failed\n");
        free(audio_buffer_left);
        free(audio_buffer_right);
        aic_model_destroy(model);
        return 1;
    }
    float* audio_buffer_planar[2] = {audio_buffer_left, audio_buffer_right};

    // Test both audio processing methods
    if (aic_model_process_planar(model, audio_buffer_planar, 2, optimal_num_frames) != AIC_ERROR_CODE_SUCCESS)
    {
        printf("Planar processing failed\n");
    }
    else
    {
        printf("Planar processing succeeded\n");
    }

    float* audio_buffer_interleaved = (float*) calloc(2 * optimal_num_frames, sizeof(float));
    if (aic_model_process_interleaved(model, audio_buffer_interleaved, 2, optimal_num_frames)
        != AIC_ERROR_CODE_SUCCESS)
    {
        printf("Interleaved processing failed\n");
    }
    else
    {
        printf("Interleaved processing succeeded\n");
    }

    // Test reset functionality
    if (aic_model_reset(model) == AIC_ERROR_CODE_SUCCESS)
    {
        printf("Model reset succeeded\n");
    }

    // Clean up
    free(audio_buffer_left);
    free(audio_buffer_right);
    free(audio_buffer_interleaved);
    aic_model_destroy(model);
    printf("All tests completed\n");
    return 0;
}
