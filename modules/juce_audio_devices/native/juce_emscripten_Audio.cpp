/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{
//==============================================================================
class OpenALAudioIODevice : public AudioIODevice
{
public:
    OpenALAudioIODevice() : AudioIODevice ("OpenAL", "OpenAL")
    {
        // TODO
    }

    ~OpenALAudioIODevice()
    {
        // TODO
    }

    StringArray getOutputChannelNames() override
    {
        return {};
    }

    StringArray getInputChannelNames() override
    {
        return {};
    }

    Array<double> getAvailableSampleRates() override       { return { 44100.0 }; }
    Array<int> getAvailableBufferSizes() override          { return { getDefaultBufferSize() }; }
    int getDefaultBufferSize() override                    { return 256; }

    String open (const BigInteger& inputChannels,
                 const BigInteger& outputChannels,
                 double sampleRate,
                 int bufferSizeSamples) override
    {
        return {};
    }

    void close() override
    {
        // TODO
    }

    bool isOpen() override
    {
        return false;
    }

    void start (AudioIODeviceCallback* newCallback) override
    {
        // TODO
    }

    void stop() override
    {
        // TODO
    }

    bool isPlaying() override         { return false; }
    String getLastError() override    { return {}; }

    //==============================================================================
    int getCurrentBufferSizeSamples() override            { return 256; }
    double getCurrentSampleRate() override                { return 44100.0; }
    int getCurrentBitDepth() override                     { return 16; }
    BigInteger getActiveOutputChannels() const override   { BigInteger b; b.setRange (0, 2, true); return b; }
    BigInteger getActiveInputChannels() const override    { BigInteger b; b.setRange (0, 2, true);  return b; }
    int getOutputLatencyInSamples() override              { /* TODO */ return 0; }
    int getInputLatencyInSamples() override               { /* TODO */ return 0; }
    int getXRunCount() const noexcept override            { return 0; }
};

//==============================================================================
struct OpenALAudioIODeviceType  : public AudioIODeviceType
{
    OpenALAudioIODeviceType() : AudioIODeviceType ("OpenAL") {}

    StringArray getDeviceNames (bool) const override                       { return StringArray ("OpenAL"); }
    void scanForDevices() override                                         {}
    int getDefaultDeviceIndex (bool) const override                        { return 0; }
    int getIndexOfDevice (AudioIODevice* device, bool) const override      { return device != nullptr ? 0 : -1; }
    bool hasSeparateInputsAndOutputs() const override                      { return false; }

    AudioIODevice* createDevice (const String& outputName, const String& inputName) override
    {
        // TODO: switching whether to support analog/digital with possible multiple Bela device types?
        if (outputName == "OpenAL" || inputName == "OpenAL")
            return new OpenALAudioIODevice();

        return nullptr;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenALAudioIODeviceType)
};

//==============================================================================
AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_OpenAL()
{
    return new OpenALAudioIODeviceType();
}


} // namespace juce
