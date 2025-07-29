cmake_minimum_required(VERSION 3.28)
project(LifePathGame)

# Create game executable
add_executable(${PROJECT_NAME}
        main.cc
)

# Set resource directory to where the game binary will be
set(GAME_BINARY_DIR $<TARGET_FILE_DIR:${PROJECT_NAME}>)
set(RESOURCE_DIR ${GAME_BINARY_DIR}/resources)

# Copy game resources
set(GAME_RESOURCE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/resources)
if(EXISTS ${GAME_RESOURCE_SOURCE_DIR})
    file(GLOB_RECURSE GAME_RESOURCE_FILES
            LIST_DIRECTORIES false
            RELATIVE "${GAME_RESOURCE_SOURCE_DIR}"
            CONFIGURE_DEPENDS
            "${GAME_RESOURCE_SOURCE_DIR}/*")

    message(STATUS "Found game resource files to copy")

    # Add copy commands for game resources
    foreach (RESOURCE_FILE ${GAME_RESOURCE_FILES})
        get_filename_component(RESOURCE_DIR_PATH "${RESOURCE_FILE}" DIRECTORY)

        add_custom_command(
                TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "${RESOURCE_DIR}/${RESOURCE_DIR_PATH}"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${GAME_RESOURCE_SOURCE_DIR}/${RESOURCE_FILE}"
                "${RESOURCE_DIR}/${RESOURCE_FILE}"
                COMMENT "Copying game resource: ${RESOURCE_FILE}..."
                VERBATIM
        )
    endforeach ()
else()
    message(STATUS "Game resource directory not found: ${GAME_RESOURCE_SOURCE_DIR}")
endif()

# Copy engine resources to the same location
set(ENGINE_RESOURCE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/TomosEngine/src/Tomos/resources")
if(EXISTS ${ENGINE_RESOURCE_SOURCE_DIR})
    file(GLOB_RECURSE ENGINE_RESOURCE_FILES
            LIST_DIRECTORIES false
            RELATIVE "${ENGINE_RESOURCE_SOURCE_DIR}"
            CONFIGURE_DEPENDS
            "${ENGINE_RESOURCE_SOURCE_DIR}/*")

    message(STATUS "Found engine resource files to copy")

    # Add copy commands for engine resources
    foreach (RESOURCE_FILE ${ENGINE_RESOURCE_FILES})
        get_filename_component(RESOURCE_DIR_PATH "${RESOURCE_FILE}" DIRECTORY)

        add_custom_command(
                TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "${RESOURCE_DIR}/${RESOURCE_DIR_PATH}"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${ENGINE_RESOURCE_SOURCE_DIR}/${RESOURCE_FILE}"
                "${RESOURCE_DIR}/${RESOURCE_FILE}"
                COMMENT "Copying engine resource: ${RESOURCE_FILE}..."
                VERBATIM
        )
    endforeach ()
else()
    message(STATUS "Engine resource directory not found: ${ENGINE_RESOURCE_SOURCE_DIR}")
endif()

# Link the engine library
target_link_libraries(${PROJECT_NAME} PRIVATE Tomos)

# Include engine headers
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/TomosEngine/src)