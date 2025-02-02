# C-Render
A pure C implementation of the book "Computer Graphics from Scratch". The rendering pipeline was designed to mimic hardware behavior that every stage will produce new data and discard the old ones. Therefore performance can be quite bad compared to other software renderer.

Uses [fenster](https://github.com/zserge/fenster) as the image viewer.

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
# To render sphere and cubes
./main
# To render cow
./main models/cow.obj
```
The loader can load obj files without texture and normal data, which is `f` and `v` only and `f` can only contain one number per vertex `f 1 2 3`. (`f 1/1 2/2 3/3` is not supported). 

If the program get stuck at clipping stage, the object is probably to big. Scale the instance smaller by adjusting `transform_t` s component or move the instance further by adjusting z component. The recommended numbers for dragon is scale 0.1, z 30. For teapot and cow, scale 1.0 z 20.
