
# Test configuration
option(DOBBY_BUILD_TESTS_WITH_UNICORN "Build relocation tests using Unicorn/Capstone" ON)

if (DOBBY_BUILD_TESTS_WITH_UNICORN)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(CAPSTONE capstone)
  pkg_check_modules(UNICORN unicorn)

  if (NOT CAPSTONE_FOUND OR NOT UNICORN_FOUND)
    if (CMAKE_CROSSCOMPILING)
      message(STATUS "Unicorn/Capstone not found for cross-compilation target. Disabling relocation tests.")
      set(DOBBY_BUILD_TESTS_WITH_UNICORN OFF)
    else ()
      message(FATAL_ERROR "Unicorn/Capstone required for relocation tests but not found.")
    endif ()
  endif ()
endif ()

include(CTest)
enable_testing()

get_property(DOBBY_SOURCE_FILE_LIST
  TARGET dobby
  PROPERTY SOURCES)

set(DOBBY_SOURCES)
foreach (path ${DOBBY_SOURCE_FILE_LIST})
  if (NOT path MATCHES "^/")
    list(APPEND DOBBY_SOURCES ${CMAKE_SOURCE_DIR}/${path})
  else ()
    list(APPEND DOBBY_SOURCES ${path})
  endif ()
endforeach ()

set(TEST_COMPILE_DEFINITIONS
  SPDLOG_NO_EXCEPTIONS
  SPDLOG_FMT_EXTERNAL
  FMT_USE_EXCEPTIONS=0
  LOGGING_DEBUG=1
  DISABLE_ARCH_DETECT=1
)

if (DOBBY_BUILD_TESTS_WITH_UNICORN)
  list(APPEND TEST_COMPILE_DEFINITIONS TEST_WITH_UNICORN=1)
endif ()

set(TEST_INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/source
  ${CMAKE_SOURCE_DIR}/source/dobby
  ${CMAKE_SOURCE_DIR}/vendor/fmt/include
  ${CMAKE_SOURCE_DIR}/vendor/spdlog/include
  ${CMAKE_SOURCE_DIR}/vendor/json/include
  ${CMAKE_SOURCE_DIR}/vendor/tomlplusplus/include
  ${CMAKE_SOURCE_DIR}/vendor/elfio
)

# Create a clean external include directory to avoid NDK conflicts with system headers
set(EXTERNAL_INC_DIR ${CMAKE_BINARY_DIR}/external_inc)
file(MAKE_DIRECTORY ${EXTERNAL_INC_DIR})

if (DOBBY_BUILD_TESTS_WITH_UNICORN)
  # Try to find headers via pkg-config paths, fallback to Termux defaults if on Termux
  if (CAPSTONE_INCLUDE_DIRS)
    execute_process(COMMAND ln -sf ${CAPSTONE_INCLUDE_DIRS} ${EXTERNAL_INC_DIR}/capstone_inc)
  elseif (EXISTS "/data/data/com.termux/files/usr/include/capstone")
    execute_process(COMMAND ln -sf /data/data/com.termux/files/usr/include/capstone ${EXTERNAL_INC_DIR}/capstone_inc)
  endif ()

  if (UNICORN_INCLUDE_DIRS)
    execute_process(COMMAND ln -sf ${UNICORN_INCLUDE_DIRS} ${EXTERNAL_INC_DIR}/unicorn)
  elseif (EXISTS "/data/data/com.termux/files/usr/include/unicorn")
    execute_process(COMMAND ln -sf /data/data/com.termux/files/usr/include/unicorn ${EXTERNAL_INC_DIR}/unicorn)
  endif ()
endif ()

macro(setup_test_target name arch_define)
  if (DOBBY_BUILD_TESTS_WITH_UNICORN)
    add_executable(${name}
      ${name}.cpp
      UniconEmulator.cpp
      ${DOBBY_SOURCES}
      )

    target_compile_definitions(${name} PUBLIC
      ${TEST_COMPILE_DEFINITIONS}
      ${arch_define}=1
      )

    target_include_directories(${name} PUBLIC
      ${TEST_INCLUDE_DIRS}
      ${EXTERNAL_INC_DIR}
    )

    if (CAPSTONE_INCLUDE_DIRS)
      target_include_directories(${name} PUBLIC ${CAPSTONE_INCLUDE_DIRS})
    elseif (EXISTS "${EXTERNAL_INC_DIR}/capstone_inc")
      target_include_directories(${name} PUBLIC "${EXTERNAL_INC_DIR}/capstone_inc")
    endif ()

    if (UNICORN_INCLUDE_DIRS)
      target_include_directories(${name} PUBLIC ${UNICORN_INCLUDE_DIRS})
    elseif (EXISTS "${EXTERNAL_INC_DIR}/unicorn")
      target_include_directories(${name} PUBLIC "${EXTERNAL_INC_DIR}")
    endif ()
    target_link_directories(${name} PUBLIC
      ${CAPSTONE_LIBRARY_DIRS}
      ${UNICORN_LIBRARY_DIRS}
      )

    target_link_libraries(${name} PUBLIC
      fmt::fmt
      spdlog::spdlog
      nlohmann_json::nlohmann_json
      tomlplusplus::tomlplusplus
      elfio::elfio
      ${CAPSTONE_LIBRARIES}
      ${UNICORN_LIBRARIES}
      )

    target_compile_options(${name} PRIVATE -fblocks)
  endif ()
endmacro()

set(DOBBY_TEST_TARGET_ARM64 OFF)
set(DOBBY_TEST_TARGET_ARM OFF)
set(DOBBY_TEST_TARGET_X64 OFF)

if (SYSTEM.Android)
  if (ANDROID_ABI STREQUAL "arm64-v8a")
    set(DOBBY_TEST_TARGET_ARM64 ON)
  elseif (ANDROID_ABI STREQUAL "armeabi-v7a")
    set(DOBBY_TEST_TARGET_ARM ON)
  elseif (ANDROID_ABI STREQUAL "x86_64")
    set(DOBBY_TEST_TARGET_X64 ON)
  endif ()
else ()
  if (PROCESSOR.AARCH64)
    set(DOBBY_TEST_TARGET_ARM64 ON)
  endif ()
  if (PROCESSOR.ARM)
    set(DOBBY_TEST_TARGET_ARM ON)
  endif ()
  if (PROCESSOR.X86_64)
    set(DOBBY_TEST_TARGET_X64 ON)
  endif ()
endif ()

if (DOBBY_TEST_TARGET_ARM64)
  setup_test_target(test_insn_relo_arm64 TARGET_ARCH_ARM64)
endif ()
if (DOBBY_TEST_TARGET_ARM)
  setup_test_target(test_insn_relo_arm TARGET_ARCH_ARM)
endif ()
if (DOBBY_TEST_TARGET_X64)
  setup_test_target(test_insn_relo_x64 TARGET_ARCH_X64)
  setup_test_target(test_insn_decoder_x86 TARGET_ARCH_X64)
endif ()

# ---

add_executable(test_native
  test_native.cpp)

target_link_libraries(test_native
  dobby)

# ---

add_executable(test_vtable
  test_vtable.cpp)

target_compile_features(test_vtable PRIVATE cxx_std_26)

target_link_libraries(test_vtable
  dobby)

if (BUILD_TESTING)
  add_test(NAME test_native COMMAND test_native)
  add_test(NAME test_vtable COMMAND test_vtable)
  if (PROCESSOR.AARCH64)
    add_test(NAME test_insn_relo_arm64 COMMAND test_insn_relo_arm64)
  endif ()
  if (PROCESSOR.ARM)
    add_test(NAME test_insn_relo_arm COMMAND test_insn_relo_arm)
  endif ()
  if (PROCESSOR.X86_64)
    add_test(NAME test_insn_relo_x64 COMMAND test_insn_relo_x64)
    add_test(NAME test_insn_decoder_x86 COMMAND test_insn_decoder_x86)
  endif ()
endif ()
