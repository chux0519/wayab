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
- cairo and cairo-gl (the `aur/cairo-glesv2-bin` package if you use Arch)
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

> wayab -f <fps> -o <output>:</path/to/picture/dir>

for example, if you have two directory for two gifs, `/tmp/a` and `/tmp/b`, and you have two output `eDP-1` and `DP-2`.

> wayab -f 10 -o eDP-1:/tmp/a -o DP-2:/tmp/b

will set gif A to `eDP-1` and gif B to `DP-2`, and set fps to 10.

