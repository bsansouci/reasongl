/*
 * vim: set ft=rust:
 * vim: set ft=reason:
 */
module type t = {
  let target: string;
  type contextT;
  module type FileT = {type t; let readFile: filename::string => cb::(string => unit) => unit;};
  module File: FileT;
  module type WindowT = {
    type t;
    let getWidth: t => int;
    let getHeight: t => int;
    let init: argv::array string => t;
    let setWindowSize: window::t => width::int => height::int => unit;
    let initDisplayMode: window::t => double_buffer::bool => unit => unit;
    let getContext: t => contextT;
  };
  module Window: WindowT;
  module Events: RGLEvents.t;

  /** We're currently mimicking the JS asynchronous event handling allowing the user to register callbacks.
   * Instead of mutating global state in the Events module, we simply force the user to register all events
   * handlers at once, allowing us to use the closure to keep track of the data for us.
   * For native, the easiest way to handle events is in the render loop, so we force the user to also
   * register the draw call `displayFunc` which will effectively do all of the rendering.
   */
  let render:
    window::Window.t =>
    mouseDown::(button::Events.buttonStateT => state::Events.stateT => x::int => y::int => unit)? =>
    mouseUp::(button::Events.buttonStateT => state::Events.stateT => x::int => y::int => unit)? =>
    mouseMove::(x::int => y::int => unit)? =>
    keyDown::(keycode::Events.keycodeT => repeat::bool => unit)? =>
    keyUp::(keycode::Events.keycodeT => unit)? =>
    windowResize::(unit => unit)? =>
    displayFunc::(float => unit) =>
    unit =>
    unit;
  type programT;
  type shaderT;
  let clearColor: context::contextT => r::float => g::float => b::float => a::float => unit;
  let createProgram: context::contextT => programT;
  let createShader: context::contextT => shaderType::int => shaderT;
  let attachShader: context::contextT => program::programT => shader::shaderT => unit;
  let deleteShader: context::contextT => shader::shaderT => unit;
  let shaderSource: context::contextT => shader::shaderT => source::string => unit;
  let compileShader: context::contextT => shader::shaderT => unit;
  let linkProgram: context::contextT => program::programT => unit;
  let useProgram: context::contextT => program::programT => unit;
  type bufferT;
  type attributeT;
  type uniformT;
  let createBuffer: context::contextT => bufferT;
  let bindBuffer: context::contextT => target::int => buffer::bufferT => unit;
  type textureT;
  let createTexture: context::contextT => textureT;
  let activeTexture: context::contextT => target::int => unit;
  let bindTexture: context::contextT => target::int => texture::textureT => unit;
  let texParameteri: context::contextT => target::int => pname::int => param::int => unit;
  type rawTextureDataT;
  let toTextureData: array int => rawTextureDataT;
  let enable: context::contextT => int => unit;
  let disable: context::contextT => int => unit;
  let blendFunc: context::contextT => int => int => unit;
  /*type frameBufferT;
    let createFrameBuffer: context::contextT => frameBufferT;*/
  /*let bindFrameBuffer:
    context::contextT => target::int => frameBuffer::option frameBufferT => unit;*/
  /*let framebufferTexture2d:
    context::contextT =>
    target::int =>
    attachment::int =>
    texTarget::int =>
    texture::textureT =>
    level::int =>
    unit;*/
  let readPixelsRGBA:
    context::contextT => x::int => y::int => width::int => height::int => rawTextureDataT;
  type imageT;
  let getImageWidth: imageT => int;
  let getImageHeight: imageT => int;
  type loadOptionT =
    | LoadAuto
    | LoadL
    | LoadLA
    | LoadRGB
    | LoadRGBA;
  let loadImage:
    filename::string =>
    loadOption::loadOptionT? =>
    callback::(option imageT => unit) =>
    unit =>
    unit;
  let texImage2DWithImage: context::contextT => target::int => level::int => image::imageT => unit;
  /*let texImage2D:
    context::contextT =>
    target::int =>
    level::int =>
    internalFormat::int =>
    width::int =>
    height::int =>
    format::int =>
    type_::int =>
    data::rawTextureDataT =>
    unit;*/
  let uniform1i: context::contextT => location::uniformT => int => unit;
  let uniform1f: context::contextT => location::uniformT => float => unit;
  /*let generateMipmap: context::contextT => target::int => unit;*/
  module type Bigarray = {
    type t 'a 'b;
    type float64_elt;
    type float32_elt;
    type int16_unsigned_elt;
    type int16_signed_elt;
    type int8_unsigned_elt;
    type int8_signed_elt;
    type int_elt;
    type int32_elt;
    type int64_elt;
    type kind 'a 'b =
      | Float64 :kind float float64_elt
      | Float32 :kind float float32_elt
      | Int16 :kind int int16_signed_elt
      | Uint16 :kind int int16_unsigned_elt
      | Int8 :kind int int8_signed_elt
      | Uint8 :kind int int8_unsigned_elt
      | Char :kind char int8_unsigned_elt
      | Int :kind int int_elt
      | Int64 :kind int64 int64_elt
      | Int32 :kind int32 int32_elt;
    let create: kind 'a 'b => int => t 'a 'b;
    let of_array: kind 'a 'b => array 'a => t 'a 'b;
    let dim: t 'a 'b => int;
    let get: t 'a 'b => int => 'a;
    let set: t 'a 'b => int => 'a => unit;
    let sub: t 'a 'b => offset::int => len::int => t 'a 'b;
  };
  module Bigarray: Bigarray;
  let texImage2D_RGBA:
    context::contextT =>
    target::int =>
    level::int =>
    width::int =>
    height::int =>
    border::int =>
    data::Bigarray.t 'a 'b =>
    unit;
  let bufferData: context::contextT => target::int => data::Bigarray.t 'a 'b => usage::int => unit;
  /* let bufferData2:
     context::contextT =>
     target::int =>
     data::Bigarray.Array1.t 'a 'b Bigarray.c_layout =>
     byteSize::int =>
     usage::int =>
     unit; */
  let viewport: context::contextT => x::int => y::int => width::int => height::int => unit;
  let clear: context::contextT => mask::int => unit;
  let getUniformLocation: context::contextT => program::programT => name::string => uniformT;
  let getAttribLocation: context::contextT => program::programT => name::string => attributeT;
  let enableVertexAttribArray: context::contextT => attribute::attributeT => unit;
  let vertexAttribPointer:
    context::contextT =>
    attribute::attributeT =>
    size::int =>
    type_::int =>
    normalized::bool =>
    stride::int =>
    offset::int =>
    unit;
  module type Mat4T = {
    type t;
    let to_array: t => array float;
    let create: unit => t;
    let identity: out::t => unit;
    let translate: out::t => matrix::t => vec::array float => unit;
    let scale: out::t => matrix::t => vec::array float => unit;
    let rotate: out::t => matrix::t => rad::float => vec::array float => unit;
    let ortho:
      out::t =>
      left::float =>
      right::float =>
      bottom::float =>
      top::float =>
      near::float =>
      far::float =>
      unit;
  };
  module Mat4: Mat4T;
  let uniformMatrix4fv: context::contextT => location::int => transpose::bool => value::array float => unit;
  type shaderParamsT =
    | Shader_delete_status
    | Compile_status
    | Shader_type;
  type programParamsT =
    | Program_delete_status
    | Link_status
    | Validate_status;
  let getProgramParameter:
    context::contextT => program::programT => paramName::programParamsT => int;
  let getShaderParameter: context::contextT => shader::shaderT => paramName::shaderParamsT => int;
  let getShaderInfoLog: context::contextT => shader::shaderT => string;
  let getProgramInfoLog: context::contextT => program::programT => string;
  let getShaderSource: context::contextT => shader::shaderT => string;
  let drawArrays: context::contextT => mode::int => first::int => count::int => unit;
  let drawElements:
    context::contextT => mode::int => count::int => type_::int => offset::int => unit;
};
