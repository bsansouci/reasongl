ReasonglWeb
===

Slightly high level OCaml/Reason bindings to WebGL.

This repository follows the interface described in [ReasonglInterface](https://github.com/bsansouci/reasongl-interface) which is an abstraction designed on top of OpenGL 2.1 and WebGL.

The best way to use this is to use [bsb](https://bucklescript.github.io/bucklescript/Manual.html#_bucklescript_build_system_code_bsb_code), add this package to your `bs-dependencies` along with `ReasonglInterface` and then simply refer to the module `Reasongl`.
