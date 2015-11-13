cmake_minimum_required (VERSION 2.6) 
project (OPENGL_DEMO)

include_directories("${PROJECT_SOURCE_DIR}/ThirdParty")
include_directories("${PROJECT_SOURCE_DIR}/ThirdParty/SOIL/include")
link_directories("${PROJECT_SOURCE_DIR}/ThirdParty/SOIL/lib")

add_subdirectory("ThirdParty/glfw")
include_directories("ThirdParty/glfw/include")

set (SRC
	main.cpp
	glew.c
	)

add_executable (OpenGLDemo ${SRC})
target_link_libraries(OpenGLDemo SOIL glfw ${GLFW_LIBRARIES})