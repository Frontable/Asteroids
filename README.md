## Getting Started

### 1. Clone the repository

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