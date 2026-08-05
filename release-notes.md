This release contains two breaking changes that affect every integration:

- **All audio APIs are mono only.** The multi-channel process/buffer functions are gone.
- **The VAD is its own object.** VAD runs on dedicated VAD models through `AicVad`, and can no
   longer be derived from a processor.

Both migrations are covered step by step below.

## Breaking Changes

### Multi-channel support removed

The processor and the analyzer's collector now operate on mono audio only.

All models process mono inputs. Previously the processor mixed all input channels down to mono
internally, which could lead to surprising results. To prevent misunderstandings, all APIs now take
exclusively mono inputs.

To process multi-channel audio, downmix to mono before calling `aic_processor_process`, or create a
separate processor instance per channel.

#### What changed

| Before (0.21.4) | Now |
| --- | --- |
| `aic_processor_initialize(processor, sample_rate, num_channels, num_frames, allow_variable_frames)` | `aic_processor_initialize(processor, sample_rate, block_size, variable_block_size)` |
| `aic_processor_process_planar` / `aic_processor_process_interleaved` / `aic_processor_process_sequential` | `aic_processor_process(processor, audio_ptr, audio_len)` |
| `aic_collector_initialize(collector, sample_rate, num_channels, num_frames, allow_variable_frames)` | `aic_collector_initialize(collector, sample_rate, block_size, variable_block_size)` |
| `aic_collector_buffer_planar` / `aic_collector_buffer_interleaved` / `aic_collector_buffer_sequential` | `aic_collector_buffer(collector, audio_ptr, audio_len)` |
| `aic_model_get_optimal_num_frames` | `aic_model_get_optimal_block_size` |

Naming also became consistent: the configured block length is `block_size` on
`aic_processor_initialize` and `aic_collector_initialize`, the per-call buffer length is `audio_len`
on `aic_processor_process` and `aic_collector_buffer`, and the `allow_variable_frames` flag is now
`variable_block_size`.

#### Before

```c
uint32_t sample_rate;
size_t   num_frames;
aic_model_get_optimal_sample_rate(model, &sample_rate);
aic_model_get_optimal_num_frames(model, sample_rate, &num_frames);

// Stereo in, stereo out. The SDK mixed both channels down to mono internally.
aic_processor_initialize(processor, sample_rate, 2, num_frames, false);

float *planar[2] = {left, right};
aic_processor_process_planar(processor, planar, 2, num_frames);
```

#### After

```c
uint32_t sample_rate;
size_t   block_size;
aic_model_get_optimal_sample_rate(model, &sample_rate);
aic_model_get_optimal_block_size(model, sample_rate, &block_size);

aic_processor_initialize(processor, sample_rate, block_size, false);

// Downmix to mono yourself, then process a single buffer in-place.
for (size_t i = 0; i < block_size; i++) {
  mono[i] = 0.5f * (left[i] + right[i]);
}

aic_processor_process(processor, mono, block_size);
```

If you need per-channel output instead of a downmix, create one processor per channel and call
`aic_processor_process` once per channel with that channel's buffer.

The same applies to the analyzer: downmix multichannel audio before calling
`aic_collector_buffer`, or create a separate collector/analyzer pair per channel.

The OpenTelemetry `audio.channels` metric has been kept for backwards compatibility, but it now
always reports exactly one channel.

### VAD moved into its own object, energy-based VAD removed

Voice activity detection is no longer a side effect of enhancement. It is now a first-class object,
`AicVad`, that runs a dedicated VAD model.

Energy-based VADs, which inferred speech activity from the output level of an enhancement model,
have been removed. They were an approximation and their accuracy depended on the enhancement model
in use. A dedicated VAD model is trained for the task and is considerably more accurate.

#### What changed

| Before (0.21.4) | Now |
| --- | --- |
| VAD came from a processor: `aic_vad_context_create(&context, processor)` | VAD is standalone: `aic_vad_create` → `aic_vad_initialize` → `aic_vad_process`, then `aic_vad_context_create(&context, vad)` |
| Any enhancement model provided a VAD (energy-based), VAD models were also loaded into a processor | Only dedicated VAD models are accepted by `aic_vad_create`; `aic_processor_create` accepts only enhancement models |
| VAD advanced whenever the processor processed audio | VAD advances on `aic_vad_process`, independently of any processor |
| `AIC_VAD_PARAMETER_SENSITIVITY` ranged 0.0 - 1.0 on VAD models and 1.0 - 15.0 on energy-based VADs | `AIC_VAD_PARAMETER_SENSITIVITY` is always a probability threshold, 0.0 - 1.0 |

#### Before

```c
// One model, one processor: enhancement and VAD were coupled.
struct AicProcessor *processor = NULL;
aic_processor_create(&processor, enhancement_model, license, NULL);
aic_processor_initialize(processor, sample_rate, 1, num_frames, false);

struct AicVadContext *vad_context = NULL;
aic_vad_context_create(&vad_context, processor);
aic_vad_context_set_parameter(vad_context, AIC_VAD_PARAMETER_SENSITIVITY, 5.0f); // energy threshold

// The VAD updated as a side effect of enhancement.
aic_processor_process_interleaved(processor, audio, 1, num_frames);

bool is_speech_detected = false;
aic_vad_context_is_speech_detected(vad_context, &is_speech_detected);

aic_vad_context_destroy(vad_context);
aic_processor_destroy(processor);
```

#### After

```c
// Load a dedicated VAD model and create an AicVad from it.
struct AicModel *vad_model = NULL;
aic_model_create_from_file(&vad_model, "path/to/vad-model.aicmodel");

struct AicVad *vad = NULL;
// Returns AIC_ERROR_CODE_MODEL_TYPE_UNSUPPORTED if the model is not a VAD model.
aic_vad_create(&vad, vad_model, license, NULL);

uint32_t sample_rate;
size_t   block_size;
aic_model_get_optimal_sample_rate(vad_model, &sample_rate);
aic_model_get_optimal_block_size(vad_model, sample_rate, &block_size);
aic_vad_initialize(vad, sample_rate, block_size, false);

struct AicVadContext *vad_context = NULL;
aic_vad_context_create(&vad_context, vad);
aic_vad_context_set_parameter(vad_context, AIC_VAD_PARAMETER_SENSITIVITY, 0.8f); // probability

// The VAD is driven explicitly and does not modify the audio.
aic_vad_process(vad, audio, block_size);

bool is_speech_detected = false;
aic_vad_context_is_speech_detected(vad_context, &is_speech_detected);

aic_vad_context_destroy(vad_context);
aic_vad_destroy(vad);
aic_model_destroy(vad_model);
```

#### Run the VAD on the original audio

If you use enhancement and VAD together, **feed the VAD the original input audio, not the
processor's enhanced output.** Run the two objects side by side on the same mono block rather than chaining
them:

```c
// Recommended: both objects see the same original input block.
aic_vad_process(vad, input, block_size);             // reads the block, does not modify it
aic_processor_process(processor, input, block_size); // enhances the block in-place
```

`aic_vad_process` takes a `const float *` and leaves the buffer untouched, so calling it on the same
buffer before `aic_processor_process` is all it takes to keep the VAD on the unprocessed signal.

Enhancement is designed to change the signal, so running the VAD on its output means detecting
speech in audio that no longer matches what the VAD model expects. It also stacks the processor's
delay on top of the VAD's own prediction delay, which makes speech decisions harder to align.

#### Delay queries renamed

There is no single "output delay" any more. The processor delays audio, the VAD does not, so the two
queries are now named after what they actually report:

| Before (0.21.4) | Now | What it means |
| --- | --- | --- |
| `aic_processor_context_get_output_delay` | `aic_processor_context_get_audio_delay` | An **audio** delay. The enhanced samples leave `aic_processor_process` that many samples behind their input. |
| No VAD-specific query. The VAD was driven by a processor, so its prediction delay was the processor's `aic_processor_context_get_output_delay`. | `aic_vad_context_get_prediction_delay` | A **prediction** delay. It is *not* applied to the audio, `aic_vad_process` leaves the buffer untouched. It tells you how far behind its own input the published prediction is, so you can line speech decisions up with the audio timeline. |

With both objects fed from the same input block, the two delays are independent of each other:

```c
size_t audio_delay, prediction_delay;
aic_processor_context_get_audio_delay(proc_context, &audio_delay);
aic_vad_context_get_prediction_delay(vad_context, &prediction_delay);

// The enhanced audio lags the input by audio_delay.
// The VAD prediction lags the same input by prediction_delay.
```

`AicVad` mirrors the processor's lifecycle and control surface:

- `aic_vad_create`, `aic_vad_initialize`, `aic_vad_process`, `aic_vad_destroy`
- `aic_vad_context_create` / `aic_vad_context_destroy` for thread-safe control handles
- `aic_vad_context_reset`, `aic_vad_context_get_prediction_delay`,
  `aic_vad_context_update_bearer_token`
- `aic_vad_context_is_speech_detected`, `aic_vad_context_get_raw_vad_probability`,
  `aic_vad_context_set_parameter`, `aic_vad_context_get_parameter`

See `examples/vad.c` for a complete, error-checked example.

The OpenTelemetry `experimental.vad.speech_duration` metric is now reported for dedicated VAD models
only. Enhancement and bypass sessions no longer derive VAD state from the model output, so they
always report zero. `processor.vad_created` reports whether the session runs a VAD model.

### Renamed error codes

Some error codes are no longer processor-specific, since they are now also returned by the VAD:

| Before (0.21.4) | Now |
| --- | --- |
| `AIC_ERROR_CODE_PROCESSOR_NOT_INITIALIZED` | `AIC_ERROR_CODE_NOT_INITIALIZED` |
| `AIC_ERROR_CODE_ENHANCEMENT_NOT_ALLOWED` | `AIC_ERROR_CODE_PROCESSING_NOT_ALLOWED` |
| `AIC_ERROR_CODE_MODEL_FILE_PATH_INVALID` | `AIC_ERROR_CODE_FILE_PATH_INVALID` |

The numeric values are unchanged, so only source-level references need updating.

## New Features

There are new explicit session termination APIs. Use these APIs to close telemetry sessions
during lifecycle events without waiting for the corresponding SDK object to be destroyed,
which is useful in integrations where object deallocation may be delayed.

- `aic_processor_terminate_session`
- `aic_vad_terminate_session`
- `aic_analyzer_terminate_session`

## Bug Fixes

- Resetting VAD state now immediately clears the published speech detection and
  raw VAD probability values, so query APIs no longer return stale values from
  the previous stream after reset.
