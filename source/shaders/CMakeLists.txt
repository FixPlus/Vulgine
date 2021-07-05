find_program(GLSL NAMES glslc.exe glslc PATHS $ENV{VK_SDK_PATH}/Bin)

if(GLSL-NOTFOUND)
    message("GLSL compiler not found - shader compilation skipped")
ELSE()
    message("Found following GLSL compiler:")
    message(STATUS ${GLSL})

function(compileShader SHADER_NAME)

    add_custom_target(${SHADER_NAME} ALL COMMAND ${GLSL}  ${SHADER_DIR}/${SHADER_NAME} -o ${VULGINE_TOP_BINARY_DIR}/${SHADER_NAME}.spv)


endfunction(compileShader)

function(compileShaders)
    foreach(SHADER ${SHADERS})
        compileShader(${SHADER})
    endforeach(SHADER)
endfunction(compileShaders)

set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB SHADERS RELATIVE ${SHADER_DIR}  *.vert *.frag)
compileShaders()

list(TRANSFORM SHADERS APPEND ".spv" OUTPUT_VARIABLE SHADERS_BINS)
list(TRANSFORM SHADERS_BINS PREPEND ${VULGINE_TOP_BINARY_DIR}/)
install(FILES ${SHADERS_BINS} DESTINATION ${CMAKE_INSTALL_PREFIX})
add_dependencies(test ${SHADERS})
ENDIF()