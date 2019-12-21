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
MidiInput::MidiInput (const String& deviceName, const String& deviceID)
    : deviceInfo (deviceName, deviceID) {}
MidiInput::~MidiInput()   { }
void MidiInput::start()   { }
void MidiInput::stop()    { }
Array<MidiDeviceInfo> MidiInput::getAvailableDevices() { return {}; }
MidiDeviceInfo MidiInput::getDefaultDevice() { return {}; }
std::unique_ptr<MidiInput> MidiInput::openDevice (const String& deviceIdentifier, MidiInputCallback* callback) { return nullptr; }
StringArray MidiInput::getDevices() { return {}; }
int MidiInput::getDefaultDeviceIndex() { return 0; }
std::unique_ptr<MidiInput> MidiInput::openDevice (int index, MidiInputCallback* callback) { return nullptr; }

MidiOutput::~MidiOutput()                                                {}
void MidiOutput::sendMessageNow (const MidiMessage&)                     {}
Array<MidiDeviceInfo> MidiOutput::getAvailableDevices()                  { return {}; }
MidiDeviceInfo MidiOutput::getDefaultDevice()                            { return {}; }
std::unique_ptr<MidiOutput> MidiOutput::openDevice (const String&)       { return {}; }
StringArray MidiOutput::getDevices()                                     { return {}; }
int MidiOutput::getDefaultDeviceIndex()                                  { return 0;}
std::unique_ptr<MidiOutput> MidiOutput::openDevice (int)                 { return {}; }

} // namespace juce
