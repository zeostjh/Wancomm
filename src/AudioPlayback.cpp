#include "AudioPlayback.h"
#include <iostream>
#include <cstring>
#include <algorithm>

namespace VoiceChat {

std::vector<AudioDeviceInfo> AudioPlayback::getAvailableDevices() {
    std::vector<AudioDeviceInfo> devices;
    
    Pa_Initialize();
    int numDevices = Pa_GetDeviceCount();
    int defaultInput = Pa_GetDefaultInputDevice();
    int defaultOutput = Pa_GetDefaultOutputDevice();
    
    for (int i = 0; i < numDevices; i++) {
        const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);
        if (deviceInfo) {
            AudioDeviceInfo info;
            info.index = i;
            info.name = deviceInfo->name;
            info.maxInputChannels = deviceInfo->maxInputChannels;
            info.maxOutputChannels = deviceInfo->maxOutputChannels;
            info.defaultSampleRate = deviceInfo->defaultSampleRate;
            info.isDefaultInput = (i == defaultInput);
            info.isDefaultOutput = (i == defaultOutput);
            devices.push_back(info);
        }
    }
    
    return devices;
}

std::vector<AudioDeviceInfo> AudioPlayback::getOutputDevices() {
    std::vector<AudioDeviceInfo> allDevices = getAvailableDevices();
    std::vector<AudioDeviceInfo> outputDevices;
    
    for (const auto& device : allDevices) {
        if (device.maxOutputChannels > 0) {
            outputDevices.push_back(device);
        }
    }
    
    return outputDevices;
}

AudioPlayback::AudioPlayback()
    : stream_(nullptr)
    , sampleRate_(0)
    , framesPerBuffer_(0)
    , isPlaying_(false)
    , currentPosition_(0) {
}

AudioPlayback::~AudioPlayback() {
    stop();
    if (stream_) {
        Pa_CloseStream(stream_);
    }
}

bool AudioPlayback::initialize(int sampleRate, int framesPerBuffer, int deviceIndex) {
    sampleRate_ = sampleRate;
    framesPerBuffer_ = framesPerBuffer;

    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio initialization failed: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    PaStreamParameters outputParams;
    if (deviceIndex >= 0) {
        outputParams.device = deviceIndex;
    } else {
        outputParams.device = Pa_GetDefaultOutputDevice();
    }
    
    if (outputParams.device == paNoDevice) {
        std::cerr << "No output device found" << std::endl;
        return false;
    }

    outputParams.channelCount = 1;  // Mono
    outputParams.sampleFormat = paFloat32;
    outputParams.suggestedLatency = Pa_GetDeviceInfo(outputParams.device)->defaultLowOutputLatency;
    outputParams.hostApiSpecificStreamInfo = nullptr;

    err = Pa_OpenStream(&stream_,
                       nullptr,  // No input
                       &outputParams,
                       sampleRate_,
                       framesPerBuffer_,
                       paClipOff,
                       &AudioPlayback::audioCallback,
                       this);

    if (err != paNoError) {
        std::cerr << "Failed to open audio stream: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    std::cout << "Audio playback initialized: " << sampleRate << "Hz, " 
              << framesPerBuffer << " frames" << std::endl;
    return true;
}

bool AudioPlayback::start() {
    if (!stream_) {
        std::cerr << "Stream not initialized" << std::endl;
        return false;
    }

    PaError err = Pa_StartStream(stream_);
    if (err != paNoError) {
        std::cerr << "Failed to start stream: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    isPlaying_ = true;
    std::cout << "Audio playback started" << std::endl;
    return true;
}

bool AudioPlayback::stop() {
    if (!stream_ || !isPlaying_) {
        return true;
    }

    isPlaying_ = false;
    PaError err = Pa_StopStream(stream_);
    if (err != paNoError) {
        std::cerr << "Failed to stop stream: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    std::cout << "Audio playback stopped" << std::endl;
    return true;
}

bool AudioPlayback::isPlaying() const {
    return isPlaying_;
}

void AudioPlayback::queueAudio(const float* samples, size_t frameCount) {
    std::lock_guard<std::mutex> lock(queueMutex_);
    std::vector<float> buffer(samples, samples + frameCount);
    audioQueue_.push(std::move(buffer));
}

int AudioPlayback::getSampleRate() const {
    return sampleRate_;
}

void AudioPlayback::clearBuffer() {
    std::lock_guard<std::mutex> lock(queueMutex_);
    while (!audioQueue_.empty()) {
        audioQueue_.pop();
    }
    currentBuffer_.clear();
    currentPosition_ = 0;
}

int AudioPlayback::audioCallback(const void* input, void* output,
                                unsigned long frameCount,
                                const PaStreamCallbackTimeInfo* timeInfo,
                                PaStreamCallbackFlags statusFlags,
                                void* userData) {
    auto* playback = static_cast<AudioPlayback*>(userData);
    float* out = static_cast<float*>(output);
    
    std::lock_guard<std::mutex> lock(playback->queueMutex_);
    
    size_t samplesNeeded = frameCount;
    size_t samplesWritten = 0;

    while (samplesWritten < samplesNeeded) {
        // If current buffer is empty or exhausted, get next buffer from queue
        if (playback->currentBuffer_.empty() || 
            playback->currentPosition_ >= playback->currentBuffer_.size()) {
            if (playback->audioQueue_.empty()) {
                // No more data, output silence
                std::memset(out + samplesWritten, 0, 
                           (samplesNeeded - samplesWritten) * sizeof(float));
                break;
            }
            playback->currentBuffer_ = std::move(playback->audioQueue_.front());
            playback->audioQueue_.pop();
            playback->currentPosition_ = 0;
        }

        // Copy from current buffer
        size_t availableInBuffer = playback->currentBuffer_.size() - playback->currentPosition_;
        size_t toCopy = std::min(availableInBuffer, samplesNeeded - samplesWritten);
        
        std::memcpy(out + samplesWritten,
                   playback->currentBuffer_.data() + playback->currentPosition_,
                   toCopy * sizeof(float));
        
        playback->currentPosition_ += toCopy;
        samplesWritten += toCopy;
    }

    return paContinue;
}

} // namespace VoiceChat
