ReasonGL
---
Simple bindings to opengl 2.0 and webgl.

100% shared code: you write Reason code, and the build system chooses the right backend for you, native or web. 
This library exposes bindings to both webgl and opengl to be used from Reason.

If you want to get started quickly, check out [ReasonglExampleProject](https://github.com/bsansouci/reasonglexampleproject). It's a great starting point to any project using `ReasonGL` and a simple tutorial.

[ReWitness](https://github.com/bsansouci/rewitness) is a big example project using those bindings.

All of the method names are taken directly from opengl, and their docs should apply.

# Where is the code
The code for this is actually in `packages`, dividing into 3 sub projects: `reasongl-interface`, `reasongl-native`, `reasongl-web`.
- `reasongl-interface` This exposes the common interface to both backends (see [ReasonglExampleProject](https://github.com/bsansouci/reasonglexampleproject)). This also contains the `Constants` module which contains a bunch of numbers that are needed for a lot of GL functions.
- `reasongl-native` This is the native backend, which uses OpenGL 2.1.
- `resaongl-web` This is the web backend, which uses WebGL.

We use [lerna](https://lernajs.io/) to manage this repo and publish to npm each package separately. 
