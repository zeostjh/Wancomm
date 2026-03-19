#pragma once

#include "AudioCapture.h"
#include "AudioPlayback.h"
#include "AudioCodec.h"
#include "NetworkManager.h"
#include <memory>
#include <atomic>

namespace VoiceChat {

struct VoiceChatConfig {
    int sampleRate = 48000;
    int framesPerBuffer = 960;  // 20ms at 48kHz
    int channels = 1;           // Mono
    int bitrate = 24000;        // 24 kbps
    std::string serverAddress;
    std::string serverBindAddress = "0.0.0.0";  // Server bind IP
    int serverPort = 5000;
    bool isServer = false;
    int inputDeviceIndex = -1;  // -1 for default device
    int outputDeviceIndex = -1; // -1 for default device
    int channelNumber = 0;      // Which channel (0-3)
    std::string username;       // User's display name
    std::string password;       // User's password for authentication
};

class VoiceChatClient {
public:
    VoiceChatClient();
    ~VoiceChatClient();

    // Initialize the voice chat client with configuration
    bool initialize(const VoiceChatConfig& config);
    
    // Start the voice chat session
    bool start();
    
    // Stop the voice chat session
    void stop();
    
    // Check if active
    bool isActive() const;
    
    // Mute/unmute microphone (stops sending audio)
    void setMuted(bool muted);
    bool isMuted() const;
    
    // Set talk button state for a specific channel (actively transmitting)
    void setTalking(int channel, bool talking);
    bool isTalking(int channel) const;
    
    // Set listen button state for a specific channel (receiving audio)
    void setListening(int channel, bool listening);
    bool isListening(int channel) const;
    
    // Get audio levels
    struct AudioLevels {
        float inputLevel;
        float outputLevel;
    };
    AudioLevels getAudioLevels() const;
    
    // Get connected users
    std::vector<ClientInfo> getConnectedUsers() const;
    
    // Get statistics
    struct Stats {
        uint64_t packetsSent;
        uint64_t packetsReceived;
        uint64_t packetsLost;
        float latency;
    };
    Stats getStats() const;

private:
    void onAudioCaptured(const float* samples, size_t frameCount);
    void onPacketReceived(const AudioPacket& packet);

    std::unique_ptr<AudioCapture> audioCapture_;
    std::unique_ptr<AudioPlayback> audioPlayback_;
    std::unique_ptr<AudioCodec> codec_;
    
    // Multi-channel support (4 channels)
    std::atomic<bool> isTalking_[4];
    std::atomic<bool> isListening_[4];
    
    // Audio levels
    std::atomic<float> inputLevel_;
    std::atomic<float> outputLevel_;
    std::unique_ptr<NetworkManager> network_;
    
    VoiceChatConfig config_;
    std::atomic<bool> isActive_;
    std::atomic<bool> isMuted_;
    
    // Statistics
    std::atomic<uint64_t> packetsSent_;
    std::atomic<uint64_t> packetsReceived_;
    uint32_t lastSequenceReceived_;
    uint32_t sequenceNumber_;
};

} // namespace VoiceChat
