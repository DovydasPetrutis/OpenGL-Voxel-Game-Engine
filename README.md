# OpenGL-Voxel-Game-Engine

A lightweight Minecraft-like, 3D game engine built from scratch using C++ and OpenGL. 

> **Project Status:** This engine is currently in early development. It's a passion/learning project aimed at understanding low-level graphics programming and engine architecture.

---

## Features (Current & Planned)

* **Graphics:** Can render efficiently vertices,textures, has TAA.
* **Windowing & Input:** Both are managed via GLFW, movement is only flying.
* **World:** Can render a randomly generated collection of cubes in a rectangular prism.
* **Planned next:** Basic world-gen(actually generate world as the player is flying), breaking, placing blocks.

---

## Built With

* **Language:** C++20
* **Graphics API:** OpenGL 4.6 Core Profile
* **Libraries:** GLFW, GLAD, GLM

---

## Getting Started

This project is built using **Visual Studio** on Windows. All necessary dependencies are included in the repository.

### Prerequisites
* [Visual Studio 2022](https://visualstudio.microsoft.com/vs/older-downloads/) (with the **Desktop development with C++** workload installed).

### Setup & Compilation
1. **Clone the repository:**
```bash
   git clone https://github.com/DovydasPetrutis/OpenGL-Voxel-Game-Engine.git
```
2. **Open the project:**
   - Navigate to the cloned folder.
   - Double-click the `OpenGL-Voxel-Game-Engine.sln` file to open it in Visual Studio.
3. **Build and Run:**
   - Set your build configuration to **Debug** or **Release** (usually `x64`).
   - Press **F5** (or click the *Local Windows Debugger* button) to compile and launch the voxel engine!

## Photos and tests:

Hardware: 
GPU: Nvidia GeForce RTX 4070 SUPER
CPU: AMD Ryzen 5 7600X
RAM: 32 GB
          
## Photos and tests:

**Hardware:**
- GPU: Nvidia GeForce RTX 4070 SUPER
- CPU: AMD Ryzen 5 7600X
- RAM: 32 GB

- (chunk is a 16x16x16 blocks region)
**10x10x10 chunks cube:**

<img width="2559" height="1439" alt="10x10x10" src="https://github.com/user-attachments/assets/cffa177c-41d8-48cc-a60a-14ac7172c766" />

RAM usage: 600 MB

**100x100x100 chunks cube:**

<img width="2559" height="1439" alt="100x100x100" src="https://github.com/user-attachments/assets/62cb4abc-bd9c-4e44-bd13-ad2e3df9b13d" />

RAM usage: 10 GB

