file(GLOB rply_sources rply/*.c)
file(GLOB ralloc_sources ralloc/*.c)

set(external_sources ${rply_sources} ${ralloc_sources} PARENT_SCOPE)
