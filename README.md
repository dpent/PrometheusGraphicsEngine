# Prometheus Graphics Engine

This is a graphics engine i made using libraries and tools mentioned in the tech stack section below. This is entirely free to use.

## Features (And planned)
- ✅ Linux support
- ✅ Multisampling
- ✅ Vsync
- ✅ Culling
- ⏳ Raytracing
- ⏳ Rendering scenes
- ⏳ Game engine features like adding objects and editing a scene
- ⏳ Fluid simulation
- ⏳ Multithreaded implementation

## Tech Stack
| Name | Version |
|----------|----------|
| C++    | 23     |
| Make    | Any recent version is ok     |
| Vulkan packages | Get all the packages for linux using sudo apt install |

## Installation
I used a tutorial to set up this project so it will be a lot more convinient for you to do the same by following this link 
[Vulkan Tutorial](https://vulkan-tutorial.com/Development_environment)

### Linux
- You need to clone the repo first using your prefered method e.g. with ssh keys
  ``` bash
  git clone git@githubcom/dpent/PrometheusGameEngine.git
  ```
- Make sure you have the Vulkan SDK for windows or the vulkan packages installed for linux
  ``` bash
  sudo apt install vulkan-tools vulkan-utils libvulkan1 mesa-vulkan-drivers libvulkan-dev
  ```
- To test if vulkan is installed correctly you can run:
  ``` bash
  vkcube # Or vulkaninfo to get a massive amount of info for your vulkan version
  ```
- If you want to recompile or extend the program you should get the necessary libraries used for window creation and math calculations
  ``` bash
  sudo apt install build-essential spirv-tools libglm-dev libglfw3-dev
  ```
- Finally you should get the glslc program (GLSL compiler) from google's unofficial binaries [glslc](https://github.com/google/shaderc/blob/main/downloads.md)
  ``` bash
  https://github.com/google/shaderc/blob/main/downloads.md
  ```
  Pick the gcc or clang version from release. Remember to copy and paste the glslc file in /usr/local/bin for ease of use
### Windows
- Download the latest VulkanSDK, glfw and glm libraries.
- Then open a new Visual Studio project and enter the necessary include and lib paths in the project settings.

# License
MIT License

Copyright (c) 2025 [Your Name]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell    
copies of the Software, and to permit persons to whom the Software is        
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,     
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE  
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER       
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN    
THE SOFTWARE.
