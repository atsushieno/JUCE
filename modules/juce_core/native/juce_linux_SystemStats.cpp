/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

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

#if JUCE_BELA
extern "C" int cobalt_thread_mode();
#elif JUCE_EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/threading.h>
#include <deque>
#endif

namespace juce
{

#if JUCE_EMSCRIPTEN
std::deque<std::string> debugPrintQueue;
std::mutex debugPrintQueueMtx;
#endif

void Logger::outputDebugString (const String& text)
{
   #if JUCE_EMSCRIPTEN
    if (Thread::getCurrentThread() == nullptr)
        std::cout << text << std::endl;
    else
    {
        debugPrintQueueMtx.lock();
        debugPrintQueue.push_back (text.toStdString());
        debugPrintQueueMtx.unlock();
    }
   #else
    std::cerr << text << std::endl;
   #endif
}

//==============================================================================
SystemStats::OperatingSystemType SystemStats::getOperatingSystemType()
{
    return Linux;
}

String SystemStats::getOperatingSystemName()
{
   #if JUCE_EMSCRIPTEN
    char* str = (char*)MAIN_THREAD_EM_ASM_INT({
        var userAgent = navigator.userAgent;
        var lengthBytes = lengthBytesUTF8(userAgent) + 1;
        var heapBytes = _malloc(lengthBytes);
        stringToUTF8(userAgent, heapBytes, lengthBytes);
        return heapBytes;
    });
    String ret(str);
    free((void*)str);
    return ret;
   #else
    return "Linux";
   #endif
}

bool SystemStats::isOperatingSystem64Bit()
{
   #if JUCE_64BIT && ! JUCE_EMSCRIPTEN
    return true;
   #else
    //xxx not sure how to find this out?..
    return false;
   #endif
}

//==============================================================================
static String getCpuInfo (const char* key)
{
    #if JUCE_EMSCRIPTEN
    return "";
    #else
    return readPosixConfigFileValue ("/proc/cpuinfo", key);
    #endif
}

String SystemStats::getDeviceDescription()
{
   #if JUCE_EMSCRIPTEN
    char* str = (char*)MAIN_THREAD_EM_ASM_INT({
        var platform = navigator.platform;
        var lengthBytes = lengthBytesUTF8(platform) + 1;
        var heapBytes = _malloc(lengthBytes);
        stringToUTF8(platform, heapBytes, lengthBytes);
        return heapBytes;
    });
    String ret(str);
    free((void*)str);
    return ret;
   #else
    return getCpuInfo ("Hardware");
   #endif
}

String SystemStats::getDeviceManufacturer()
{
   #if JUCE_EMSCRIPTEN
    char* str = (char*)MAIN_THREAD_EM_ASM_INT({
        var vendor = navigator.vendor;
        var lengthBytes = lengthBytesUTF8(vendor) + 1;
        var heapBytes = _malloc(lengthBytes);
        stringToUTF8(vendor, heapBytes, lengthBytes);
        return heapBytes;
    });
    String ret(str);
    free((void*)str);
    return ret;
   #else
    return {};
   #endif
}

String SystemStats::getCpuVendor()
{
   #if JUCE_EMSCRIPTEN
    return getDeviceManufacturer();
   #else
    auto v = getCpuInfo ("vendor_id");

    if (v.isEmpty())
        v = getCpuInfo ("model name");

    return v;
   #endif
}

String SystemStats::getCpuModel()
{
   #if JUCE_EMSCRIPTEN
    return "emscripten";
   #else
    return getCpuInfo ("model name");
   #endif
}

int SystemStats::getCpuSpeedInMegahertz()
{
   #if JUCE_EMSCRIPTEN
    return 1000;
   #else
    return roundToInt (getCpuInfo ("cpu MHz").getFloatValue());
   #endif
}

int SystemStats::getMemorySizeInMegabytes()
{
   #if JUCE_EMSCRIPTEN
    int heapSizeLimit = MAIN_THREAD_EM_ASM_INT({
        if (performance != undefined)
            if (performance.memory != undefined)
                return performance.memory.jsHeapSizeLimit / 1024 / 1024;
        return 128; // some arbitrary number just to get things working (hopefully)
    });
    return heapSizeLimit;
   #else
    struct sysinfo sysi;

    if (sysinfo (&sysi) == 0)
        return (int) (sysi.totalram * sysi.mem_unit / (1024 * 1024));
   #endif

    return 0;
}

int SystemStats::getPageSize()
{
    return (int) sysconf (_SC_PAGESIZE);
}

//==============================================================================
String SystemStats::getLogonName()
{
    if (auto user = getenv ("USER"))
        return String::fromUTF8 (user);

    if (auto pw = getpwuid (getuid()))
        return String::fromUTF8 (pw->pw_name);

    return {};
}

String SystemStats::getFullUserName()
{
    return getLogonName();
}

String SystemStats::getComputerName()
{
    char name[256] = {};

    if (gethostname (name, sizeof (name) - 1) == 0)
        return name;

    return {};
}

static String getLocaleValue (nl_item key)
{
    auto oldLocale = ::setlocale (LC_ALL, "");
    auto result = String::fromUTF8 (nl_langinfo (key));
    ::setlocale (LC_ALL, oldLocale);
    return result;
}

#if JUCE_EMSCRIPTEN
String SystemStats::getDisplayLanguage()
{
    char* str = (char*)MAIN_THREAD_EM_ASM_INT({
        var language = navigator.language;
        var lengthBytes = lengthBytesUTF8(language) + 1;
        var heapBytes = _malloc(lengthBytes);
        stringToUTF8(language, heapBytes, lengthBytes);
        return heapBytes;
    });
    String ret(str);
    free((void*)str);
    return ret;
}

String SystemStats::getUserLanguage()
{
    String langRegion = getDisplayLanguage();
    return langRegion.upToFirstOccurrenceOf("-", false, true);
}

String SystemStats::getUserRegion()
{
    String langRegion = getDisplayLanguage();
    return langRegion.fromFirstOccurrenceOf("-", false, true);
}

#else
String SystemStats::getUserLanguage()     { return getLocaleValue (_NL_IDENTIFICATION_LANGUAGE); }
String SystemStats::getUserRegion()       { return getLocaleValue (_NL_IDENTIFICATION_TERRITORY); }
String SystemStats::getDisplayLanguage()  { return getUserLanguage() + "-" + getUserRegion(); }
#endif

//==============================================================================
void CPUInformation::initialise() noexcept
{
    auto flags = getCpuInfo ("flags");

    hasMMX             = flags.contains ("mmx");
    hasFMA3            = flags.contains ("fma");
    hasFMA4            = flags.contains ("fma4");
    hasSSE             = flags.contains ("sse");
    hasSSE2            = flags.contains ("sse2");
    hasSSE3            = flags.contains ("sse3");
    has3DNow           = flags.contains ("3dnow");
    hasSSSE3           = flags.contains ("ssse3");
    hasSSE41           = flags.contains ("sse4_1");
    hasSSE42           = flags.contains ("sse4_2");
    hasAVX             = flags.contains ("avx");
    hasAVX2            = flags.contains ("avx2");
    hasAVX512F         = flags.contains ("avx512f");
    hasAVX512BW        = flags.contains ("avx512bw");
    hasAVX512CD        = flags.contains ("avx512cd");
    hasAVX512DQ        = flags.contains ("avx512dq");
    hasAVX512ER        = flags.contains ("avx512er");
    hasAVX512IFMA      = flags.contains ("avx512ifma");
    hasAVX512PF        = flags.contains ("avx512pf");
    hasAVX512VBMI      = flags.contains ("avx512vbmi");
    hasAVX512VL        = flags.contains ("avx512vl");
    hasAVX512VPOPCNTDQ = flags.contains ("avx512_vpopcntdq");
   
   #if JUCE_EMSCRIPTEN
    numLogicalCPUs = emscripten_num_logical_cores();
    numPhysicalCPUs = numLogicalCPUs;
   #else
    numLogicalCPUs  = getCpuInfo ("processor").getIntValue() + 1;

    // Assume CPUs in all sockets have the same number of cores
    numPhysicalCPUs = getCpuInfo ("cpu cores").getIntValue() * (getCpuInfo ("physical id").getIntValue() + 1);
   #endif

    if (numPhysicalCPUs <= 0)
        numPhysicalCPUs = numLogicalCPUs;
}

//==============================================================================
uint32 juce_millisecondsSinceStartup() noexcept
{
    return (uint32) (Time::getHighResolutionTicks() / 1000);
}

int64 Time::getHighResolutionTicks() noexcept
{
    timespec t;

   #if JUCE_BELA
    if (cobalt_thread_mode() == 0x200 /*XNRELAX*/)
        clock_gettime (CLOCK_MONOTONIC, &t);
    else
        __wrap_clock_gettime (CLOCK_MONOTONIC, &t);
   #else
    clock_gettime (CLOCK_MONOTONIC, &t);
   #endif

    return (t.tv_sec * (int64) 1000000) + (t.tv_nsec / 1000);
}

int64 Time::getHighResolutionTicksPerSecond() noexcept
{
    return 1000000;  // (microseconds)
}

double Time::getMillisecondCounterHiRes() noexcept
{
    return (double) getHighResolutionTicks() * 0.001;
}

bool Time::setSystemTimeToThisTime() const
{
    timeval t;
    t.tv_sec = decltype (timeval::tv_sec) (millisSinceEpoch / 1000);
    t.tv_usec = decltype (timeval::tv_usec) ((millisSinceEpoch - t.tv_sec * 1000) * 1000);

    return settimeofday (&t, nullptr) == 0;
}

JUCE_API bool JUCE_CALLTYPE juce_isRunningUnderDebugger() noexcept
{
   #if JUCE_BSD || JUCE_EMSCRIPTEN
    return false;
   #else
    return readPosixConfigFileValue ("/proc/self/status", "TracerPid").getIntValue() > 0;
   #endif
}

} // namespace juce
