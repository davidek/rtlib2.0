# Include Environment-based settings.
include(${CMAKE_CURRENT_DOURCE_DIR}../common_settings.txt)

# Create the executable.
add_executable(srp srp.cpp)

# Indicate that the executable needs metasim and rtlib library.
target_link_libraries( srp rtlib ${metasim_LIBRARY} )
