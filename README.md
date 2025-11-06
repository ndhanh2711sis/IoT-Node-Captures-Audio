# IoT-Node-Captures-Audio
# 
# Project small IoT node for capturing audio, extracting features, running a TinyML model and publishing event metadata + audio via MQTT.
- Capture audio via I2S on ESP32-S3 (or use a mock WAV for development)
- Voice activity detection (VAD) + segmenter
- Feature extraction (log-mel / MFCC)
- TinyML inference (AI module is pluggable / can be stubbed for dev)
- Store WAV and metadata in LittleFS
- Publish metadata and upload audio in chunks over MQTT
- `src/`
  - `main.cpp` (entry, app loop)
  - `App.cpp` / `include/App.h`         — application orchestration
  - `AudioIn.cpp` / `include/AudioIn.h` — I2S input + mock WAV playback
  - `VADSeg.cpp`  / `include/VADSeg.h`  — VAD + buffer + segmenter
  - `FE.cpp`      / `include/FE.h`      — feature extraction (log-mel / MFCC)
  - `AI.cpp`      / `include/AI.h`      — inference (TinyML / stub)
  - `Storage.cpp` / `include/Storage.h` — LittleFS helpers, WAV + JSON
  - `MqttPub.cpp` / `include/MqttPub.h` — MQTT publish helper
  - `TimeSync.cpp`/ `include/TimeSync.h`— NTP, timestamp helpers
  - `Cfg.cpp`     / `include/Cfg.h`     — runtime configuration loader

- `data/`
  - `mock.wav` — optional mock audio file used in `dev-mock` profile (put here before uploading to LittleFS)

- `platformio.ini`, `partitions.csv` — project build and partitioning

Build & upload (PlatformIO, PowerShell on Windows)

1) Build

```powershell
platformio run
```

2) Upload to device (assumes board connected and `upload_port` configured or auto-detected)

```powershell
platformio run --target upload
```

3) Monitor serial output

```powershell
platformio device monitor
```

Development profiles

- `dev-mock` (default) — uses `BUILD_PROFILE_MOCK` and reads `data/mock.wav` from LittleFS. Good for running the app on desktop or dev board without an I2S microphone.
- `hw-i2s` — uses `BUILD_PROFILE_HW` and the I2S driver. Configure I2S pins in `src/AudioIn.cpp` for your hardware.

Quick notes

- Ensure `data/mock.wav` is uploaded to LittleFS (or bundled) when using `dev-mock`.
- Edit `Cfg.cpp`/`Cfg.h` or `config.json` (if provided) to set Wi‑Fi, MQTT and device-specific values.
- LittleFS is selected as the filesystem (see `platformio.ini` board settings).

Contributing

Feel free to open issues or PRs. Keep changes small and test on both mock and hardware profiles where possible.

License

This project is provided under the MIT license (if applicable — add your license file).


