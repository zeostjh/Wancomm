#include "AudioCapture.h"
#include <iostream>
#include <cstring>

namespace VoiceChat {

std::vector<AudioDeviceInfo> AudioCapture::getAvailableDevices() {
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

std::vector<AudioDeviceInfo> AudioCapture::getInputDevices() {
    std::vector<AudioDeviceInfo> allDevices = getAvailableDevices();
    std::vector<AudioDeviceInfo> inputDevices;
    
    for (const auto& device : allDevices) {
        if (device.maxInputChannels > 0) {
            inputDevices.push_back(device);
        }
    }
    
    return inputDevices;
}

AudioCapture::AudioCapture()
    : stream_(nullptr)
    , sampleRate_(0)
    , framesPerBuffer_(0)
    , isCapturing_(false) {
}

AudioCapture::~AudioCapture() {
    stop();
    if (stream_) {
        Pa_CloseStream(stream_);
    }
}

bool AudioCapture::initialize(int sampleRate, int framesPerBuffer, int deviceIndex) {
    sampleRate_ = sampleRate;
    framesPerBuffer_ = framesPerBuffer;

    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio initialization failed: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    PaStreamParameters inputParams;
    if (deviceIndex >= 0) {
        inputParams.device = deviceIndex;
    } else {
        inputParams.device = Pa_GetDefaultInputDevice();
    }
    
    if (inputParams.device == paNoDevice) {
        std::cerr << "No input device found" << std::endl;
        return false;
    }

    inputParams.channelCount = 1;  // Mono
    inputParams.sampleFormat = paFloat32;
    inputParams.suggestedLatency = Pa_GetDeviceInfo(inputParams.device)->defaultLowInputLatency;
    inputParams.hostApiSpecificStreamInfo = nullptr;

    err = Pa_OpenStream(&stream_,
                       &inputParams,
                       nullptr,  // No output
                       sampleRate_,
                       framesPerBuffer_,
                       paClipOff,
                       &AudioCapture::audioCallback,
                       this);

    if (err != paNoError) {
        std::cerr << "Failed to open audio stream: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    std::cout << "Audio capture initialized: " << sampleRate << "Hz, " 
              << framesPerBuffer << " frames" << std::endl;
    return true;
}

bool AudioCapture::start() {
    if (!stream_) {
        std::cerr << "Stream not initialized" << std::endl;
        return false;
    }

    PaError err = Pa_StartStream(stream_);
    if (err != paNoError) {
        std::cerr << "Failed to start stream: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    isCapturing_ = true;
    std::cout << "Audio capture started" << std::endl;
    return true;
}

bool AudioCapture::stop() {
    if (!stream_ || !isCapturing_) {
        return true;
    }

    isCapturing_ = false;
    PaError err = Pa_StopStream(stream_);
    if (err != paNoError) {
        std::cerr << "Failed to stop stream: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    std::cout << "Audio capture stopped" << std::endl;
    return true;
}

bool AudioCapture::isCapturing() const {
    return isCapturing_;
}

void AudioCapture::setAudioCallback(AudioCallback callback) {
    callback_ = callback;
}

int AudioCapture::getSampleRate() const {
    return sampleRate_;
}

int AudioCapture::audioCallback(const void* input, void* output,
                               unsigned long frameCount,
                               const PaStreamCallbackTimeInfo* timeInfo,
                               PaStreamCallbackFlags statusFlags,
                               void* userData) {
    auto* capture = static_cast<AudioCapture*>(userData);
    const float* samples = static_cast<const float*>(input);

    if (capture->callback_ && samples) {
        capture->callback_(samples, frameCount);
    }

    return paContinue;
}

} // namespace VoiceChat
