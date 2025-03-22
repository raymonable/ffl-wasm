# Mii Creator WASM

This was my first real attempt to create C++ to JS bindings for a C++ library.<br>
Hopefully you don't learn anything from this (because it sucks).

But seriously, don't do `malloc` and `free` within C++ to allocate space for objects you're going to import via JavaScript through the WASM HEAP anyway.
In Emscripten, you can export `free` and `malloc`. Just do that.

Functional bindings can be found here: https://github.com/ariankordi/FFL.js
