set(SOURCE_FILE_LIST)

include_directories(
  .
)

if (NOT DEFINED DOBBY_DIR)
  message(FATAL_ERROR "DOBBY_DIR must be set!")
endif ()

if (SYSTEM.Linux OR SYSTEM.Android)
  set(SOURCE_FILE_LIST ${SOURCE_FILE_LIST}
    elf/dobby_symbol_resolver.cc
    ${DOBBY_DIR}/source/Plugins/SymbolResolver/ElfSymbolResolver.cc

    ${DOBBY_DIR}/source/Backend/UserMode/PlatformUtil/Android/ProcessRuntimeUtility.cc
    )
endif ()


get_absolute_path_list(SOURCE_FILE_LIST SOURCE_FILE_LIST_)
set(SOURCE_FILE_LIST ${SOURCE_FILE_LIST_})

add_library(dobby_symbol_resolver
  ${SOURCE_FILE_LIST}
  )

target_link_libraries(dobby_symbol_resolver PUBLIC logging fmt::fmt)



