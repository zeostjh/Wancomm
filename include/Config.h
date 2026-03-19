#pragma once

// Audio Configuration
#define DEFAULT_SAMPLE_RATE     48000   // 48 kHz
#define DEFAULT_FRAME_SIZE      960     // 20ms at 48kHz
#define DEFAULT_CHANNELS        1       // Mono
#define DEFAULT_BITRATE         24000   // 24 kbps

// Network Configuration
#define DEFAULT_PORT            5000
#define MAX_PACKET_SIZE         4096
#define RECV_TIMEOUT_MS         100

// Buffer Configuration
#define AUDIO_QUEUE_SIZE        10      // Number of audio buffers to queue
#define MAX_JITTER_BUFFER_MS    200     // Maximum jitter buffer size

// Quality Presets
namespace VoiceChat {
namespace Presets {

    // Low quality, minimal bandwidth
    struct LowQuality {
        static constexpr int sampleRate = 16000;
        static constexpr int bitrate = 8000;
        static constexpr int frameSize = 320;  // 20ms at 16kHz
    };

    // Standard quality, balanced
    struct StandardQuality {
        static constexpr int sampleRate = 48000;
        static constexpr int bitrate = 24000;
        static constexpr int frameSize = 960;  // 20ms at 48kHz
    };

    // High quality, higher bandwidth
    struct HighQuality {
        static constexpr int sampleRate = 48000;
        static constexpr int bitrate = 64000;
        static constexpr int frameSize = 960;  // 20ms at 48kHz
    };

} // namespace Presets
} // namespace VoiceChat
