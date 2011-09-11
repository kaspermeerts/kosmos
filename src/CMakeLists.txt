set(mathlib_sources matrix.c vector.c quaternion.c)
set(render_sources renderer_progr.c shader.c camera.c glm.c mesh.c)

find_package(OpenGL REQUIRED)
find_package(Allegro5 REQUIRED)
find_package(GLEW REQUIRED)
set(render_includes ${OPENGL_INCLUDE_DIR} ${ALLEGRO_INCLUDE_DIR} ${GLEW_INCLUDE_PATH})
set(render_libs ${OPENGL_LIBRARIES} ${ALLEGRO_LIBRARIES} ${GLEW_LIBRARY})

add_executable(teapot teapot.c ${render_sources} ${mathlib_sources}
		${external_sources})
include_directories(${render_includes})
target_link_libraries(teapot ${render_libs} m)

add_executable(meshinfo meshinfo.c mesh.c ${mathlib_sources} ${external_sources})
target_link_libraries(meshinfo m)
