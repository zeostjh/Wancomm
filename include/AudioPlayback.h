#pragma once

#include <portaudio.h>
#include <vector>
#include <mutex>
#include <queue>
#include <atomic>
#include "AudioDevice.h"

namespace VoiceChat {

class AudioPlayback {
public:
    AudioPlayback();
    ~AudioPlayback();

    // Get list of available audio devices
    static std::vector<AudioDeviceInfo> getAvailableDevices();
    
    // Get list of output devices only
    static std::vector<AudioDeviceInfo> getOutputDevices();

    // Initialize the audio playback system
    bool initialize(int sampleRate = 48000, int framesPerBuffer = 960, int deviceIndex = -1);
    
    // Start playing audio
    bool start();
    
    // Stop playing audio
    bool stop();
    
    // Check if currently playing
    bool isPlaying() const;
    
    // Queue audio data for playback
    void queueAudio(const float* samples, size_t frameCount);
    
    // Get the current sample rate
    int getSampleRate() const;
    
    // Clear the playback buffer
    void clearBuffer();

private:
    static int audioCallback(const void* input, void* output,
                           unsigned long frameCount,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void* userData);

    PaStream* stream_;
    std::queue<std::vector<float>> audioQueue_;
    std::mutex queueMutex_;
    int sampleRate_;
    int framesPerBuffer_;
    std::atomic<bool> isPlaying_;
    std::vector<float> currentBuffer_;
    size_t currentPosition_;
};

} // namespace VoiceChat
