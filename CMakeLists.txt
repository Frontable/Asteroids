cmake_minimum_required(VERSION 3.20)
project(Asteroids LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE ASTEROIDS_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_executable(Asteroids ${ASTEROIDS_SOURCES})

target_include_directories(Asteroids
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../Frost/src  
        ${CMAKE_CURRENT_SOURCE_DIR}/Game           
)

target_link_libraries(Asteroids
    PRIVATE
        Frost
)

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
    COMMENT "Copying assets to build directory"
)


#[[
cmake_minimum_required(VERSION 3.20)
project(Asteroids LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE ASTEROIDS_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_executable(Asteroids ${ASTEROIDS_SOURCES} "Game/AsteroidsApp.h" "Game/Components/GameComponents.h" "Game/SpriteSheet.h" "Game/Events/GameEvents.h" "Game/Systems/GameSystems.h" "Game/Systems/CollisionSystem.h" "Game/Systems/RenderSystem.h" "Game/Scripts/PlayerScript.h" "Game/Scripts/ShootingScript.h" "Game/Systems/DamageSystem.h" "Game/Systems/AsteroidSpawner.h" "Game/Scenes/AsteroidsScene.h")

target_link_libraries(Asteroids
    PRIVATE
        Frost
)

target_include_directories(Asteroids
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../Frost/src
        ${CMAKE_CURRENT_SOURCE_DIR}/Game    # ← add this
)
#]]

#[[
# Copy assets to build directory
add_custom_command(TARGET Asteroids POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Asteroids/Assets"
        "$<TARGET_FILE_DIR:Asteroids>/Assets"
)
]]#