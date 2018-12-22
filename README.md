# Jae3d
Just Another Engine - DX12 rendering engine

A basic C++17 DirectX 12 rendering engine. I made this to learn DirectX 12, it is by no means professional.

Requires zlib (via NuGet): Install-Package zlib-msvc14-x64

## Features
+ Asset importer/packer, imports models, textures, fonts, and compiles shaders, then packs them into an .asset file. Also shows assets in a basic UI if run without command line arguments (WIP)
+ Profiler with frame time graph
+ Materials with automatic name-based shader parameter handling (somewhat like how Unity has Material.SetValue())
+ Tiled forward rendering (up to 64 lights on-screen)
+ Basic OBB frustum culling

## TODO
+ Use an octree for finding renderers/lights within view
+ Everything else

## Future
+ Add scene assets, with a scene editor in the asset importer GUI

## Screenshots
![scr](https://i.imgur.com/ub00x6b.png "screenshot")
