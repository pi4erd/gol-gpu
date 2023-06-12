project(imgui)

add_library(imgui 
    imgui.cpp imgui_impl_opengl3.cpp 
    imgui_impl_glfw.cpp imgui_widgets.cpp
    imgui_tables.cpp imgui_draw.cpp
)

message("${CMAKE_INSTALL_LIBDIR}")

install(TARGETS imgui
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)
