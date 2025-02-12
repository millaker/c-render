# C-Render
A pure C implementation of the book "Computer Graphics from Scratch". The rendering pipeline was designed to mimic hardware behavior that every stage will produce new data and discard the old ones. Therefore performance can be quite bad compared to other software renderer.

Uses [fenster](https://github.com/zserge/fenster) as the image viewer and [stb_image.h](https://github.com/nothings/stb) to load textures.

## Requirement
All fenster requirements.

### On Ubuntu
```shell
sudo apt install libX11-dev
make
make ASAN=1 # to turn on address sanitizer
```

## Usage
```shell
# To render a quick demo
./main demo
# To render single model
./main models/*
# To render a moving scene with controllable camera
# Use WASD to move around the scene
./main anime
```
The loader can load obj files without normal data, which is `f` ,`v` and `vt` only.

