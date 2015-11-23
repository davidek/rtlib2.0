# Include Environment-based settings.
include(${CMAKE_CURRENT_DOURCE_DIR}../common_settings.txt)

# Create the executable.
add_executable(apa apa.cpp)

# Indicate that the executable needs metasim and rtlib library.
target_link_libraries( apa rtlib ${metasim_LIBRARY} )
