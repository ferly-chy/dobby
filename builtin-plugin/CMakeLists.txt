if (Plugin.ImportTableReplace AND SYSTEM.Darwin)
  message(STATUS "[Dobby] Enable got hook")
  include_directories(builtin-plugin/ImportTableReplace)
  add_subdirectory(builtin-plugin/ImportTableReplace)
endif ()

if (Plugin.Android.BionicLinkerUtil)
  if (NOT SYSTEM.Android)
    message(FATAL_ERROR "[!] Plugin.Android.BionicLinkerUtil only works on Android.")
  endif ()
  message(STATUS "[Dobby] Enable Plugin.Android.BionicLinkerUtil")
  set(dobby.plugin.SOURCE_FILE_LIST ${dobby.plugin.SOURCE_FILE_LIST}
    BionicLinkerUtil/bionic_linker_util.cc
    )
endif ()

# KittyMemory Integration
message(STATUS "[Dobby] Enable KittyMemory")
include_directories(builtin-plugin/KittyMemory)
include_directories(builtin-plugin/KittyMemory/Deps/Keystone/includes)
set(dobby.plugin.SOURCE_FILE_LIST ${dobby.plugin.SOURCE_FILE_LIST}
  KittyMemory/KittyAsm.cpp
  KittyMemory/KittyIOFile.cpp
  KittyMemory/KittyMemory.cpp
  KittyMemory/KittyPtrValidator.cpp
  KittyMemory/KittyScanner.cpp
  KittyMemory/KittyUtils.cpp
  KittyMemory/MemoryBackup.cpp
  KittyMemory/MemoryPatch.cpp
)