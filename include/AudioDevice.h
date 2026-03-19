#pragma once

#include <string>
#include <vector>

namespace VoiceChat {

struct AudioDeviceInfo {
    int index;
    std::string name;
    int maxInputChannels;
    int maxOutputChannels;
    double defaultSampleRate;
    bool isDefaultInput;
    bool isDefaultOutput;
};

// Forward declarations
class AudioCapture;
class AudioPlayback;

class AudioDevice {
public:
    // Get list of available input devices
    static std::vector<AudioDeviceInfo> getInputDevices();
    
    // Get list of available output devices
    static std::vector<AudioDeviceInfo> getOutputDevices();
};

} // namespace VoiceChat
