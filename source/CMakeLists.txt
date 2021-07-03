add_library(base OBJECT Vulgine.cpp Vulgine.h Utilities.cpp Utilities.h ../include/IVulgine.h vulkan/VulkanDebug.h vulkan/VulkanDebug.cpp)
set_property(TARGET base PROPERTY CXX_STANDARD 20)