ReasonglWeb
===

Slightly-high level OCaml/Reason bindings to WebGL.

This repository follows the interface described in [ReasonglInterface](https://github.com/bsansouci/reasongl-interface) which is an abstraction designed on top of OpenGL 2.1 and WebGL.

This package isn't really meant to be used directly. The best way to use this is to wrap your code in a functor that takes a module `Gl` of type `ReasonglInterface.Gl` and then pass in either [ReasonglNative](https://github.com/bsansouci/reasongl-native) or [ReasonglWeb](https://github.com/bsansouci/reasongl-web).

