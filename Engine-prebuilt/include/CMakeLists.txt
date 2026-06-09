cmake_minimum_required(VERSION 3.17)
project(Engine LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(Engine STATIC
    Application.cpp
    "Platform/Window.cpp"
    Rendering/Essentials/Shader.cpp
    Rendering/Essentials/ShaderLoader.cpp
    Rendering/Essentials/Texture.cpp
    Rendering/Essentials/TextureLoader.cpp
    "Rendering/Core/SpriteBatchRenderer.cpp"
    Rendering/Core/Camera2D.cpp
    Logger/Logger.cpp
    "Platform/Input.cpp"
    "Rendering/Essentials/FontLoader.h"
    "Rendering/Essentials/FontLoader.cpp"
    "Rendering/Core/TextRenderer.h"
    "Rendering/Core/TextRenderer.cpp"
    "Rendering/Core/DebugRenderer.h"
    "Rendering/Core/DebugRenderer.cpp"
)

target_include_directories(Engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Engine PUBLIC sol2 lua glfw glad stb_image freetype)

# ---------------------------------------------------------------
# Copy Engine lib + headers and all third-party artifacts into
# Asteroids/ so it can be built standalone
# ---------------------------------------------------------------
set(ASTEROIDS_PREBUILT_DIR   "${CMAKE_SOURCE_DIR}/Asteroids/Engine-prebuilt")
set(ASTEROIDS_THIRDPARTY_DIR "${CMAKE_SOURCE_DIR}/Asteroids/third_party")

add_custom_command(TARGET Engine POST_BUILD
    COMMENT "Copying Engine and third-party libs into Asteroids prebuilt folders..."

    # --- Engine lib ---
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ASTEROIDS_PREBUILT_DIR}/lib"
    COMMAND ${CMAKE_COMMAND} -E copy
        "$<TARGET_FILE:Engine>"
        "${ASTEROIDS_PREBUILT_DIR}/lib/"

    # --- Engine headers ---
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${ASTEROIDS_PREBUILT_DIR}/include"

    # --- Third-party libs ---
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ASTEROIDS_THIRDPARTY_DIR}/lib"
    COMMAND ${CMAKE_COMMAND} -E copy
        "$<TARGET_FILE:glfw>"
        "${ASTEROIDS_THIRDPARTY_DIR}/lib/"
    COMMAND ${CMAKE_COMMAND} -E copy
        "$<TARGET_FILE:glad>"
        "${ASTEROIDS_THIRDPARTY_DIR}/lib/"
    COMMAND ${CMAKE_COMMAND} -E copy
        "$<TARGET_FILE:freetype>"
        "${ASTEROIDS_THIRDPARTY_DIR}/lib/"

    # --- Third-party headers ---
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/dependencies/glfw/include"
        "${ASTEROIDS_THIRDPARTY_DIR}/include/GLFW"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/dependencies/glad/include"
        "${ASTEROIDS_THIRDPARTY_DIR}/include"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/dependencies/freetype/include"
        "${ASTEROIDS_THIRDPARTY_DIR}/include"
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_SOURCE_DIR}/dependencies/stb_image/stb_image.h"
        "${ASTEROIDS_THIRDPARTY_DIR}/include/"
)