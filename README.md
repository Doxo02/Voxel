# Voxel

**Voxel** is a basic voxel engine built with C++ and OpenGL, as a Hobby project and is designed to render block-based 3D terrain in real-time. It uses a Brick map to sparsely and efficiently store Voxel data and renders it using Raycasting.

---

## Features

- Simple noise-based terrain generation
- First-person camera controls
- Custom OpenGL shader pipeline using GLSL

---

## Building the Project

### Prerequisites

- C++17 compiler
- CMake 3.10+
- OpenGL 4.5+
- GLFW
- GLAD

### Build Instructions

```bash
git clone https://github.com/Doxo02/Voxel.git
cd Voxel
mkdir build
cmake -B build
make -C build
./build/Voxel
```

---

## Controls

| Key     | Action            |
|---------|-------------------|
| W/A/S/D | Move              |
| Space   | Fly up            |
| Shift   | Fly down          |
| Mouse   | Look around       |
| ESC     | Lock/Unlock Mouse |

---

## TODO

- [x] Refactoring
- [x] Material support
- [x] Lighting system (directional, ambient)
- [ ] Multithreaded chunk generation
- [ ] Save/load voxel worlds

---

## Screenshots

![image](screenshots/Basic%20terrain.png)

---

## License

This project is licensed under the MIT License. See [`LICENSE`](./LICENSE) for details.
