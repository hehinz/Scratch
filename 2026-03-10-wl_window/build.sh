#!/bin/bash
set -e

APP_NAME="wayland_bitmap"
SOURCE="main.c"

echo "Building $APP_NAME..."


# Find the protocol file on your system
XDG_SHELL_XML="/usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml"
XDG_DECO_XML="/usr/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml"

if [ ! -f "$XDG_SHELL_XML" ]; then
    echo "Error: xdg-shell.xml not found. Install wayland-protocols."
    exit 1
fi

echo "Generating XDG-Shell glue code..."
wayland-scanner client-header $XDG_SHELL_XML xdg-shell-client-protocol.h
wayland-scanner private-code $XDG_SHELL_XML xdg-shell-client-protocol.c
wayland-scanner client-header $XDG_DECO_XML xdg-decoration-client-protocol.h
wayland-scanner private-code $XDG_DECO_XML xdg-decoration-client-protocol.c
# Use pkg-config to get the necessary libraries
gcc $SOURCE xdg-shell-client-protocol.c xdg-decoration-client-protocol.c -o $APP_NAME $(pkg-config --cflags --libs wayland-client) -lrt

if [ $? -eq 0 ]; then
    chmod +x $APP_NAME
    echo "Build successful! Run with ./$APP_NAME"
else
    echo "Build failed."
    exit 1
fi
