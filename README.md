# Terrain
A terrain rendering demo built with **LVK** that generates a 3D terrain mesh from a heightmap, includes basic post process pass as well.  
<p align="center">
<b>**Work In Progress**</b>
</p>

****

## Features
- Heightmap-based terrain generation
- Bilinear interpolation for smooth height sampling
- Adjustable terrain resolution independent of source image resolution
- Wireframe visualization
- First-person camera controls
- ImGui debug interface
- Post-processing pass
  - Chromatic Aberration
  - Exposure control

## Screenshots
<p align="center">
  <img src="docs/mesh.jpg" width="48%" style="border: 2px solid black;" />
  <img src="docs/wireframe.jpg" width="48%" style="border: 2px solid black;" />
</p>

<p align="center">
  <img src="docs/wireframe_and_mesh.jpg" width="48%" style="border: 2px solid black;" />
</p>


## Rendering

Rendering consists of two passes.

#### Scene Pass

The terrain is rendered into an off-screen framebuffer.

Optional rendering modes include:

- Solid rendering
- Wireframe overlay

#### Post Processing Pass

The off-screen image is rendered to the swapchain while applying post-processing effects.

Current effects include:

- Chromatic Aberration
- Exposure adjustment

---

## Controls

<p align = center>

| Action | Input |
|---------|-------|
| Move Camera | WASD |
| Look Around | Mouse |
| Toggle Wireframe | ImGui |
| Draw Filled Mesh | ImGui |
| Exposure | ImGui Slider |
| Chromatic Aberration | ImGui Slider |

</p>

## Technologies

**Language**: C++ 20  
**Third-Party**: [LVK](https://github.com/corporateshark/lightweightvk), [GLM](https://github.com/g-truc/glm), [GLFW](https://github.com/glfw/glfw), [ImGui](https://github.com/ocornut/imgui)  
**Rendering API**: Vulkan 1.4