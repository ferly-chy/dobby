add_library(osbase STATIC
  ${PROJECT_SOURCE_DIR}/source/Backend/UserMode/UnifiedInterface/platform-posix.cc
  )

target_link_libraries(osbase PUBLIC logging fmt::fmt)