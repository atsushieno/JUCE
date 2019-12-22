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
    int numUnderRuns{0};

    ALCdevice* device{nullptr};
    ALCcontext* context{nullptr};
    ALuint source, buffers[numBuffers];
    ALuint frequency;
    ALenum format, errorCode{AL_NO_ERROR};

    bool isDeviceOpen{false};

    struct AudioThread : public Thread
    {
        OpenALAudioIODevice* parent{nullptr};
        AudioIODeviceCallback* callback{nullptr};

        AudioThread (OpenALAudioIODevice* parent) : Thread ("OpenAL Audio Thread")
        {
            this->parent = parent;
        }

        ~AudioThread () { }

        void start (AudioIODeviceCallback* callback)
        {
            DBG("Starting OpenAL Audio Thread...");
            this->callback = callback;
            callback->audioDeviceAboutToStart (parent);
            startThread (Thread::realtimeAudioPriority);
        }

        void stop ()
        {
            DBG("Stopping OpenAL Audio Thread...");
            stopThread (500);
            this->callback->audioDeviceStopped();
        }

        // https://kcat.strangesoft.net/openal-tutorial.html
        void run () override
        {
            // int bytePerSampleIn = 2 * parent->numIn;
            int bytePerSampleOut = 2 * parent->numOut;

            while (! threadShouldExit())
            {
                int status = MAIN_THREAD_EM_ASM_INT ({
                    return window.juce_hadUserInteraction;
                });
                if (status) break;
                sleep (5);
            }

            if (threadShouldExit()) return;

            alSourceQueueBuffers (parent->source, parent->numBuffers, parent->buffers);
            alSourcePlay (parent->source);
            if ((parent->errorCode = alGetError()) != AL_NO_ERROR)
            {
                DBG("OpenAL error occurred when starting to play.");
                return;
            }

            int16* formatBuffer = new int16[parent->bufferSize * parent->numOut];

            float** buffersIn;
            float** buffersOut;
            buffersIn  = new float*[16];
            buffersOut = new float*[16];
            for (int i = 0; i < parent->numOut; i ++)
                buffersOut[i] = new float[parent->bufferSize];

            while (! threadShouldExit())
            {
                ALuint buffer;
                ALint val;

                alGetSourcei (parent->source, AL_SOURCE_STATE, & val);
                if(val != AL_PLAYING)
                    alSourcePlay (parent->source);
                
                alGetSourcei (parent->source, AL_BUFFERS_PROCESSED, & val);
                if (val <= 0)
                {
                    // This is bad for audio apps but it seems like without the sleep
                    //   it becomes a busy loop that burns a lot of CPU.
                    sleep (1);
                    continue;
                }
                if (val == parent->numBuffers)
                    parent->numUnderRuns ++;

                // DBG(val);
                while (val --)
                {
                    for (int c = 0; c < parent->numOut; c ++)
                        for (int i = 0; i < parent->bufferSize; i ++)
                            buffersOut[c][i] = 0;
                    
                    callback->audioDeviceIOCallback (
                        (const float**)buffersIn, parent->numIn,
                        buffersOut, parent->numOut, parent->bufferSize);
                    
                    for (int c = 0; c < parent->numOut; c ++)
                    {
                        for (int i = 0; i < parent->bufferSize; i ++)
                            formatBuffer[c + i * parent->numOut] = buffersOut[c][i] * 32768;
                    }
                    
                    alSourceUnqueueBuffers (parent->source, 1, & buffer);
                    alBufferData (buffer, parent->format, formatBuffer,
                        parent->bufferSize * bytePerSampleOut, parent->frequency);
                    alSourceQueueBuffers (parent->source, 1, & buffer);

                    if ((parent->errorCode = alGetError()) != AL_NO_ERROR)
                    {
                        DBG("OpenAL error occurred when playing.");
                        break;
                    }
                }
            }

            delete [] formatBuffer;
            for (int i = 0; i < parent->numOut; i ++)
                delete [] buffersOut[i];
            delete [] buffersOut;
            delete [] buffersIn;
        }
    };

    std::unique_ptr<AudioThread> audioThread;

public:
    OpenALAudioIODevice () : AudioIODevice ("OpenAL", "OpenAL")
    {
        EM_ASM({
            if (window.juce_hadUserInteraction == undefined)
            {
                window.juce_hadUserInteraction = false;
                window.juce_interactionListener = function() {
                    window.juce_hadUserInteraction = true;
                    window.removeEventListener("mousedown",
                      window.juce_interactionListener);
                };
                window.addEventListener("mousedown",
                    window.juce_interactionListener);
            }
        });
    }

    ~OpenALAudioIODevice ()
    {
        if (isDeviceOpen)
            close();
    }

    StringArray getOutputChannelNames () override
    {
        StringArray ret;
        ret.add ("Out #1");
        if (numOut == 2)
          ret.add ("Out #2");
        return ret;
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
        return 1024;
    }

    String open (const BigInteger& inputChannels,
                 const BigInteger& outputChannels,
                 double sampleRate,
                 int bufferSizeSamples) override
    {
        if (isDeviceOpen) close();

        this->bufferSize = bufferSizeSamples;
        this->sampleRate = sampleRate;
        numIn = inputChannels.countNumberOfSetBits ();
        numOut = outputChannels.countNumberOfSetBits ();

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

        if (numOut == 1)
            format = AL_FORMAT_MONO16;
        else if (numOut == 2)
            format = AL_FORMAT_STEREO16;
        else
            return "Invalid output channel configuration.";
        frequency = (ALuint)sampleRate;
        
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
        numUnderRuns = 0;
        audioThread.reset (new AudioThread(this));
        audioThread->start (newCallback);
    }

    void stop () override
    {
        if (isPlaying())
        {
            audioThread->stop ();
        }
    }

    bool isPlaying () override
    {
        return audioThread != nullptr && audioThread->isThreadRunning();
    }

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

    int getOutputLatencyInSamples () override
    {
        return numBuffers * bufferSize;
    }

    int getInputLatencyInSamples () override
    {
        return numBuffers * bufferSize;
    }

    int getXRunCount () const noexcept override
    {
        return numUnderRuns;
    }
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
