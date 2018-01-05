ReasonGL
---
GL library for web and native.

100% shared rendering code between platforms.

If you'd like a higher level API than GL, check out [Reprocessing](https://github.com/Schmavery/reprocessing).

ReasonGL works by exposing a common interface between WebGL and OpenGL and then choosing a backend to use as the implementation of the interface at compile time.

If you want to get started quickly, check out [this simple branch of ReasonglExampleProject](https://github.com/bsansouci/reasonglexampleproject/tree/simple). It's a great starting point to any project using `ReasonGL` and a simple tutorial. The master branch is a much bigger and complete example.

[ReWitness](https://github.com/bsansouci/rewitness) is another big example project using those bindings.

All of the method names are taken directly from opengl, and their docs should apply.

ReasonglNative
===

Slightly high level OCaml/Reason bindings to OpenGL.

This repository follows the interface described in [ReasonglInterface](https://github.com/bsansouci/reasongl-interface) which is an abstraction designed on top of OpenGL 2.1 and WebGL. This package also depends on SDL to create a window and handle events.

The best way to use this is to use [bsb-native](https://github.com/bsansouci/bsb-native), add this package to your `bs-dependencies` along with `ReasonglInterface` and then simply refer to the module `Reasongl`.

If you're not using `bsb-native` you can simply wrap your code in a functor that takes `Gl : ReasonglInterface.Gl.t` and have, for example, a native entry point file containing `module NativeMain = Main (ReasonglNative);` which just passes the right implementation manually.

If you want super thin direct bindings to OpenGL use [tgls](https://github.com/bsansouci/tgls).

If you want direct bindings to SDL use [tsdl](https://github.com/bsansouci/tsdl).

ReasonglWeb
===

Slightly high level OCaml/Reason bindings to WebGL.

This repository follows the interface described in [ReasonglInterface](https://github.com/bsansouci/reasongl-interface) which is an abstraction designed on top of OpenGL 2.1 and WebGL.

The best way to use this is to use [bsb](https://bucklescript.github.io/bucklescript/Manual.html#_bucklescript_build_system_code_bsb_code), add this package to your `bs-dependencies` along with `ReasonglInterface` and then simply refer to the module `Reasongl`.
