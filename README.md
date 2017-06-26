ReasonglNative
===

Slightly-high level OCaml/Reason bindings to OpenGL.

This repository follows the interface described in [ReasonglInterface](https://github.com/bsansouci/reasongl-interface) which is an abstraction designed on top of OpenGL 2.1 and WebGL. This package also depends on SDL to create a window and handle events.

This package isn't really meant to be used directly. The best way to use this is to wrap your code in a functor that takes a module `Gl` of type `ReasonglInterface.Gl` and then pass in either [ReasonglNative](https://github.com/bsansouci/reasongl-native) or [ReasonglWeb](https://github.com/bsansouci/reasongl-web).

If you want direct bindings to OpenGL use [tgls](https://github.com/bsansouci/tgls).

If you want direct bindings to SDL use [tsdl](https://github.com/bsansouci/tsdl).
