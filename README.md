ReasonglNative
===

Slightly high level OCaml/Reason bindings to OpenGL.

This repository follows the interface described in [ReasonglInterface](https://github.com/bsansouci/reasongl-interface) which is an abstraction designed on top of OpenGL 2.1 and WebGL. This package also depends on SDL to create a window and handle events.

The best way to use this is to use [bsb-native](https://github.com/bsansouci/bsb-native), add this package to your `bs-dependencies` along with `ReasonglInterface` and then simply refer to the module `Reasongl`.

If you're not using `bsb-native` you can simply wrap your code in a functor that takes `Gl : ReasonglInterface.Gl.t` and have, for example, a native entry point file containing `module NativeMain = Main (ReasonglNative);` which just passes the right implementation manually.

If you want super thin direct bindings to OpenGL use [tgls](https://github.com/bsansouci/tgls).

If you want direct bindings to SDL use [tsdl](https://github.com/bsansouci/tsdl).
