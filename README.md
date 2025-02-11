# Mii Creator WASM

This is a wrapper for FFL for WebAssembly, specifically for usage in Mii Creator.

> ⚠️ You will need to build FFL for WebAssembly first. This library does not build off of FFL's source directly, but links it statically.

## TODO

 - [x] Setup CMake, build library
 - [ ] Bindings:
    - [x] `initBuffers`: Opening a buffer with X size for the DAT
    - [x] `init`: Set up any additional tasks before running. This assumes your DAT data is already injected to the buffer returned by `setup`.
    - [ ] `generateModel`: Generate a head model. This is not a GLTF model, this is raw data.<br>
            When the library is built, I will make it clear how to read this raw data.
    - [ ] `generateTexture`: Generate a face texture.<br>
            Note: This outputs to it's native format to avoid implementing any GL calls directly in the wrapper.<br>
            You will have to manually convert it using a WebGL canvas (this isn't difficult to do, if necessary I can help.)
    - [ ] `cleanup`: Cleans up data returned by the wrapper.
 - [ ] TypeScript Library
    - [ ] Implement downloading of WebAssembly binary (argument for where the WASM binary is located)
    - [ ] Implement basic bindings