#include "VoiceChatClient.h"
#include <iostream>
#include <chrono>
#include <cmath>
#include <algorithm>

namespace VoiceChat {

VoiceChatClient::VoiceChatClient()
    : isActive_(false)
    , isMuted_(false)
    , inputLevel_(0.0f)
    , outputLevel_(0.0f)
    , packetsSent_(0)
    , packetsReceived_(0)
    , lastSequenceReceived_(0)
    , sequenceNumber_(0) {
    
    // Initialize all channels to false
    for (int i = 0; i < 4; i++) {
        isTalking_[i] = false;
        isListening_[i] = true; // Default to listening on all channels
    }
}

VoiceChatClient::~VoiceChatClient() {
    stop();
}

bool VoiceChatClient::initialize(const VoiceChatConfig& config) {
    config_ = config;

    // Initialize audio codec
    codec_ = std::make_unique<AudioCodec>();
    if (!codec_->initializeEncoder(config.sampleRate, config.channels, config.bitrate)) {
        std::cerr << "Failed to initialize encoder" << std::endl;
        return false;
    }
    if (!codec_->initializeDecoder(config.sampleRate, config.channels)) {
        std::cerr << "Failed to initialize decoder" << std::endl;
        return false;
    }

    // Initialize network
    network_ = std::make_unique<NetworkManager>();
    if (config.isServer) {
        if (!network_->initializeServer(config.serverPort, config.serverBindAddress)) {
            std::cerr << "Failed to initialize server" << std::endl;
            return false;
        }
    } else {
        if (!network_->initializeClient(config.serverAddress, config.serverPort, config.username, config.password)) {
            std::cerr << "Failed to initialize client" << std::endl;
            return false;
        }
    }

    // Set network receive callback
    network_->setReceiveCallback([this](const AudioPacket& packet) {
        onPacketReceived(packet);
    });

    // Initialize audio capture
    audioCapture_ = std::make_unique<AudioCapture>();
    if (!audioCapture_->initialize(config.sampleRate, config.framesPerBuffer, config.inputDeviceIndex)) {
        std::cerr << "Failed to initialize audio capture" << std::endl;
        return false;
    }

    // Set audio capture callback
    audioCapture_->setAudioCallback([this](const float* samples, size_t frameCount) {
        onAudioCaptured(samples, frameCount);
    });

    // Initialize audio playback
    audioPlayback_ = std::make_unique<AudioPlayback>();
    if (!audioPlayback_->initialize(config.sampleRate, config.framesPerBuffer, config.outputDeviceIndex)) {
        std::cerr << "Failed to initialize audio playback" << std::endl;
        return false;
    }

    std::cout << "VoiceChat client initialized successfully" << std::endl;
    return true;
}

bool VoiceChatClient::start() {
    if (isActive_) {
        return true;
    }

    // Start network receiving
    if (!network_->startReceiving()) {
        std::cerr << "Failed to start network receiving" << std::endl;
        return false;
    }

    // Start audio playback
    if (!audioPlayback_->start()) {
        std::cerr << "Failed to start audio playback" << std::endl;
        return false;
    }

    // Start audio capture
    if (!audioCapture_->start()) {
        std::cerr << "Failed to start audio capture" << std::endl;
        return false;
    }

    isActive_ = true;
    std::cout << "VoiceChat started" << std::endl;
    return true;
}

void VoiceChatClient::stop() {
    if (!isActive_) {
        return;
    }

    isActive_ = false;

    if (audioCapture_) {
        audioCapture_->stop();
    }

    if (audioPlayback_) {
        audioPlayback_->stop();
    }

    if (network_) {
        network_->stopReceiving();
    }

    std::cout << "VoiceChat stopped" << std::endl;
}

bool VoiceChatClient::isActive() const {
    return isActive_;
}

void VoiceChatClient::setMuted(bool muted) {
    isMuted_ = muted;
    std::cout << (muted ? "Muted" : "Unmuted") << std::endl;
}

bool VoiceChatClient::isMuted() const {
    return isMuted_;
}

void VoiceChatClient::setTalking(int channel, bool talking) {
    if (channel >= 0 && channel < 4) {
        isTalking_[channel] = talking;
        std::cout << "Channel " << channel << (talking ? " talking" : " not talking") << std::endl;
    }
}

bool VoiceChatClient::isTalking(int channel) const {
    if (channel >= 0 && channel < 4) {
        return isTalking_[channel];
    }
    return false;
}

void VoiceChatClient::setListening(int channel, bool listening) {
    if (channel >= 0 && channel < 4) {
        isListening_[channel] = listening;
        std::cout << "Channel " << channel << (listening ? " listening" : " not listening") << std::endl;
    }
}

bool VoiceChatClient::isListening(int channel) const {
    if (channel >= 0 && channel < 4) {
        return isListening_[channel];
    }
    return false;
}

VoiceChatClient::AudioLevels VoiceChatClient::getAudioLevels() const {
    AudioLevels levels;
    levels.inputLevel = inputLevel_;
    levels.outputLevel = outputLevel_;
    return levels;
}

std::vector<ClientInfo> VoiceChatClient::getConnectedUsers() const {
    if (network_) {
        return network_->getConnectedUsers();
    }
    return {};
}

VoiceChatClient::Stats VoiceChatClient::getStats() const {
    Stats stats;
    stats.packetsSent = packetsSent_;
    stats.packetsReceived = packetsReceived_;
    stats.packetsLost = 0;  // TODO: Calculate actual packet loss
    stats.latency = 0.0f;   // TODO: Calculate actual latency
    return stats;
}

void VoiceChatClient::onAudioCaptured(const float* samples, size_t frameCount) {
    if (!isActive_ || isMuted_) {
        return;
    }
    
    // Check if any channel is talking
    bool anyTalking = false;
    int talkingChannel = -1;
    for (int i = 0; i < 4; i++) {
        if (isTalking_[i]) {
            anyTalking = true;
            talkingChannel = i;
            break; // Use first talking channel
        }
    }
    
    if (!anyTalking) {
        return;
    }
    
    // Calculate input level
    float level = 0.0f;
    for (size_t i = 0; i < frameCount; i++) {
        level += std::abs(samples[i]);
    }
    level /= frameCount;
    inputLevel_ = level;

    // Encode audio
    const int maxPacketSize = 4000;
    unsigned char encodedData[maxPacketSize];
    
    int encodedBytes = codec_->encode(samples, frameCount, encodedData, maxPacketSize);
    if (encodedBytes <= 0) {
        return;
    }

    // Create packet
    AudioPacket packet;
    packet.packetType = static_cast<uint8_t>(PacketType::Audio);
    packet.username = config_.username;
    packet.channel = talkingChannel;
    packet.sequence = sequenceNumber_++;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
    packet.isTalking = anyTalking;
    packet.isListening = isListening_[talkingChannel];
    packet.dataSize = encodedBytes;
    packet.data.assign(encodedData, encodedData + encodedBytes);

    // Send packet
    if (network_->sendPacket(packet)) {
        packetsSent_++;
    }
}

void VoiceChatClient::onPacketReceived(const AudioPacket& packet) {
    // Check if we're listening to this channel
    if (packet.channel < 0 || packet.channel >= 4 || !isListening_[packet.channel]) {
        return;
    }
    
    packetsReceived_++;
    lastSequenceReceived_ = std::max(lastSequenceReceived_, packet.sequence);

    // Decode audio
    std::vector<float> decodedAudio(config_.framesPerBuffer);
    int decodedSamples = codec_->decode(packet.data.data(), packet.dataSize,
                                       decodedAudio.data(), config_.framesPerBuffer);
    
    if (decodedSamples > 0) {
        // Calculate output level
        float level = 0.0f;
        for (int i = 0; i < decodedSamples; i++) {
            level += std::abs(decodedAudio[i]);
        }
        level /= decodedSamples;
        outputLevel_ = level;
        
        // Queue for playback
        audioPlayback_->queueAudio(decodedAudio.data(), decodedSamples);
    }
}

} // namespace VoiceChat
