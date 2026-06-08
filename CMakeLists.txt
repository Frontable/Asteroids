

project(Asteroids)

add_executable(Asteroids 
    main.cpp
    src/Editor.cpp
    src/GameState.h
    "src/Systems/PlayerSystems/PlayerSystem.cpp"
    "src/Systems/UtilitySystems/LifetimeSystem.cpp"
    "src/Systems/PhysicsSystems/MovementSystem.cpp"
    "src/Systems/PlayerSystems/ShootingSystem.cpp"

 "src/Systems/AsteroidsSystems/AsteroidSpawnerSystem.cpp" "src/Systems/PhysicsSystems/CollisionSystem.h" "src/Systems/PhysicsSystems/CollisionSystem.cpp" "src/Systems/AsteroidsSystems/DamageSystem.h" "src/Systems/AsteroidsSystems/DamageSystem.cpp" "src/Systems/UtilitySystems/ScoreSystem.h" "src/Systems/UtilitySystems/SpawnImmunitySystem.h" "src/Systems/UtilitySystems/SpawnImmunitySystem.cpp" "src/Systems/RenderSystem.h" "src/Systems/RenderSystem.cpp" "src/Systems/PhysicsSystems/ScreenWrapSystem.h" "src/Systems/PhysicsSystems/ScreenWrapSystem.cpp" "src/SpriteSheet.h" "src/Systems/UtilitySystems/FlickerSystem.h" "src/Systems/UtilitySystems/FlickerSystem.cpp")

target_include_directories(Asteroids PUBLIC
${CMAKE_CURRENT_SOURCE_DIR}
)


target_link_libraries(Asteroids PUBLIC Engine glfw glad stb_image)

add_custom_command(TARGET Asteroids POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Assets"
        "$<TARGET_FILE_DIR:Asteroids>/Assets"
)





