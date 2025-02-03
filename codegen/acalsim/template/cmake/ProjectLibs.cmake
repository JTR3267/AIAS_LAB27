# ##########################################################################
# Configure
# ##########################################################################
# Include utility functions
include(${PROJECT_SOURCE_DIR}/cmake/Utility.cmake)

# ##########################################################################
# Build Configure
# ##########################################################################
set(LIB_NAME ${PROJECT_NAME})

# ##########################################################################
# # Build Source Files
# ##########################################################################
file(GLOB_RECURSE LIBS_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

# ##########################################################################
# # Build rules : Function to configure target properties and dependencies
# ##########################################################################
add_library(${LIB_NAME} ${LIBS_SRCS})

set_target_properties(
    ${LIB_NAME}
    PROPERTIES POSITION_INDEPENDENT_CODE ON
)

# Set C++ version to C++20
set_property(TARGET ${LIB_NAME} PROPERTY CXX_STANDARD 20)

# Set and link the libraries.
target_link_libraries(
    ${LIB_NAME}
    PRIVATE Threads::Threads
    PUBLIC ACALSim::acalsim
)

# Set the libraries include path
target_include_directories(
    ${LIB_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

# ##########################################################################
# # Compile Definitions
# ##########################################################################
# Add compile definitions based on options
add_target_compile_definitions(${LIB_NAME}
    ACALSIM_VERBOSE
    MT_DEBUG
    NO_LOGS
    NVSIM_VERBOSE
)

# ##########################################################################
# # Alias Library Name
# ##########################################################################
# Create an alias library target
add_library(${PROJECT_NAME}::${LIB_NAME} ALIAS ${LIB_NAME})
