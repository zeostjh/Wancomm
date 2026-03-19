#pragma once

#include <opus/opus.h>
#include <vector>
#include <memory>

namespace VoiceChat {

class AudioCodec {
public:
    AudioCodec();
    ~AudioCodec();

    // Initialize encoder
    bool initializeEncoder(int sampleRate = 48000, int channels = 1, int bitrate = 24000);
    
    // Initialize decoder
    bool initializeDecoder(int sampleRate = 48000, int channels = 1);
    
    // Encode audio samples to compressed data
    // Returns the number of bytes encoded, or negative on error
    int encode(const float* pcm, int frameSize, unsigned char* data, int maxDataBytes);
    
    // Decode compressed audio data to samples
    // Returns the number of samples decoded, or negative on error
    int decode(const unsigned char* data, int dataSize, float* pcm, int frameSize);
    
    // Get the configured sample rate
    int getSampleRate() const;
    
    // Get the number of channels
    int getChannels() const;

private:
    OpusEncoder* encoder_;
    OpusDecoder* decoder_;
    int sampleRate_;
    int channels_;
    int bitrate_;
};

} // namespace VoiceChat
