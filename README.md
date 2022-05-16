# Alihan's Custom Optimised OpenGL Developer Tool
A modern OpenGL 3D rendering developer tool. The idea behind this developer tool is very simple, no janky variable names or massive classes.
Everything has been kept as simple as possible. The tool can be used as a playground to implement different graphics programming techniques. The tool comes with its -very powerful custom shader manager. The whole project is based on C++20's latests features and optimizations.

<img src="https://i.imgur.com/grMKHF5.png" width="200" height="300" />

## Compiling

Official support only in Windows. Use at your own risk in another OS.

Requirements:

* x64 v140 or above Toolset
* C++20 compliant Visual Studio

Clone the repo, launch the solution.
Allow a little bit time for NuGet package manager to download some OpenGL libraries. The rest of the libraries are pre-compiled, ready to use. (fmt/spdlog)

## Features

**The Code** 
* Fully written in C++20
* Camel cased clean variable namings and organized code
* Full ImGUI integration as User Interface
* Custom Shader Manager for compilation, linking, activation and very easy debugging and uniform location caching
* Custom camera/mouse controller integration

**Explored Graphics Programming Techniques and Methods** 

* Utilising latest OpenGL +4 features 
* Uniform Buffer Objects usage demonstration
* Programming Pipeline usage demonstration
* Custom Perlin noise generator
* Phong + BlinnPhong reflection model demonstration
* Normal Mapping (Bump Map) demonstration
* Wave vertex animation demonstration
* Two different variations of Toon Shading usage demonstration
* Multiple texture mixing
* Advanced mixing of all techniques above

## Controls

* Spacebar: activate mouse look-at
* WASD move around
* Mouse scroll: zoom in-out

## Screenshots

<img src="https://i.imgur.com/Lpsmzd6.png" />
<img src="https://i.imgur.com/vtiAv92.png" />
<img src="https://i.imgur.com/yX5seY8.png" />
<img src="https://i.imgur.com/qjhiIgh.png" />
<img src="https://i.imgur.com/6xwuz2O.png" />
