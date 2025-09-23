# graphics-engine



An experimental **OpenGL-based graphics engine** written in C++.  
This project is a sandbox for learning and implementing modern graphics programming concepts such as rendering pipelines, camera systems, lighting, and procedural terrain generation.



---

##  Features

- [x] Basic OpenGL setup with GLFW
- [x] Render colored triangle
- [x] ImGui integration for debugging and controls
- [x] Cube rendering
- [x] Procedural terrain using Perlin noise
- [ ] Implement directional lighting
- [ ] Shadow mapping
- [ ] Texture loading
- [ ] camera system

---

<p align="center">
  <img src="https://github.com/Tyousafdev/graphics-engine/assets/111599910/77c26390-472b-4f9e-bf37-be98d7f54a91" width="45%"/>
  <img src="https://github.com/Tyousafdev/graphics-engine/assets/111599910/c442c09d-63f0-48ff-b306-61c1400036e9" width="45%"/>
  <img src="https://github.com/user-attachments/assets/06f1b40f-7f75-4a7e-ac82-c82c152b4c1f" width="45%"/>
</p>


---

##  Getting Started

### Prerequisites
Make sure you have the following installed:
- A C++17 compatible compiler (g++, clang, MSVC)

#### macOS
```bash
# Install build tools
brew install cmake make gcc

# Clone and build
git clone https://github.com/Tyousafdev/graphics-engine.git
cd graphics-engine
make

# Run the engine
make run # or ./bin/game in root dir
```


#### Linux (Debian / Ubuntu)
```bash
# Install required build tools and OpenGL headers
sudo apt-get update
sudo apt-get install build-essential cmake xorg-dev libglu1-mesa-dev

# Clone and build
git clone https://github.com/Tyousafdev/graphics-engine.git
cd graphics-engine
make

# Run the engine
make run # or ./bin/game in root dir
```

#### Windows 
```bash
# Install required build tools and OpenGL headers
sudo apt-get update
sudo apt-get install build-essential cmake xorg-dev libglu1-mesa-dev

# Clone and build
git clone https://github.com/Tyousafdev/graphics-engine.git
cd graphics-engine
make

# Run the engine
make run # or ./bin/game in root dir
```


### Verify Build
After running `make`, you should see an executable in the **bin/** directory:

```bash
Graphics-engine/
├── bin/ # Compiled executable
│   └── game
├── Headers/
├── Imgui/
├── Lib/
├── Resources/
├── src/
├── Imgui.ini
├── MakeFile
├── README.md
└── compile_flags.txt
```



##  Usage


###  Keyboard
- **W / A / S / D** → Move camera (forward / left / backward / right)  
- **SPACE** → Move camera up  
- **Left Shift** → Move camera down  
- **ESC** → Quit the application  
- **1** → Toggle mouse capture (switch between free look and normal cursor)  

###  Mouse
- Move mouse → Rotate camera (when mouse capture is enabled)  
- Sensitivity adjustable in ImGui menu  

###  ImGui Debug Menu
-  **drawtriangle** – toggle terrain rendering  
-  **wireMode** – toggle wireframe mode  
-  **frequency / amplitude** – adjust Perlin noise generation  
-  **xScale / zScale / scale** – scale terrain grid  
-  **Base Height / Max Height** – adjust terrain height  
-  **camera speed / sensitivity** – tweak movement feel  
-  Switch between **Main Game** and **Settings** screen  

###  Output
- A **GLFW window** will open showing a procedurally generated terrain.  
- Camera controls let you fly around, while ImGui sliders let you **modify terrain and rendering in real time**.  




































