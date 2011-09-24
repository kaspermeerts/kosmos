set(mathlib_sources vector.c quaternion.c matrix.c)
set(render_sources render.c shader.c camera.c glm.c mesh.c)

find_package(OpenGL REQUIRED)
find_package(Allegro5 REQUIRED)
find_package(GLEW REQUIRED)
set(render_includes ${OPENGL_INCLUDE_DIR} ${ALLEGRO_INCLUDE_DIR}
		${GLEW_INCLUDE_PATH})
set(render_libs ${OPENGL_LIBRARIES} ${ALLEGRO_LIBRARIES} ${GLEW_LIBRARY})

include_directories(${render_includes})

add_library(MyMath STATIC ${mathlib_sources})
add_library(MyRender STATIC ${render_sources} ${mathlib_sources})
target_link_libraries(MyMath m)
target_link_libraries(MyRender ${render_libs} MyMath)

add_executable(teapot teapot.c log.c)
target_link_libraries(teapot MyRender External)

add_executable(meshinfo meshinfo.c mesh.c log.c)
target_link_libraries(meshinfo allegro MyMath External)

add_executable(orrery orrery.c solarsystem.c keplerorbit.c log.c)
target_link_libraries(orrery MyRender External)
