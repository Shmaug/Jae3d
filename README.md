# Jae3d
Just Another Engine - a basic C++17 DirectX 12 rendering engine.

Requires zlib (via NuGet): Install-Package zlib-msvc14-x64

## Features
+ Asset importer/packer, imports models, textures, fonts, and compiles shaders, then packs them into a file for fast loading
+ Materials with automatic name-based shader parameter handling (somewhat like how Unity has Material.SetValue())
+ Basic OBB frustum culling
### Test game features
+ Profiler with frame time graph
+ Tiled forward rendering (up to 64 lights on-screen)

## Asset Importer
Use to pack assets together. "core.asset" required for some core functionality (SpriteBatch, Debug boxes, Arial font, etc). All these assets are stored in /Assets/Core.
To pack "core.asset", run AssetImporter.exe with the following arguments:

AssetImporter.exe -dr ..\..\Assets\Core -o core.asset -v -uc

To pack your own assets, simply pass a directory of assets with -d or -dr (-dr for recursive). To tell the asset importer where to find the core shader includes,
use -i [dir]. For example:

AssetImporter.exe -dr ..\..\Assets\Shaders -o shaders.asset -v -i ..\..\Assets\Core

The test project requires 4 asset files: core.asset, models.asset, shaders.asset, textures.asset. I packed them with the following commands (from batch files):

AssetImporter.exe -dr ..\..\Assets\Textures -o textures.asset -v -uc

AssetImporter.exe -dr ..\..\Assets\Shaders -o shaders.asset -v -i ..\..\Assets\Core

AssetImporter.exe -dr ..\..\Assets\Models -o models.asset -v -uc

AssetImporter.exe -dr ..\..\Assets\Core -o core.asset -v -uc

## TODO
+ Use an octree for finding renderers/lights within view
+ Everything else

## Future
+ Add scene assets, with a scene editor in the asset importer GUI

## Screenshots
![scr](https://i.imgur.com/yeMr8QS.jpg "screenshot")
![scr](https://i.imgur.com/njdErDx.jpg "screenshot")
