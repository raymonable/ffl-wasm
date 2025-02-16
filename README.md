# Mii Creator WASM

This is a wrapper for FFL for WebAssembly, specifically made for usage in Mii Creator.

> ⚠️ Please manually add `ffl`, `rio` and `ninTexUtils` to the `lib` folder.<br>
> Source RIO from [here](https://github.com/raymonable/rio). The primary release does not support GLES3 headers by default.
 
> 📝 I installed ZLIB from vcpkg. You can build it from scratch if you'd like, it doesn't matter. As long as ZLIB is available.e

I decided to wipe this README since I keep changing the function names and how they should work.

There is **no** documentation. Start by going into `bin/index.html` and look at how I use it.<br>
Proper JS Bindings for the WebAssembly module are available in `interface.ts`.

If you have any specific requests for things to add, please tell me! ❤️