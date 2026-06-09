cmake_minimum_required(VERSION 3.17)
project(Asteroids LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------
# Dual-mode build:
#   - If built as part of FrostEngine, Engine target already exists
#   - If built standalone, use prebuilt libs in Engine-prebuilt/ and third_party/
# ---------------------------------------------------------------
if(NOT TARGET Engine)
    message(STATUS "[Asteroids] Standalone build — using prebuilt Engine and third-party libs")

    set(PREBUILT_DIR   "${CMAKE_CURRENT_SOURCE_DIR}/Engine-prebuilt")
    set(THIRDPARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party")

    # Validate that prebuilt files exist
    if(NOT EXISTS "${PREBUILT_DIR}/lib/Engine.lib")
        message(FATAL_ERROR
            "Engine-prebuilt/lib/Engine.lib not found.\n"
            "Build FrostEngine first to populate the prebuilt folder, "
            "or copy the files manually into Asteroids/Engine-prebuilt/")
    endif()

    # Import Engine as a prebuilt static lib
    add_library(Engine STATIC IMPORTED)
    set_target_properties(Engine PROPERTIES
        IMPORTED_LOCATION             "${PREBUILT_DIR}/lib/Engine.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${PREBUILT_DIR}/include"
    )

    # Import glfw
    add_library(glfw STATIC IMPORTED)
    set_target_properties(glfw PROPERTIES
        IMPORTED_LOCATION             "${THIRDPARTY_DIR}/lib/glfw3.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${THIRDPARTY_DIR}/include/GLFW"
    )

    # Import glad
    add_library(glad STATIC IMPORTED)
    set_target_properties(glad PROPERTIES
        IMPORTED_LOCATION             "${THIRDPARTY_DIR}/lib/glad.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${THIRDPARTY_DIR}/include"
    )

    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")

    # Import freetype — needs to be linked by Asteroids directly
    add_library(freetype STATIC IMPORTED)
    set_target_properties(freetype PROPERTIES
    IMPORTED_LOCATION "${THIRDPARTY_DIR}/lib/freetype.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${THIRDPARTY_DIR}/include"
)

    # stb_image (header only)
    add_library(stb_image INTERFACE IMPORTED)
    set_target_properties(stb_image PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${THIRDPARTY_DIR}/include"
    )

else()
    message(STATUS "[Asteroids] FrostEngine build — using live Engine target")
endif()

# ---------------------------------------------------------------
# Asteroids executable
# ---------------------------------------------------------------
add_executable(Asteroids
    main.cpp
    src/Editor.cpp
    src/GameState.h
    "src/Systems/PlayerSystems/PlayerSystem.cpp"
    "src/Systems/UtilitySystems/LifetimeSystem.cpp"
    "src/Systems/PhysicsSystems/MovementSystem.cpp"
    "src/Systems/PlayerSystems/ShootingSystem.cpp"
    "src/Systems/AsteroidsSystems/AsteroidSpawnerSystem.cpp"
    "src/Systems/PhysicsSystems/CollisionSystem.h"
    "src/Systems/PhysicsSystems/CollisionSystem.cpp"
    "src/Systems/AsteroidsSystems/DamageSystem.h"
    "src/Systems/AsteroidsSystems/DamageSystem.cpp"
    "src/Systems/UtilitySystems/ScoreSystem.h"
    "src/Systems/UtilitySystems/SpawnImmunitySystem.h"
    "src/Systems/UtilitySystems/SpawnImmunitySystem.cpp"
    "src/Systems/RenderSystem.h"
    "src/Systems/RenderSystem.cpp"
    "src/Systems/PhysicsSystems/ScreenWrapSystem.h"
    "src/Systems/PhysicsSystems/ScreenWrapSystem.cpp"
    "src/SpriteSheet.h"
    "src/Systems/UtilitySystems/FlickerSystem.h"
    "src/Systems/UtilitySystems/FlickerSystem.cpp"
)

target_include_directories(Asteroids PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Asteroids PRIVATE
    Engine
    glfw
    glad
    stb_image
    freetype
    opengl32      # system OpenGL — always available on Windows
)

# --- Copy assets next to the executable ---
add_custom_command(TARGET Asteroids POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Assets"
        "$<TARGET_FILE_DIR:Asteroids>/Assets"
)