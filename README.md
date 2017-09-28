ReasonGL
---
GL library for web and native.

100% shared rendering code between platforms.

ReasonGL works by exposing a common interface between WebGL and OpenGL and then choosing a backend to use as the implementation of the interface at compile time.

If you want to get started quickly, check out [this branch of ReasonglExampleProject](https://github.com/bsansouci/reasonglexampleproject/tree/bsb-support-new). It's a great starting point to any project using `ReasonGL` and a simple tutorial. The master branch is a much bigger and complete example.

[ReWitness](https://github.com/bsansouci/rewitness) is another big example project using those bindings.

All of the method names are taken directly from opengl, and their docs should apply.

# Where is the code
The code for this is actually divided into 3 sub projects: [reasongl-interface](https://github.com/bsansouci/reasongl-interface), [reasongl-native](https://github.com/bsansouci/reasongl-native), [reasongl-web](https://github.com/bsansouci/reasongl-web). You always need `reasongl-interface` and then can choose which backend you'd like depending on your needs. Simply add them to your [bs-dependencies](https://github.com/bsansouci/reasonglexampleproject/blob/60c23c61348be43ace772d45968048b462c1c2d9/bsconfig.json#L4) in your `bsconfig.json` (see [bsb-native](https://github.com/bsansouci/bsb-native) for help with the build system).

- `reasongl-interface` This exposes the common interface to both backends. This also contains the `Constants` module which contains a bunch of constants that are needed for a lot of GL functions.
- `reasongl-native` This is the native backend, which uses OpenGL 2.1.
- `resaongl-web` This is the web backend, which uses WebGL.
