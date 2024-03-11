# wayab

wayab(wayland animated background) allows user to set animated background on Linux(wayland).


## Features

- low CPU costs. Unlike `oguri` which change the background by sending requests via wayland-protocols, wayab use [dmabuf](https://wayland-book.com/surfaces/dmabuf.html) to save CPU costs.
- configurable frame rate
- multi-monitor support

## Dependencies

- wayland-client
- wayland-egl
- wayland-protocols
  - stable/xdg-shell/xdg-shell.xml
  - unstable/xdg-output/xdg-output-unstable-v1.xml (for output name)
  - wlr-layer-shell-unstable-v1.xml (for the `background` role and output size)
- cairo and cairo-gl (the `aur/cairo-glesv3-bin` package if you use Arch)
- also need your PC to support EGL and GLESv2

## Build

use cmake

```bash
mkdir build && cd build
cmake ..
make
```

## Usage

1. Extract all frames from your source(gif for example), usually using `imagemagick`'s `convert` command

> convert -coalesce ./input.gif out%04d.png

if you have a viedeo as input, you can use ffmpeg to do similar work.

2. run following command

> wayab -f $fps -o $config

- `-h` show help message 
- `-f` set fps
- `-o` output config, in format: `<output>:<path>:<resize>:<anchor>`
  - output is the output name
  - path is the abs path which contains all frames(need to be PNG) of some animated pictures
  - resize could be none/fill/fit/stretch/tile
  - anchor is a coordinate in range (0.0,0.0) to (1.0,1.0)
    - 0.0,0.0 means left-top
    - 1.0,0.0 means right-top
    - 1.0,1.0 means right-bottom
    - 0.0,1.0 means left-bottom
    - 0.5,0.5 means center
    - ..

for example, if you have two directory for two gifs, `/tmp/a` and `/tmp/b`, and you have two output `eDP-1` and `DP-2`.

> wayab -f 10 -o eDP-1:/tmp/a:none:0.5,0.5 -o DP-2:/tmp/b:fill:0.5,1.0

means, you set the fps to 10, and set `/tmp/a` as `eDP-1`'s background, no resize will be applied and aligned to center. And `/tmp/b` as `DP-2`'s background, it will be resized to fill the output and aligned to center-bottom.


## Example


https://user-images.githubusercontent.com/14276970/144039851-497ed32e-fd20-4b27-9c84-86b1bab2fccd.mp4

image from [1041uuu](https://1041uuu.tumblr.com/page/3)


## Update

cairo dropped opengl backend support since 1.17.8, to make wayab work, we should use cairo 1.17.4 which still provide `cairo-gl`.

just download the prebuilt version of cairo from [aur/cairo-glesv3-bin](https://webreflection.github.io/aur/cairo-glesv2-bin-x86_64-1.17.4.tar.gz).

then use following command with cmake

> cmake -DCairo_ROOT=/path/to/prebuilt/root ..

### Update 2024/03/11

this repo vendored libcairo 1.17.4, so just

> git submodule --init

after clone or add `--recursive` when getting this repo