# Build Dobby-Fork Android

## CMake Build Options

```cmake
option(DOBBY_DEBUG "Enable debug logging" OFF)

option(NearBranch "Enable near branch trampoline" ON)

option(FullFloatingPointRegisterPack "Save and pack all floating-point registers" ON)

option(Plugin.SymbolResolver "Enable symbol resolver" ON)

option(Plugin.Android.BionicLinkerUtil "Enable android bionic linker util" ON)

option(DOBBY_BUILD_EXAMPLE "Build example" OFF)

option(DOBBY_BUILD_TEST "Build test" OFF)
```

## Build with `scripts/platform_builder.py`

This is the recommended way to build Dobby for various architectures.

#### Build for Android

```bash
# Example: Build for all supported Android architectures
python3 scripts/platform_builder.py --platform=android --arch=all --ndk=/path/to/android-ndk
```

#### Build for Linux

```bash
python3 scripts/platform_builder.py --platform=linux --arch=x86_64
```

## Build with CMake (Manual)

#### Build for Android

```bash
mkdir build-android && cd build-android
cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-21 \
  -DDOBBY_DEBUG=ON
make -j4
```

#### Build for Host (Linux)

```bash
mkdir build-host && cd build-host
cmake ..
make -j4
```

## Integrating with Android Studio

Add Dobby as a subdirectory in your `CMakeLists.txt`:

```cmake
set(DOBBY_DIR ${CMAKE_SOURCE_DIR}/external/dobby)
add_subdirectory(${DOBBY_DIR} dobby)

target_link_libraries(your-native-lib dobby)
```
