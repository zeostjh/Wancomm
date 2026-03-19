#pragma once

#include <portaudio.h>
#include <functional>
#include <vector>
#include <atomic>
#include <memory>
#include "AudioDevice.h"

namespace VoiceChat {

class AudioCapture {
public:
    using AudioCallback = std::function<void(const float* samples, size_t frameCount)>;

    AudioCapture();
    ~AudioCapture();

    // Get list of available audio devices
    static std::vector<AudioDeviceInfo> getAvailableDevices();
    
    // Get list of input devices only
    static std::vector<AudioDeviceInfo> getInputDevices();

    // Initialize the audio capture system
    bool initialize(int sampleRate = 48000, int framesPerBuffer = 960, int deviceIndex = -1);
    
    // Start capturing audio
    bool start();
    
    // Stop capturing audio
    bool stop();
    
    // Check if currently capturing
    bool isCapturing() const;
    
    // Set callback for when audio data is captured
    void setAudioCallback(AudioCallback callback);
    
    // Get the current sample rate
    int getSampleRate() const;

private:
    static int audioCallback(const void* input, void* output,
                           unsigned long frameCount,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void* userData);

    PaStream* stream_;
    AudioCallback callback_;
    int sampleRate_;
    int framesPerBuffer_;
    std::atomic<bool> isCapturing_;
};

} // namespace VoiceChat
