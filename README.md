## Getting Started

### Important - since I've changed a lot in the current engine layout there may be bugs occuring with rendering and resseting the game.
### the old layout still works from the 

### 1. Clone the repository main branch https://github.com/Frontable/Asteroids/tree/main

git clone https://github.com/Frontable/Asteroids.git
cd Asteroids

### 2. Generate the project files

cmake -S . -B build

### 3. Build

Release:
cmake --build build --config Release

Debug:
cmake --build build --config Debug

### 4. Run

The executable will be in build/Release/ or build/Debug/ depending on the config.


Controlls:
W -> Apply thrust to move forward.
A / D -> Rotate left and right only while thrust is on.
Space -> Shoot

<img width="1314" height="766" alt="Asteroids" src="https://github.com/user-attachments/assets/4fc6ed3c-9747-4368-a48e-8006fabe8af6" />
