file(GLOB rply_sources rply/*.c)
file(GLOB ralloc_sources ralloc/*.c)

add_library(External STATIC ${ralloc_sources} ${rply_sources})
