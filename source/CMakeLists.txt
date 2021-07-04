add_subdirectory(vulkan)

add_library(VulgineCore OBJECT Vulgine.cpp Vulgine.h Utilities.cpp Utilities.h ../include/IVulgine.h VulgineScene.cpp VulgineScene.h ../include/IVulgineScene.h ../include/IVulgineObjects.h VulgineObjects.cpp VulgineObjects.h VulgineRenderPass.cpp VulgineRenderPass.h VulgineFramebuffer.cpp VulgineFramebuffer.h)
set_property(TARGET VulgineCore PROPERTY CXX_STANDARD 20)
