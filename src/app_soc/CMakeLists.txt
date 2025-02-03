# Get the directory name
get_filename_component(DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

# Set the executable name
set(EXE_NAME ${DIR_NAME})

# Declare the executable
add_executable(${EXE_NAME} main.cc)

# Include directories for the target
target_include_directories(${EXE_NAME} PRIVATE include)

# Add subdirectories
add_subdirectory(libs)

# Link libraries to the executable
target_link_libraries(${EXE_NAME} ${PROJECT_NAME}::app_soc)
