# Shaders in GirEngine

This document (placeholder rn) is going to contain important information about writing shaders and providing them to the
engine.

For now, it'll just define stuff like the (temporary) hard-coded shader attributes, which means your vertex shaders just
have to put certain data channels in the right order according to a particular layout of the commonly-used attributes (
positions, normals, tan/bitan, texture coords, colors)

It's a lame restriction but I'll figure out a way around it eventually; for now it's not that restrictive imo.

This document is TODO, as is modifying slightly the whole shader compilation part of this engine

## Compiling

As the project-level readme says, just build the CMake `Shaders` target, and it will search your `src/shaders` for glsl
files and then put the binaries in `src/shaders/bin`