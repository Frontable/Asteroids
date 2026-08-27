## Getting Started

## Clone with recurse for submodule
  git clone --recurse-submodule https://github.com/Frontable/Asteroids.git
## CMake
  Configure - cmake --preset debug
  Build     - cmake --build --preset debug
  Run       - cmake --build --preset debug --target run



Controlls:
W -> Apply thrust to move forward.
A / D -> Rotate left and right only while thrust is on.
Space -> Shoot
F1 - toggle debug renderer. Will show hitboxes.

Power-ups:
Speed boost, shield and rapid fire.
##Due to lack of texture for them they currently take a random part of the atlas but they are noticable on drop.
##On pickup its also drawn under the hud as a buff icon in different colors.

<img width="400" height="225" alt="Asteroids 2026-08-28 01-50-29" src="https://github.com/user-attachments/assets/a844b5b1-a12c-476f-9daa-9d8e62c057d2" />


TODO:
  Waves of asteroids instead of consistend spawning, adding enemy ships and lighring.

