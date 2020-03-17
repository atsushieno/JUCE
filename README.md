# Port of [JUCE](http://www.juce.com/) for the browser via Emscripten

This port was originally a proof-of-concept created [here](https://github.com/beschulz/juce_emscripten) by @beschulz. This fork is an unofficial continuation of the attempt towards a complete JUCE framework running inside a browser.

Play with JUCE [DemoRunner](https://synthesizerv.com/lab/wasm-juce-demorunner/DemoRunner.html) in your browser.

## Status

- `juce_analytics`: not supported
- `juce_audio_basics`: fully supported
- `juce_audio_devices`: partial support
   - Audio input: not supported
   - Audio output: supported through Emscripten's OpenAL API (`OpenALAudioIODevice`)
   - MIDI input/output: not supported
- `juce_audio_formats`: fully supported
- `juce_audio_plugin_client`: not supported (with no plan to port)
- `juce_audio_processors`: partial support (no supported plugin format)
- `juce_audio_utils`: fully supported
- `juce_box2d`: fully supported
- `juce_core`: all except network and `MemoryMappedFile` are supported
   - File: based on Emscripten's memory file system; directories such as `/tmp` and `/home` are created on startup.
   - Logging: `DBG(...)` prints to console (`std::cerr`), not emrun console.
   - Threads: without `-s PROXY_TO_PTHREAD=1` linker flag, threading is subjected to some [platform-specific limitations](https://emscripten.org/docs/porting/pthreads.html) - notably, the program will hang if you spawn new threads from the main thread and wait for them to start within the same message dispatch cycle. Toggle this linker flag to run the message loop on a pthread and you will have full threading support.
   - SystemStats: operating system maps to browser `userAgent` info; number of logical/physical CPUs is `navigator.hardwareConcurrency`; memory size is javascript heap size, which could be different from what's available to WASM module; CPU speed is set to 1000 MHz.
- `juce_cryptography`: fully supported
- `juce_data_structures`: fully supported
- `juce_dsp`: all except SIMD features are supported
- `juce_events`: fully supported; the message loop is [synchronized with browser repaints](https://emscripten.org/docs/api_reference/emscripten.h.html#c.emscripten_set_main_loop).
- `juce_graphics`: fully supported; font rendering is based on freetype.
- `juce_gui_basics`: mostly supported
   - Clipboard: the first paste will fail due to security restrictions. With the user's permission, following pastes will succeed.
   - Input Method: works but without showing the characters being typed in until finish.
   - Native window title bar: not supported.
   - Native dialogs: not supported. File open/close dialogs are especially tricky. Passing data in and out is not hard if we use HTML5 input, however, interfacing with the in-memory file system is the real problem.
- `juce_gui_extra`: fully supported
- `juce_opengl`: not supported
- `juce_osc`: not supported
- `juce_product_unlocking`: all supported except features that depend on networking.
- `juce_video`: not supported.

## Build instructions

- [Download Emscripten](https://emscripten.org/docs/getting_started/downloads.html)
- install Emscripten
```shell
# Fetch the latest registry of available tools.
./emsdk update

# Download and install the latest SDK tools.
./emsdk install latest

# Make the "latest" SDK "active"
./emsdk activate latest

# Set the current Emscripten path on Linux/Mac OS X
source ./emsdk_env.sh
```

- compile the sample
```shell
cd examples/DemoRunner/Builds/Emscripten/
emmake make
cd build
python -m SimpleHTTPServer
```
- Goto http://127.0.0.1:8000

Note: I had to modify the auto-generated Makefile to get everything to work. So be carefull when you modify and save the jucer project. In the long run, it would be nice to have a emscripten target inside Projucer.

## Firefox support

As of late 2019, stable and beta releases of Firefox have `SharedArrayBuffer` support removed due to [security concerns](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/SharedArrayBuffer#Browser_compatibility) (brought by Spectre/Meltdown). Nightly builds do have the support but they also require `Cross-Origin-Opener-Policy` and `Cross-Origin-Embedder-Policy` attributes being set in the HTTP header.

A quick and dirty way to bypass this restriction is to go to `about:config` and set `dom.postMessage.sharedArrayBuffer.bypassCOOP_COEP.insecure.enabled` to true. However, this is a bit risky and is not the best way of doing this.

The better way is to (1) set the attributes mentioned above in the server you're hosting the WASM application and (2) enable CORP/COEP flags in Firefox. See [this issue](https://github.com/emscripten-core/emscripten/issues/10014) in the Emscripten repository for detailed instructions.

## Tutorial (contributed by @atsushieno)

[English] https://atsushieno.github.io/2020/01/01/juce-emscripten-the-latest-juce-on-webassembly.html

[Japanese] http://atsushieno.hatenablog.com/entry/2020/01/01/121050

## Licensing

See JUCE licensing below.

[Lato](http://www.latofonts.com/lato-free-fonts/) (The Font I used for the example) is licensed under SIL Open Font License.

JUCE is an open-source cross-platform C++ application framework used for rapidly
developing high quality desktop and mobile applications, including VST, AU (and AUv3),
RTAS and AAX audio plug-ins. JUCE can be easily integrated with existing projects or can
be used as a project generation tool via the [Projucer](https://juce.com/discover/projucer),
which supports exporting projects for Xcode (macOS and iOS), Visual Studio, Android Studio,
Code::Blocks, CLion and Linux Makefiles as well as containing a source code editor and
live-coding engine which can be used for rapid prototyping.

---

## Getting Started
The JUCE repository contains a [master](https://github.com/weareroli/JUCE/tree/master)
and [develop](https://github.com/weareroli/JUCE/tree/develop) branch. The develop branch
contains the latest bugfixes and features and is periodically merged into the master
branch in stable [tagged releases](https://github.com/WeAreROLI/JUCE/releases)
(the latest release containing pre-built binaries can be also downloaded from the
[JUCE website](https://shop.juce.com/get-juce)).

The repository doesn't contain a pre-built Projucer so you will need to build it
for your platform - Xcode, Visual Studio and Linux Makefile projects are located in
[extras/Projucer/Builds](/extras/Projucer/Builds)
(the minumum system requirements are listed in the __System Requirements__ section below).
The Projucer can then be used to create new JUCE projects, view tutorials and run examples.
It is also possible to include the JUCE modules source code in an existing project directly,
or build them into a static or dynamic library which can be linked into a project.

For further help getting started, please refer to the JUCE
[documentation](https://juce.com/learn/documentation) and
[tutorials](https://juce.com/learn/tutorials).

## Minimum System Requirements
#### Building JUCE Projects
- __macOS__: macOS 10.11 and Xcode 7.3.1
- __Windows__: Windows 8.1 and Visual Studio 2015 64-bit
- __Linux__: GCC 4.8

#### Deployment Targets
- __macOS__: macOS 10.7
- __Windows__: Windows Vista
- __Linux__: Mainstream Linux distributions

## Contributing
For bug reports and features requests, please visit the [JUCE Forum](https://forum.juce.com/) -
the JUCE developers are active there and will read every post and respond accordingly. When
submitting a bug report, please ensure that it follows the
[issue template](/.github/ISSUE_TEMPLATE.txt).
We don't accept third party GitHub pull requests directly due to copyright restrictions
but if you would like to contribute any changes please contact us.

## License
The core JUCE modules (juce_audio_basics, juce_audio_devices, juce_blocks_basics, juce_core
and juce_events) are permissively licensed under the terms of the
[ISC license](http://www.isc.org/downloads/software-support-policy/isc-license/).
Other modules are covered by a
[GPL/Commercial license](https://www.gnu.org/licenses/gpl-3.0.en.html).

There are multiple commercial licensing tiers for JUCE 5, with different terms for each:
- JUCE Personal (developers or startup businesses with revenue under 50K USD) - free
- JUCE Indie (small businesses with revenue under 200K USD) - $35/month
- JUCE Pro (no revenue limit) - $65/month
- JUCE Educational (no revenue limit) - free for bona fide educational institutes

For full terms see [LICENSE.md](LICENSE.md).
