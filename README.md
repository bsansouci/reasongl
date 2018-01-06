ReasonGL
---
GL library for web and native.

If you'd like a higher level API than GL, check out [Reprocessing](https://github.com/Schmavery/reprocessing).

Install
===
This library is designed to be used with [bsb-native](https://github.com/bsansouci/bsb-native).

Install it using:
`npm install bsansouci/reasongl`

Documentation
===
All of the method names are taken directly from opengl, and their docs should apply.

Example projects
===
If you want to get started quickly, check out [this simple branch of ReasonglExampleProject](https://github.com/bsansouci/reasonglexampleproject/tree/simple). It's a great starting point to any project using `ReasonGL` directly. The master branch is a much bigger and complete example.

[ReWitness](https://github.com/bsansouci/rewitness) is another big example project using those bindings.

How it works
===
ReasonGL works by exposing a common interface between WebGL and OpenGL and then choosing a backend to use as the implementation of the interface at compile time.

Related libraries
===
If you want super thin direct bindings to OpenGL use [tgls](https://github.com/bsansouci/tgls).

If you want direct bindings to SDL use [tsdl](https://github.com/bsansouci/tsdl).
