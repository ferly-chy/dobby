cmake_minimum_required(VERSION 3.10)
project(Dobby)
enable_language(ASM)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Global options for vendor libraries
set(FMT_INSTALL OFF CACHE BOOL "" FORCE)
set(FMT_TEST OFF CACHE BOOL "" FORCE)
set(SPDLOG_INSTALL OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_PIC ON CACHE BOOL "" FORCE)
set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "" FORCE)
set(SPDLOG_NO_EXCEPTIONS ON CACHE BOOL "" FORCE)
set(JSON_Install OFF CACHE BOOL "" FORCE)
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ELFIO_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ELFIO_BUILD_TESTS OFF CACHE BOOL "" FORCE)

# Add vendor subdirectories
add_subdirectory(vendor/fmt)
add_subdirectory(vendor/spdlog)
add_subdirectory(vendor/json)
add_subdirectory(vendor/tomlplusplus)
add_subdirectory(vendor/elfio)

include(cmake/Util.cmake)
include(cmake/Macros.cmake)
include(cmake/build_environment_check.cmake)
include(cmake/auto_source_group.cmake)
include(CTest)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_C_STANDARD 23)

auto_source_group("." "auto-source-group" "\\.(cc|cpp|c|h)$")

# --- options ---
option(DOBBY_DEBUG "Enable debug logging" OFF)
option(NearBranch "Enable near branch trampoline" ON)
option(FullFloatingPointRegisterPack "Save and pack all floating-point registers" ON)
option(Plugin.SymbolResolver "Enable symbol resolver" ON)
option(Plugin.ImportTableReplace "Enable import table replace " ON)
option(Plugin.Android.BionicLinkerUtil "Enable android bionic linker util" ON)
option(DOBBY_BUILD_EXAMPLE "Build example" OFF)
option(DOBBY_BUILD_TEST "Build test" OFF)
option(Private.Obfuscation "Enable llvm obfuscation" OFF)

if ((NOT DEFINED CMAKE_BUILD_TYPE) OR (CMAKE_BUILD_TYPE STREQUAL "Debug"))
  set(DOBBY_DEBUG ON)
endif ()

set(compile_definitions "")
if (FullFloatingPointRegisterPack)
  set(compile_definitions "${compile_definitions} -DFULL_FLOATING_POINT_REGISTER_PACK")
endif ()
if (DOBBY_DEBUG)
  set(compile_definitions "${compile_definitions} -DDOBBY_DEBUG")
else ()
  set(compile_definitions "${compile_definitions} -DDOBBY_LOGGING_DISABLE")
endif ()

include(cmake/compiler_and_linker.cmake)

# --- source files ---
set(DOBBY_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(dobby.SOURCE_FILE_LIST
  source/core/arch/CpuFeature.cc
  source/core/arch/CpuRegister.cc
  source/core/assembler/assembler.cc
  source/core/assembler/assembler-arm.cc
  source/core/assembler/assembler-arm64.cc
  source/core/assembler/assembler-ia32.cc
  source/core/assembler/assembler-x64.cc
  source/core/codegen/codegen-arm.cc
  source/core/codegen/codegen-arm64.cc
  source/core/codegen/codegen-ia32.cc
  source/core/codegen/codegen-x64.cc
  source/MemoryAllocator/CodeBuffer/CodeBufferBase.cc
  source/MemoryAllocator/AssemblyCodeBuilder.cc
  source/MemoryAllocator/MemoryAllocator.cc
  source/InstructionRelocation/arm/InstructionRelocationARM.cc
  source/InstructionRelocation/arm64/InstructionRelocationARM64.cc
  source/InstructionRelocation/x86/InstructionRelocationX86.cc
  source/InstructionRelocation/x86/InstructionRelocationX86Shared.cc
  source/InstructionRelocation/x64/InstructionRelocationX64.cc
  source/InstructionRelocation/x86/x86_insn_decode/x86_insn_decode.c
  source/InterceptRouting/InterceptRouting.cpp
  source/TrampolineBridge/Trampoline/arm/trampoline_arm.cc
  source/TrampolineBridge/Trampoline/arm64/trampoline_arm64.cc
  source/TrampolineBridge/Trampoline/x86/trampoline_x86.cc
  source/TrampolineBridge/Trampoline/x64/trampoline_x64.cc
  source/TrampolineBridge/ClosureTrampolineBridge/common_bridge_handler.cc
  source/TrampolineBridge/ClosureTrampolineBridge/arm/helper_arm.cc
  source/TrampolineBridge/ClosureTrampolineBridge/arm/closure_bridge_arm.cc
  source/TrampolineBridge/ClosureTrampolineBridge/arm/ClosureTrampolineARM.cc
  source/TrampolineBridge/ClosureTrampolineBridge/arm64/helper_arm64.cc
  source/TrampolineBridge/ClosureTrampolineBridge/arm64/closure_bridge_arm64.cc
  source/TrampolineBridge/ClosureTrampolineBridge/arm64/ClosureTrampolineARM64.cc
  source/TrampolineBridge/ClosureTrampolineBridge/x86/helper_x86.cc
  source/TrampolineBridge/ClosureTrampolineBridge/x86/closure_bridge_x86.cc
  source/TrampolineBridge/ClosureTrampolineBridge/x86/ClosureTrampolineX86.cc
  source/TrampolineBridge/ClosureTrampolineBridge/x64/helper_x64.cc
  source/TrampolineBridge/ClosureTrampolineBridge/x64/closure_bridge_x64.cc
  source/TrampolineBridge/ClosureTrampolineBridge/x64/ClosureTrampolineX64.cc
  source/InterceptRouting/Routing/InstructionInstrument/InstructionInstrument.cc
  source/InterceptRouting/Routing/InstructionInstrument/RoutingImpl.cc
  source/InterceptRouting/Routing/InstructionInstrument/instrument_routing_handler.cc
  source/InterceptRouting/Routing/FunctionInlineHook/FunctionInlineHook.cc
  source/InterceptRouting/Routing/FunctionInlineHook/RoutingImpl.cc
  source/InterceptRouting/RoutingPlugin/RoutingPlugin.cc
  source/Plugins/SymbolResolver/ElfSymbolResolver.cc
  source/Plugins/SymbolResolver/ImportTableResolver.cc
  source/Plugins/ApplicationEventMonitor/PosixFileMonitor.cc
  source/Plugins/ApplicationEventMonitor/PosixSocketMonitor.cc
  source/Plugins/ApplicationEventMonitor/DynamicLoaderMonitor.cc
  source/Plugins/PluginManager.cc
  source/Config/Config.cc
  source/dobby.cpp
  source/Interceptor.cpp
  source/InterceptEntry.cpp
  )

if (SYSTEM.Android OR SYSTEM.Linux)
  list(APPEND dobby.SOURCE_FILE_LIST
    source/PlatformUtil/ProcessRuntimeUtility.cc
    source/Backend/UserMode/UnifiedInterface/platform-posix.cc
    source/Backend/UserMode/ExecMemory/code-patch-tool-posix.cc
    source/Backend/UserMode/ExecMemory/clear-cache-tool-all.c
    )
endif ()

if (PROCESSOR.X86_64 OR PROCESSOR.X86)
  set(NearBranch ON)
endif ()

if (NearBranch)
  list(APPEND dobby.SOURCE_FILE_LIST
    source/InterceptRouting/RoutingPlugin/NearBranchTrampoline/near_trampoline_arm64.cc
    source/InterceptRouting/RoutingPlugin/NearBranchTrampoline/NearBranchTrampoline.cc
    source/MemoryAllocator/NearMemoryAllocator.cc)
endif ()

set(dobby.HEADER_FILE_LIST include/dobby.h)

# Build version
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
  execute_process(
    COMMAND git rev-parse --short --verify HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE VERSION_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if (VERSION_COMMIT_HASH)
    set(VERSION_REVISION "${VERSION_COMMIT_HASH}")
  endif ()
endif ()
set(DOBBY_BUILD_VERSION "Dobby${VERSION_REVISION}")
set(compile_definitions "${compile_definitions} -D__DOBBY_BUILD_VERSION__=\"${DOBBY_BUILD_VERSION}\"")

set(SOURCE_FILE_LIST ${dobby.HEADER_FILE_LIST} ${dobby.SOURCE_FILE_LIST})
get_absolute_path_list(SOURCE_FILE_LIST SOURCE_FILE_LIST_)
set(SOURCE_FILE_LIST ${SOURCE_FILE_LIST_})

# --- target: dobby (SHARED) ---
add_library(dobby SHARED ${SOURCE_FILE_LIST})
target_compile_features(dobby PRIVATE cxx_std_26)
target_link_libraries(dobby PRIVATE fmt::fmt spdlog::spdlog nlohmann_json::nlohmann_json tomlplusplus::tomlplusplus elfio::elfio)
target_include_directories(dobby PUBLIC
  $<BUILD_INTERFACE:${DOBBY_DIR}/include>
  $<BUILD_INTERFACE:${DOBBY_DIR}/source>
  $<BUILD_INTERFACE:${DOBBY_DIR}/source/dobby>
)
target_include_directories(dobby SYSTEM PUBLIC
  ${DOBBY_DIR}/vendor/fmt/include
  ${DOBBY_DIR}/vendor/spdlog/include
  ${DOBBY_DIR}/vendor/json/include
  ${DOBBY_DIR}/vendor/tomlplusplus/include
  ${DOBBY_DIR}/vendor/elfio
)
target_compile_definitions(dobby PUBLIC SPDLOG_NO_EXCEPTIONS SPDLOG_FMT_EXTERNAL FMT_USE_EXCEPTIONS=0)
target_compile_definitions(dobby PRIVATE "COMPILE_DEFINITIONS ${compile_definitions}")
if (SYSTEM.Android)
  target_compile_definitions(dobby PRIVATE DOBBY_CMAKE_ANDROID=1)
endif ()

# --- target: dobby_static (STATIC) ---
add_library(dobby_static STATIC ${SOURCE_FILE_LIST})
target_compile_features(dobby_static PRIVATE cxx_std_26)
target_link_libraries(dobby_static PRIVATE fmt::fmt spdlog::spdlog nlohmann_json::nlohmann_json tomlplusplus::tomlplusplus elfio::elfio)
target_include_directories(dobby_static PUBLIC
  $<BUILD_INTERFACE:${DOBBY_DIR}/include>
  $<BUILD_INTERFACE:${DOBBY_DIR}/source>
  $<BUILD_INTERFACE:${DOBBY_DIR}/source/dobby>
)
target_include_directories(dobby_static SYSTEM PUBLIC
  ${DOBBY_DIR}/vendor/fmt/include
  ${DOBBY_DIR}/vendor/spdlog/include
  ${DOBBY_DIR}/vendor/json/include
  ${DOBBY_DIR}/vendor/tomlplusplus/include
  ${DOBBY_DIR}/vendor/elfio
)
target_compile_definitions(dobby_static PUBLIC SPDLOG_NO_EXCEPTIONS SPDLOG_FMT_EXTERNAL FMT_USE_EXCEPTIONS=0)
target_compile_definitions(dobby_static PRIVATE "COMPILE_DEFINITIONS ${compile_definitions}")
if (SYSTEM.Android)
  target_compile_definitions(dobby_static PRIVATE DOBBY_CMAKE_ANDROID=1)
endif ()
set_target_properties(dobby_static PROPERTIES OUTPUT_NAME "dobby")

# Combine static libs
set(EXTRA_LIBS_TO_MERGE $<TARGET_FILE:fmt>)
add_custom_command(TARGET dobby_static POST_BUILD
  COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/scripts/combine_static_libs.py ${CMAKE_AR} $<TARGET_FILE:dobby_static> ${EXTRA_LIBS_TO_MERGE}
  COMMENT "Merging extra static libraries"
)

# Target-specific properties
find_library(ANDROID_LOG_LIB log)
if (SYSTEM.Android OR ANDROID_LOG_LIB)
  target_link_libraries(dobby PRIVATE ${ANDROID_LOG_LIB})
  if (PROCESSOR.ARM)
    set_target_properties(dobby PROPERTIES ANDROID_ARM_MODE arm)
    set_target_properties(dobby_static PROPERTIES ANDROID_ARM_MODE arm)
  endif ()
endif ()
if (SYSTEM.Linux)
  target_link_libraries(dobby PRIVATE dl)
endif ()

if (DOBBY_BUILD_EXAMPLE)
  add_subdirectory(examples)
endif ()
if (DOBBY_BUILD_TEST)
  enable_testing()
  add_subdirectory(tests)
endif ()
