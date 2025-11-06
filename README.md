# IoT-Node-Captures-Audio
/src
  main.ino
  App.h / App.cpp              // điều phối
  AudioIn.h / AudioIn.cpp      // I2S + mock WAV
  VADSeg.h / VADSeg.cpp        // VAD + circular buffer + segmenter
  FE.h / FE.cpp                // log-mel/MFCC
  AI.h / AI.cpp                // TinyML (stub → thật)
  Storage.h / Storage.cpp      // WAV + metadata JSON (LittleFS)
  MqttPub.h / MqttPub.cpp      // MQTT publish meta + chunk wav
  TimeSync.h / TimeSync.cpp    // NTP + ISO8601
  Cfg.h / Cfg.cpp              // cấu hình (JSON file + defaults)
/data
  mock.wav                     // file test cho dev-mock
platformio.ini
partitions.csv
