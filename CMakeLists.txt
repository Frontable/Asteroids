cmake_minimum_required(VERSION 3.20)
project(Asteroids LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ======================================================
# FrostEngine — from submodule
# ======================================================
add_subdirectory(third_party/Frost)

# ======================================================
# Game sources
# ======================================================
file(GLOB_RECURSE ASTEROIDS_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

add_executable(Asteroids ${ASTEROIDS_SOURCES} "src/Systems/PowerUpSystem.h"   )

target_include_directories(Asteroids
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/Frost/src
)

target_link_libraries(Asteroids PRIVATE Frost)

if(WIN32)
    target_compile_definitions(Asteroids PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
endif()

add_custom_command(TARGET Asteroids POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Assets"
        "$<TARGET_FILE_DIR:Asteroids>/Assets"
)