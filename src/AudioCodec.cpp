#include "AudioCodec.h"
#include <iostream>

namespace VoiceChat {

AudioCodec::AudioCodec()
    : encoder_(nullptr)
    , decoder_(nullptr)
    , sampleRate_(0)
    , channels_(0)
    , bitrate_(0) {
}

AudioCodec::~AudioCodec() {
    if (encoder_) {
        opus_encoder_destroy(encoder_);
    }
    if (decoder_) {
        opus_decoder_destroy(decoder_);
    }
}

bool AudioCodec::initializeEncoder(int sampleRate, int channels, int bitrate) {
    sampleRate_ = sampleRate;
    channels_ = channels;
    bitrate_ = bitrate;

    int error;
    encoder_ = opus_encoder_create(sampleRate, channels, OPUS_APPLICATION_VOIP, &error);
    
    if (error != OPUS_OK) {
        std::cerr << "Failed to create Opus encoder: " << opus_strerror(error) << std::endl;
        return false;
    }

    opus_encoder_ctl(encoder_, OPUS_SET_BITRATE(bitrate));
    opus_encoder_ctl(encoder_, OPUS_SET_VBR(1));  // Variable bitrate
    opus_encoder_ctl(encoder_, OPUS_SET_COMPLEXITY(10));  // Max quality

    std::cout << "Opus encoder initialized: " << sampleRate << "Hz, " 
              << channels << " channels, " << bitrate << " bps" << std::endl;
    return true;
}

bool AudioCodec::initializeDecoder(int sampleRate, int channels) {
    if (sampleRate_ == 0) {
        sampleRate_ = sampleRate;
        channels_ = channels;
    }

    int error;
    decoder_ = opus_decoder_create(sampleRate, channels, &error);
    
    if (error != OPUS_OK) {
        std::cerr << "Failed to create Opus decoder: " << opus_strerror(error) << std::endl;
        return false;
    }

    std::cout << "Opus decoder initialized: " << sampleRate << "Hz, " 
              << channels << " channels" << std::endl;
    return true;
}

int AudioCodec::encode(const float* pcm, int frameSize, unsigned char* data, int maxDataBytes) {
    if (!encoder_) {
        std::cerr << "Encoder not initialized" << std::endl;
        return -1;
    }

    int nbBytes = opus_encode_float(encoder_, pcm, frameSize, data, maxDataBytes);
    
    if (nbBytes < 0) {
        std::cerr << "Encoding failed: " << opus_strerror(nbBytes) << std::endl;
        return -1;
    }

    return nbBytes;
}

int AudioCodec::decode(const unsigned char* data, int dataSize, float* pcm, int frameSize) {
    if (!decoder_) {
        std::cerr << "Decoder not initialized" << std::endl;
        return -1;
    }

    int numSamples = opus_decode_float(decoder_, data, dataSize, pcm, frameSize, 0);
    
    if (numSamples < 0) {
        std::cerr << "Decoding failed: " << opus_strerror(numSamples) << std::endl;
        return -1;
    }

    return numSamples;
}

int AudioCodec::getSampleRate() const {
    return sampleRate_;
}

int AudioCodec::getChannels() const {
    return channels_;
}

} // namespace VoiceChat
