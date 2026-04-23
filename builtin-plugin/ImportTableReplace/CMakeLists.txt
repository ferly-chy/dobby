add_library(dobby_import_replace STATIC
  dobby_import_replace.cc
  )

target_link_libraries(dobby_import_replace PUBLIC logging fmt::fmt)