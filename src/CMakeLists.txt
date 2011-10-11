find_package(FreeType REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Allegro5 REQUIRED)
find_package(GLEW REQUIRED)
set(render_includes ${OPENGL_INCLUDE_DIR} ${ALLEGRO_INCLUDE_DIR}
		${GLEW_INCLUDE_PATH} ${FREETYPE_INCLUDE_DIR})
set(render_libs ${OPENGL_LIBRARIES} ${ALLEGRO_LIBRARIES} ${GLEW_LIBRARY}
${FREETYPE_LIBRARIES})

include_directories(${render_includes})

set(mathlib_sources vector.c quaternion.c matrix.c)
set(render_sources render.c shader.c camera.c glm.c mesh.c input.c util.c
font.c stats.c)

add_library(MathLib STATIC ${mathlib_sources})
add_library(RenderLib STATIC ${render_sources})
target_link_libraries(MathLib m)
target_link_libraries(RenderLib ${render_libs} MathLib)

add_executable(teapot teapot.c log.c)
target_link_libraries(teapot RenderLib External)

add_executable(meshinfo meshinfo.c mesh.c log.c util.c)
target_link_libraries(meshinfo MathLib External)

add_executable(orrery orrery.c solarsystem.c keplerorbit.c log.c)
target_link_libraries(orrery RenderLib External)

add_executable(sol sol.c solarsystem.c keplerorbit.c log.c)
target_link_libraries(sol MathLib ${ALLEGRO_LIBRARIES} External)

add_executable(testfont testfont.c log.c)
target_link_libraries(testfont RenderLib External)
