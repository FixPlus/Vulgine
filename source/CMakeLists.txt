add_subdirectory(vulkan)

add_library(VulgineCore OBJECT Vulgine.cpp Vulgine.h Utilities.cpp Utilities.h ../include/IVulgine.h)
set_property(TARGET VulgineCore PROPERTY CXX_STANDARD 20)
