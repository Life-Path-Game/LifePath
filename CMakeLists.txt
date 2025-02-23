cmake_minimum_required(VERSION 3.28)

project(LifePathGame)

# Gather all resources
set(GAME_RESOURCE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/resource)
set(GAME_RESOURCE_DEST_DIR ${CMAKE_BINARY_DIR}/resource)

set(FINAL_RESOURCE_SOURCE_DIR ${CMAKE_BINARY_DIR}/resource)
set(FINAL_RESOURCE_DEST_DIR ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}/resource)

# Create game executable
add_executable(${PROJECT_NAME}
        main.cc
)

# Copy game resources
if(EXISTS ${GAME_RESOURCE_SOURCE_DIR})
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${GAME_RESOURCE_DEST_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${GAME_RESOURCE_SOURCE_DIR} ${GAME_RESOURCE_DEST_DIR}
        COMMENT "Copying shaders to output directory..."
    )
endif()

# Copy all resources to the output directory
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${FINAL_RESOURCE_DEST_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${FINAL_RESOURCE_SOURCE_DIR} ${FINAL_RESOURCE_DEST_DIR}
    COMMENT "Copying shaders to output directory..."
)

# Link the engine library
target_link_libraries(${PROJECT_NAME} PRIVATE Tomos)

# Include engine headers
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/Tomos)
