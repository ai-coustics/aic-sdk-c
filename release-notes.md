## 0.20.0

## New Features

This release includes several new APIs for running our newest audio intelligence model, *Tyto*.

The new APIs introduce two new concepts: The `Collector` and the `Analyzer`.
 - The `Collector` is designed to be placed in the audio thread, buffering audio chunks for later analysis.
 - The `Analyzer` is designed to be run separately. Analysis models are computationally expensive and cannot run in the audio thread. The analyzer has access to the audio buffered by the collector, and it can access it safely across threads.

Initialize the `Collector` with the same configuration as your existing `Processor` and you can
call the `aic_collector_buffer_*` APIs in the same manner as the `aic_processor_process_*` APIs.

Call `aic_analyzer_analyze_buffered` in a separate thread to obtain an analysis of the latest
audio buffered by the `Collector`.

## Platform Support

- Fixed a crash on Android during startup
