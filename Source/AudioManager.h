/*
 This file is part of SpatGRIS.

 Developers: Samuel Béland, Olivier Bélanger, Nicolas Masson

 SpatGRIS is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 SpatGRIS is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with SpatGRIS.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "macros.h"
DISABLE_WARNINGS
#include <JuceHeader.h>
ENABLE_WARNINGS

#include "Configuration.h"
#include "Input.h"
#include "Manager.hpp"
#include "Speaker.h"
#include "StrongTypes.hpp"
#include "TaggedAudioBuffer.h"
#include "constants.hpp"

class AudioProcessor;

//==============================================================================
class AudioManager final : juce::AudioSourcePlayer
{
    // 2^17 samples * 32 bits per sample == 0.5 mb buffer per channel
    static constexpr auto RECORDERS_BUFFER_SIZE_IN_SAMPLES = 131072;
    //==============================================================================
    struct RecorderInfo {
        std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter> threadedWriter;
        juce::AudioFormatWriter *
            audioFormatWriter; // this is only left for safety assertions : it will get deleted by the threadedWriter
        juce::Array<float const *> dataToRecord;
    };
    //==============================================================================
public:
    struct RecordingOptions {
        juce::String path;
        RecordingFormat format;
        RecordingConfig config;
        double sampleRate;
    };

private:
    juce::CriticalSection mCriticalSection{};

    AudioProcessor * mAudioProcessor{};
    Manager<Speaker, speaker_id_t> const * mSpeakers{};
    juce::OwnedArray<Input> const * mInputs{};

    juce::AudioDeviceManager mAudioDeviceManager{};

    juce::AudioBuffer<float> mInputBuffer{};
    TaggedAudioBuffer<MAX_OUTPUTS> mOutputBuffer{};

    // Recording
    bool mIsRecording{};
    juce::Atomic<int64_t> mNumSamplesRecorded{};
    juce::OwnedArray<RecorderInfo> mRecorders{};
    juce::TimeSliceThread mRecordersThread{ "SpatGRIS recording thread" };
    RecordingConfig mRecordingConfig{};
    //==============================================================================
    static std::unique_ptr<AudioManager> mInstance;

public:
    //==============================================================================
    ~AudioManager();
    //==============================================================================
    AudioManager(AudioManager const &) = delete;
    AudioManager(AudioManager &&) = delete;
    AudioManager & operator=(AudioManager const &) = delete;
    AudioManager & operator=(AudioManager &&) = delete;
    //==============================================================================
    [[nodiscard]] juce::AudioDeviceManager const & getAudioDeviceManager() const { return mAudioDeviceManager; }
    [[nodiscard]] juce::AudioDeviceManager & getAudioDeviceManager() { return mAudioDeviceManager; }

    void registerAudioProcessor(AudioProcessor * audioProcessor,
                                Manager<Speaker, speaker_id_t> const & speakers,
                                juce::OwnedArray<Input> const & inputs);

    juce::StringArray getAvailableDeviceTypeNames();

    bool prepareToRecord(RecordingOptions const & recordingOptions, Manager<Speaker, speaker_id_t> const & speakers);
    void startRecording();
    void stopRecording();
    bool isRecording() const { return mIsRecording; }
    int64_t getNumSamplesRecorded() const { return mNumSamplesRecorded.get(); }
    //==============================================================================
    // AudioSourcePlayer overrides
    void audioDeviceError(const juce::String & errorMessage) override;
    void audioDeviceIOCallback(const float ** inputChannelData,
                               int totalNumInputChannels,
                               float ** outputChannelData,
                               int totalNumOutputChannels,
                               int numSamples) override;
    void audioDeviceAboutToStart(juce::AudioIODevice * device) override;
    void audioDeviceStopped() override;
    //==============================================================================
    static void init(juce::String const & deviceType,
                     juce::String const & inputDevice,
                     juce::String const & outputDevice,
                     double sampleRate,
                     int bufferSize);
    static void free();
    [[nodiscard]] static AudioManager & getInstance();

private:
    //==============================================================================
    AudioManager(juce::String const & deviceType,
                 juce::String const & inputDevice,
                 juce::String const & outputDevice,
                 double sampleRate,
                 int bufferSize);
    //==============================================================================
    [[nodiscard]] bool tryInitAudioDevice(juce::String const & deviceType,
                                          juce::String const & inputDevice,
                                          juce::String const & outputDevice,
                                          double requestedSampleRate,
                                          int requestedBufferSize);
    //==============================================================================
    JUCE_LEAK_DETECTOR(AudioManager)
}; // class AudioManager
