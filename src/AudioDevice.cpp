#include "AudioDevice.h"
#include "AudioCapture.h"
#include "AudioPlayback.h"

namespace VoiceChat {

std::vector<AudioDeviceInfo> AudioDevice::getInputDevices() {
    return AudioCapture::getInputDevices();
}

std::vector<AudioDeviceInfo> AudioDevice::getOutputDevices() {
    return AudioPlayback::getOutputDevices();
}

} // namespace VoiceChat
