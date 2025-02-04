# ##########################################################################
# Configure
# ##########################################################################
# Include utility functions
include(${PROJECT_SOURCE_DIR}/cmake/Utility.cmake)

realpath(APP_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../include)

# ##########################################################################
# Build Configure
# ##########################################################################
set(APP_LIB_NAME app_soc)

set(APP_PREFIX_LIB_NAME ${PROJECT_NAME}_${APP_LIB_NAME})

# ##########################################################################
# # Build Source Files
# ##########################################################################
file(
    GLOB_RECURSE LIBS_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/packet/*.cc
)

# ##########################################################################
# # Build rules : Function to configure target properties and dependencies
# ##########################################################################
add_library(${APP_PREFIX_LIB_NAME} ${LIBS_SRCS})

set_target_properties(${LIBS_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

set_property(TARGET ${LIBS_NAME} PROPERTY CXX_STANDARD 20)

target_link_libraries(
    ${APP_PREFIX_LIB_NAME}
    PUBLIC Threads::Threads
    PUBLIC ACALSim::acalsim
    PUBLIC ${PROJECT_NAME}::${PROJECT_NAME}
)

add_target_compile_definitions(${APP_PREFIX_LIB_NAME}
    ACALSIM_VERBOSE
    MT_DEBUG
    NO_LOGS
    ACALSIM_STATISTICS
)

target_include_directories(${APP_PREFIX_LIB_NAME} PUBLIC ${APP_INC_DIR})

add_library(${PROJECT_NAME}::${APP_LIB_NAME} ALIAS ${APP_PREFIX_LIB_NAME})
