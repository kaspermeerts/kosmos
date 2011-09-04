set(render_sources matrix.c vector.c quaternion.c camera.c glm.c )

find_package(OpenGL REQUIRED)
find_package(Allegro5 REQUIRED)
find_package(GLEW REQUIRED)
set(render_includes ${render_includes} ${OPENGL_INCLUDE_DIR}
${ALLEGRO_INCLUDE_DIR} ${GLEW_INCLUDE_PATH})
set(render_libs ${render_libs} ${OPENGL_LIBRARIES} ${ALLEGRO_LIBRARIES}
${GLEW_LIBRARY})

add_executable(teapot teapot.c ${render_sources})
include_directories(${render_includes})
target_link_libraries(teapot ${render_libs})

