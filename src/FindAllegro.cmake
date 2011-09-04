# - Find allegro
# Find the native ALLEGRO includes and library
#
#  ALLEGRO_INCLUDE_DIR - where to find allegro.h, etc.
#  ALLEGRO_LIBRARIES   - List of libraries when using allegro.
#  ALLEGRO_FOUND       - True if allegro found.

set(ALLEGRO_FIND_QUIETLY FALSE)

if (ALLEGRO_INCLUDE_DIR)
  # Already in cache, be silent
  set(ALLEGRO_FIND_QUIETLY FALSE)
endif (ALLEGRO_INCLUDE_DIR)

find_path(ALLEGRO_INCLUDE_DIR allegro.h
  /usr/local/include
  /usr/include
  $ENV{MINGDIR}/include
)

if(UNIX AND NOT CYGWIN)
	exec_program(allegro-config ARGS --libs OUTPUT_VARIABLE ALLEGRO_LIBRARY)
else(UNIX AND NOT CYGWIN)
	set(ALLEGRO_NAMES alleg alleglib alleg41 alleg42 allegdll)
	FIND_LIBRARY(ALLEGRO_LIBRARY
	NAMES ${ALLEGRO_NAMES}
	PATHS /usr/lib /usr/local/lib $ENV{MINGDIR}/lib)
endif(UNIX AND NOT CYGWIN)

if (ALLEGRO_INCLUDE_DIR AND ALLEGRO_LIBRARY)
   set(ALLEGRO_FOUND TRUE)
    set( ALLEGRO_LIBRARIES ${ALLEGRO_LIBRARY} )
else (ALLEGRO_INCLUDE_DIR AND ALLEGRO_LIBRARY)
   set(ALLEGRO_FOUND FALSE)
   set( ALLEGRO_LIBRARIES )
endif (ALLEGRO_INCLUDE_DIR AND ALLEGRO_LIBRARY)

if (ALLEGRO_FOUND)
   if (not ALLEGRO_FIND_QUIETLY)
      message(STATUS "Found Allegro: ${ALLEGRO_LIBRARY}")
   endif (not ALLEGRO_FIND_QUIETLY)
else (ALLEGRO_FOUND)
   if (ALLEGRO_FIND_REQUIRED)
      message(STATUS "Looked for Allegro libraries named ${ALLEGRO_NAMES}.")
      message(FATAL_ERROR "Could not find Allegro library")
   endif (ALLEGRO_FIND_REQUIRED)
endif (ALLEGRO_FOUND)

mark_as_advanced(
  ALLEGRO_LIBRARY
  ALLEGRO_INCLUDE_DIR
  )
