set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(OpenGL_GL_PREFERENCE "GLVND")

find_package(OpenGL REQUIRED)
find_package(Freetype REQUIRED)
find_package(glm REQUIRED)

include_directories(${GLEW_INCLUDE_DIRS})
link_libraries(${GLEW_LIBRARIES})

find_program(glslangv_executable NAMES /usr/bin/glslangValidator)

set(RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)
set(RESOURCES_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/resources)

file(GLOB SHADERS 
    ${RESOURCES_DIR}/*.vert
    ${RESOURCES_DIR}/*.frag
    ${RESOURCES_DIR}/*.comp
)

add_custom_command(
    COMMAND
        ${CMAKE_COMMAND} -E make_directory ${RESOURCES_BINARY_DIR}
    OUTPUT ${RESOURCES_BINARY_DIR}
    COMMENT "Creating ${RESOURCES_BINARY_DIR}"
)

foreach(source IN LISTS SHADERS)
    get_filename_component(FILENAME ${source} NAME)
    add_custom_command(
        COMMAND
            ${glslangv_executable}
            -V -G
            -o ${RESOURCES_BINARY_DIR}/${FILENAME}.spv
            ${source}
        OUTPUT ${RESOURCES_BINARY_DIR}/${FILENAME}.spv
        DEPENDS ${source} ${RESOURCES_BINARY_DIR}
        COMMENT "Compiling ${FILENAME}"
    )
    list(APPEND SPV_SHADERS ${RESOURCES_BINARY_DIR}/${FILENAME}.spv)
endforeach()

configure_file(${RESOURCES_DIR}/init.png ${RESOURCES_BINARY_DIR}/init.png COPYONLY)
configure_file(${RESOURCES_DIR}/arial.ttf ${RESOURCES_BINARY_DIR}/arial.ttf COPYONLY)

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS} ${SHADER_RES})
add_executable(${PROJECT_NAME} main.cpp font_renderer.cpp shader_util.hpp imgui_util.hpp)
target_link_libraries(${PROJECT_NAME} ${GLEW_LIBRARIES} glfw OpenGL::GL freetype glm::glm imgui)
target_include_directories(${PROJECT_NAME}
    PUBLIC ${FREETYPE_INCLUDE_DIRS})
