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

int getAudioContextSampleRate() {
    return EM_ASM_INT({
        var AudioContext = window.AudioContext || window.webkitAudioContext;
        var ctx = new AudioContext();
        var sr = ctx.sampleRate;
        ctx.close();
        return sr;
    });
}

//==============================================================================
class OpenALAudioIODevice : public AudioIODevice
{
    static const int numBuffers{3};
    int bufferSize{512};
    double sampleRate{44100.0};
    int numIn{0}, numOut{0};

    ALCdevice* device{nullptr};
    ALCcontext* context{nullptr};
    ALuint source, buffers[numBuffers];
    ALuint frequency;
    ALenum format, errorCode{AL_NO_ERROR};

    bool isDeviceOpen{false};

public:
    OpenALAudioIODevice () : AudioIODevice ("OpenAL", "OpenAL")
    {
    }

    ~OpenALAudioIODevice ()
    {
        if (isDeviceOpen)
            close();
    }

    StringArray getOutputChannelNames () override
    {
        return { "Out #1", "Out #2" };
    }

    StringArray getInputChannelNames () override
    {
        return {};
    }

    Array<double> getAvailableSampleRates () override {
        return { getAudioContextSampleRate() };
    }
    
    Array<int> getAvailableBufferSizes () override 
    {
        return { 128, 256, 384, 512, 768, 1024, 1536, 2048, 3072, 4096, 8192 };
    }
    
    int getDefaultBufferSize () override
    {
        return 512;
    }

    String open (const BigInteger& inputChannels,
                 const BigInteger& outputChannels,
                 double sampleRate,
                 int bufferSizeSamples) override
    {
        this->bufferSize = bufferSizeSamples;
        this->sampleRate = sampleRate;
        
        ALenum errorCode = alGetError ();
        device = alcOpenDevice (nullptr);
        if (device == nullptr)
            return "Failed to open device.";
        DBG("OpenAL device has opened.");

        context = alcCreateContext (device, nullptr);
        alcMakeContextCurrent (context);
        if (context == nullptr || (errorCode = alGetError()) != AL_NO_ERROR)
            return "Failed to create context.";
        DBG("OpenAL context is created.");

        alGenBuffers (numBuffers, buffers);
        alGenSources (1, & source);
        if ((errorCode = alGetError()) != AL_NO_ERROR)
            return "Failed to generate sources.";
        DBG("OpenAL sources and buffers are generated.");

        numIn = inputChannels.countNumberOfSetBits();
        numOut = outputChannels.countNumberOfSetBits();

        if (numOut == 1)
            format = AL_FORMAT_MONO16;
        else if (numOut == 2)
            format = AL_FORMAT_STEREO16;
        else
            return "Invalid output channel configuration.";
        
        isDeviceOpen = true;

        return {};
    }

    void close () override
    {
        if (isDeviceOpen)
        {
            alDeleteSources (1, & source);
            alDeleteBuffers (numBuffers, buffers);
        }
        if (context)
        {
            alcMakeContextCurrent (nullptr);
            alcDestroyContext (context);
            context = nullptr;
        }
        if (device)
        {
            alcCloseDevice (device);
            device = nullptr;
        }
        isDeviceOpen = false;
    }

    bool isOpen () override
    {
        return isDeviceOpen;
    }

    void start (AudioIODeviceCallback* newCallback) override
    {
        // TODO
    }

    void stop () override
    {
        // TODO
    }

    bool isPlaying () override         { return false; }

    String getLastError () override
    {
        switch (errorCode)
        {
        case AL_NO_ERROR:
            return "";
        case AL_INVALID_NAME:
            return "AL_INVALID_NAME";
        case AL_INVALID_ENUM:
            return "AL_INVALID_NAME";
        case AL_INVALID_VALUE:
            return "AL_INVALID_VALUE";
        case AL_INVALID_OPERATION:
            return "AL_INVALID_OPERATION";
        case AL_OUT_OF_MEMORY:
            return "AL_OUT_OF_MEMORY";
        default:
            return "UNDEFINED_ERROR";
        }
    }

    //==============================================================================
    int getCurrentBufferSizeSamples () override           { return bufferSize; }
    double getCurrentSampleRate () override               { return sampleRate; }
    int getCurrentBitDepth () override                    { return 16; }

    BigInteger getActiveOutputChannels () const override
    {
        BigInteger b;
        b.setRange (0, numIn, true);
        return b;
    }
    BigInteger getActiveInputChannels () const override {
        BigInteger b;
        b.setRange (0, numOut, true);
        return b;
    }

    int getOutputLatencyInSamples () override             { /* TODO */ return 0; }
    int getInputLatencyInSamples () override              { /* TODO */ return 0; }
    int getXRunCount () const noexcept override           { return 0; }
};

//==============================================================================
struct OpenALAudioIODeviceType  : public AudioIODeviceType
{
    OpenALAudioIODeviceType () : AudioIODeviceType ("OpenAL") {}

    StringArray getDeviceNames (bool) const override                       { return StringArray ("OpenAL"); }
    void scanForDevices () override                                        {}
    int getDefaultDeviceIndex (bool) const override                        { return 0; }
    int getIndexOfDevice (AudioIODevice* device, bool) const override      { return device != nullptr ? 0 : -1; }
    bool hasSeparateInputsAndOutputs () const override                     { return false; }

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
AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_OpenAL ()
{
    return new OpenALAudioIODeviceType();
}


} // namespace juce
