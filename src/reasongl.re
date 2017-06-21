/*
 * vim: set ft=rust:
 * vim: set ft=reason:
 */
module Document = {
  type element;
  type window;
  let window: window = [%bs.raw "window"];
  /* external setGlDebug : window => GlT.context => unit = "debugContext" [@@bs.set]; */
  external getElementById : string => element = "document.getElementById" [@@bs.val];
  external getContext : element => string => 'context = "getContext" [@@bs.send];
  external getWidth : element => int = "width" [@@bs.get];
  external getHeight : element => int = "height" [@@bs.get];
  external requestAnimationFrame : (unit => unit) => unit = "window.requestAnimationFrame" [@@bs.val];
  external now : unit => float = "Date.now" [@@bs.val];
  external addEventListener : 'window => string => ('eventT => unit) => unit = "addEventListener" [@@bs.send];
};

external getButton : 'eventT => int = "button" [@@bs.get];

external getClientX : 'eventT => int = "clientX" [@@bs.get];

external getClientY : 'eventT => int = "clientY" [@@bs.get];

external getWhich : 'eventT => int = "which" [@@bs.get];

external getBoundingClientRect : 'canvas => 'leftAndTop = "getBoundingClientRect" [@@bs.send];

external getTop : 'a => int = "top" [@@bs.get];

external getLeft : 'a => int = "left" [@@bs.get];

external getWidth : 'canvas => int = "width" [@@bs.get];

external getHeight : 'canvas => int = "height" [@@bs.get];

external setWidth : 'canvas => int => unit = "width" [@@bs.set];

external setHeight : 'canvas => int => unit = "height" [@@bs.set];

external createElement : string => 'canvas = "document.createElement" [@@bs.val];

let createCanvas () => createElement "canvas";

external addToBody : 'canvas => unit = "document.body.appendChild" [@@bs.val];

external getContext : 'canvas => string => 'options => 'context = "getContext" [@@bs.send];

type styleT;

external getStyle : 'canvas => styleT = "style" [@@bs.get];

external setBackgroundColor : styleT => string => unit = "backgroundColor" [@@bs.set];

type httpRequestT;

external makeXMLHttpRequest : unit => httpRequestT = "XMLHttpRequest" [@@bs.new];

external openFile : httpRequestT =>
                    kind::string =>
                    filename::string =>
                    whatIsThis::Js.boolean =>
                    unit = "open" [@@bs.send];

external onreadystatechange : httpRequestT => (unit => unit) => unit = "onreadystatechange" [@@bs.set];

external getReadyState : httpRequestT => int = "readyState" [@@bs.get];

external getStatus : httpRequestT => int = "status" [@@bs.get];

external getResponseText : httpRequestT => string = "responseText" [@@bs.get];

external sendRequest : httpRequestT => Js.null 'a => unit = "send" [@@bs.send];

module Gl: RGLInterface.t = {
  let target = "web";
  type contextT;
  module type FileT = {type t; let readFile: filename::string => cb::(string => unit) => unit;};
  module File = {
    type t;
    let readFile ::filename ::cb => {
      let rawFile = makeXMLHttpRequest ();
      openFile rawFile kind::"GET" ::filename whatIsThis::Js.false_;
      onreadystatechange
        rawFile
        (
          fun () =>
            if (
              getReadyState rawFile === 4 && (getStatus rawFile === 200 || getStatus rawFile === 0)
            ) {
              cb (getResponseText rawFile)
            }
        );
      sendRequest rawFile Js.null
    };
  };
  module type WindowT = {
    type t;
    let getWidth: t => int;
    let getHeight: t => int;
    let init: argv::array string => t;
    let setWindowSize: window::t => width::int => height::int => unit;
    let getContext: t => contextT;
  };
  module Window = {
    type t;
    let getWidth = getWidth;
    let getHeight = getHeight;
    let init argv::_ => {
      let canvas: t = createCanvas ();
      setBackgroundColor (getStyle canvas) "black";
      addToBody canvas;
      canvas
    };
    let setWindowSize window::(window: t) ::width ::height => {
      setWidth window width;
      setHeight window height
    };
    let getContext (window: t) :contextT =>
      getContext window "webgl" {"preserveDrawingBuffer": true, "antialias": true};
  };
  module Events = Events;
  type mouseButtonEventT =
    button::Events.buttonStateT => state::Events.stateT => x::int => y::int => unit;

  /** See Gl.re for explanation. **/
  let render
      window::(canvas: Window.t)
      mouseDown::(mouseDown: option mouseButtonEventT)=?
      mouseUp::(mouseUp: option mouseButtonEventT)=?
      mouseMove::(mouseMove: option (x::int => y::int => unit))=?
      keyDown::(keyDown: option (keycode::Events.keycodeT => repeat::bool => unit))=?
      keyUp::(keyUp: option (keycode::Events.keycodeT => unit))=?
      windowResize::(windowResize: option (unit => unit))=?
      displayFunc::(displayFunc: float => unit)
      () => {
    switch mouseDown {
    | None => ()
    | Some cb =>
      Document.addEventListener
        canvas
        "mousedown"
        (
          fun e => {
            let button =
              switch (getButton e) {
              | 0 => Events.LeftButton
              | 1 => Events.MiddleButton
              | 2 => Events.RightButton
              | _ => assert false
              };
            let state = Events.MouseDown;
            let rect = getBoundingClientRect canvas;
            let x = getClientX e - getLeft rect;
            let y = getClientY e - getTop rect;
            cb ::button ::state ::x ::y
          }
        )
    };
    switch mouseUp {
    | None => ()
    | Some cb =>
      Document.addEventListener
        canvas
        "mouseup"
        (
          fun e => {
            let button =
              switch (getButton e) {
              | 0 => Events.LeftButton
              | 1 => Events.MiddleButton
              | 2 => Events.RightButton
              | _ => assert false
              };
            let state = Events.MouseUp;
            let rect = getBoundingClientRect canvas;
            let x = getClientX e - getLeft rect;
            let y = getClientY e - getTop rect;
            cb ::button ::state ::x ::y
          }
        )
    };
    switch mouseMove {
    | None => ()
    | Some cb =>
      Document.addEventListener
        canvas
        "mousemove"
        (
          fun e => {
            let rect = getBoundingClientRect canvas;
            let x = getClientX e - getLeft rect;
            let y = getClientY e - getTop rect;
            cb ::x ::y
          }
        )
    };
    let keyLastPressed = ref None;
    switch keyDown {
    | None => ()
    | Some cb =>
      Document.addEventListener
        Document.window /* This is the real window. The "window" param is the canvas. */
        "keydown"
        (
          fun e => {
            let keycode = getWhich e;
            let repeat =
              switch !keyLastPressed {
              | None => false
              | Some k => k === keycode
              };
            keyLastPressed := Some keycode;
            cb keycode::(Events.keycodeMap keycode) ::repeat
          }
        )
    };
    switch keyUp {
    | None => ()
    | Some cb =>
      Document.addEventListener
        Document.window
        "keyup"
        (
          fun e => {
            let keycode = getWhich e;
            keyLastPressed := None;
            cb keycode::(Events.keycodeMap keycode)
          }
        )
    };
    switch windowResize {
    | None => ()
    | Some cb => Document.addEventListener Document.window "resize" (fun _ => cb ())
    };
    let rec tick prev () => {
      let now = Document.now ();
      displayFunc (now -. prev);
      Document.requestAnimationFrame (tick now)
    };
    Document.requestAnimationFrame (tick (Document.now ()))
  };
  type programT;
  type shaderT;
  external clearColor : context::contextT => r::float => g::float => b::float => a::float => unit = "clearColor" [@@bs.send];
  external createProgram : context::contextT => programT = "createProgram" [@@bs.send];
  external createShader : context::contextT => int => shaderT = "createShader" [@@bs.send];
  external _shaderSource : context::contextT => shader::shaderT => source::string => unit = "shaderSource" [@@bs.send];
  let shaderSource ::context ::shader ::source =>
    _shaderSource
      ::context ::shader source::("#version 100 \n precision highp float; \n" ^ source);
  external compileShader : context::contextT => shaderT => unit = "compileShader" [@@bs.send];
  external attachShader : context::contextT => program::programT => shader::shaderT => unit = "attachShader" [@@bs.send];
  external deleteShader : context::contextT => shaderT => unit = "deleteShader" [@@bs.send];
  external linkProgram : context::contextT => programT => unit = "linkProgram" [@@bs.send];
  external useProgram : context::contextT => programT => unit = "useProgram" [@@bs.send];
  type bufferT;
  type attributeT;
  type uniformT;
  external createBuffer : context::contextT => bufferT = "createBuffer" [@@bs.send];
  external bindBuffer : context::contextT => target::int => buffer::bufferT => unit = "bindBuffer" [@@bs.send];
  type textureT;
  external createTexture : context::contextT => textureT = "createTexture" [@@bs.send];
  external activeTexture : context::contextT => int => unit = "activeTexture" [@@bs.send];
  external bindTexture : context::contextT => target::int => texture::textureT => unit = "bindTexture" [@@bs.send];
  external texParameteri : context::contextT => target::int => pname::int => param::int => unit = "texParameteri" [@@bs.send];
  external enable : context::contextT => int => unit = "enable" [@@bs.send];
  external disable : context::contextT => int => unit = "disable" [@@bs.send];
  external blendFunc : context::contextT => int => int => unit = "blendFunc" [@@bs.send];

  /** Those externals are used for bufferData to instantiate what gl.bufferData actually expects, because JS
   *  doesn't differentiate between float and int but the GL backend needs to know the types precisely.
   **/
  external createFloat32ArrayOfArray : array float => 'flot32array = "Float32Array" [@@bs.new];
  external createFloat32Array : int => 'float32array = "Float32Array" [@@bs.new];
  external createFloat64ArrayOfArray : array float => 'flot64array = "Float64Array" [@@bs.new];
  external createFloat64Array : int => 'float64array = "Float64Array" [@@bs.new];
  external createIntArrayOfArray : array int => 'int32array = "Int32Array" [@@bs.new];
  external createInt32ArrayOfArray : array int32 => 'int32array = "Int32Array" [@@bs.new];
  external createIntArray : int => 'int32array = "Int32Array" [@@bs.new];
  external createInt32Array : int => 'int32array = "Int32Array" [@@bs.new];
  external createUint16ArrayOfArray : array int => 'uint16array = "Uint16Array" [@@bs.new];
  external createUint16Array : int => 'uint16array = "Uint16Array" [@@bs.new];
  external createInt16ArrayOfArray : array int => 'int16array = "Int16Array" [@@bs.new];
  external createInt16Array : int => 'int16array = "Int16Array" [@@bs.new];
  external createUint8ArrayOfArray : array int => 'uint8array = "Uint8Array" [@@bs.new];
  external createUint8Array : int => 'uint8array = "Uint8Array" [@@bs.new];
  external createInt8ArrayOfArray : array int => 'int8array = "Int8Array" [@@bs.new];
  external createInt8Array : int => 'int8array = "Int8Array" [@@bs.new];
  external createCharArrayOfArray : array char => 'uint8array = "Uint8Array" [@@bs.new];
  external sub : 'a => int => int => 'a = "subarray" [@@bs.send];
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
  module Bigarray = {
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
    let create (type a) (type b) (kind: kind a b) size :t a b =>
      switch kind {
      | Float64 => createFloat64Array size
      | Float32 => createFloat32Array size
      | Int16 => createInt16Array size
      | Uint16 => createUint16Array size
      | Int8 => createInt8Array size
      | Uint8 => createUint8Array size
      | Char => createUint8Array size
      | Int => createIntArray size
      | Int32 => createInt32Array size
      | Int64 => assert false
      };
    let of_array (type a) (type b) (kind: kind a b) (arr: array a) :t a b =>
      switch kind {
      | Float64 => createFloat64ArrayOfArray arr
      | Float32 => createFloat32ArrayOfArray arr
      | Int16 => createInt16ArrayOfArray arr
      | Uint16 => createUint16ArrayOfArray arr
      | Int8 => createInt8ArrayOfArray arr
      | Uint8 => createUint8ArrayOfArray arr
      | Char => createCharArrayOfArray arr
      | Int => createIntArrayOfArray arr
      | Int32 => createInt32ArrayOfArray arr
      | Int64 => assert false
      };
    external dim : 'a => int = "length" [@@bs.get];
    external get : t 'a 'b => int => 'a = "" [@@bs.get_index];
    external set : t 'a 'b => int => 'a => unit = "" [@@bs.set_index];
    let sub arr ::offset ::len => sub arr offset (offset + len);
  };
  external readPixels : context::contextT =>
                        x::int =>
                        y::int =>
                        width::int =>
                        height::int =>
                        format::int =>
                        type_::int =>
                        pixels::Bigarray.t int Bigarray.int8_unsigned_elt =>
                        unit = "readPixels" [@@bs.send];
  let readPixels_RGBA ::context ::x ::y ::width ::height => {
    let data = createUint8Array (width * height * 4);
    readPixels
      ::context
      ::x
      ::y
      ::width
      ::height
      format::RGLConstants.rgba
      type_::RGLConstants.unsigned_byte
      pixels::data;
    data
  };
  type imageT;
  external getImageWidth : imageT => int = "width" [@@bs.get];
  external getImageHeight : imageT => int = "height" [@@bs.get];
  type loadOptionT =
    | LoadAuto
    | LoadL
    | LoadLA
    | LoadRGB
    | LoadRGBA;

  /** makeImage, setSrc and addEventListener are three helpers for loadImage. */
  external makeImage : unit => imageT = "Image" [@@bs.new];
  /* TODO: when the type of bs.set is `imageT => unit` you get something like

      node_modules/reglweb/src/webgl.re:
        Bsppx.Location.Error(_)
        File "node_modules/reglweb/src/webgl.re", line 1:
        Error: Error while running external preprocessor
        Command line: bsppx.exe '/var/folders/g_/v45pqsrn65xbszd33yvd8lj40000gn/T/camlppx129341' '/var/folders/g_/v45pqsrn65xbszd33yvd8lj40000gn/T/camlppx438846'

        This is due to the fact that bsppx expects 2 args, the "this" and the new value to set.

        We should open an issue in Buckelscript.
     */
  external setSrc : imageT => string => unit = "src" [@@bs.set];
  external addEventListener : imageT => string => (unit => unit) => unit = "addEventListener" [@@bs.send];

  /** TODO: We don't care about forcing load option for web images (we do allow it for native as SOIL supports
      it). We should probably not do this... */
  let loadImage ::filename ::loadOption=? ::callback () =>
    switch loadOption {
    | _ =>
      let image = makeImage ();
      setSrc image filename;
      addEventListener image "load" (fun () => callback (Some image))
    };
  external _texImage2DWithImage : context::contextT =>
                                  target::int =>
                                  level::int =>
                                  internalFormat::int =>
                                  format::int =>
                                  type_::int =>
                                  image::imageT =>
                                  unit = "texImage2D" [@@bs.send];
  let texImage2DWithImage ::context ::target ::level ::image =>
    _texImage2DWithImage
      ::context
      ::target
      ::level
      internalFormat::RGLConstants.rgba
      format::RGLConstants.rgba
      type_::RGLConstants.unsigned_byte
      ::image;
  external _texImage2D : context::contextT =>
                         target::int =>
                         level::int =>
                         internalFormat::int =>
                         width::int =>
                         height::int =>
                         border::int =>
                         format::int =>
                         type_::int =>
                         data::Bigarray.t 'a 'b =>
                         unit = "texImage2D" [@@bs.send];
  let texImage2D_RGBA ::context ::target ::level ::width ::height ::border ::data =>
    _texImage2D
      ::context
      ::target
      ::level
      internalFormat::RGLConstants.rgba
      ::width
      ::height
      ::border
      format::RGLConstants.rgba
      type_::RGLConstants.unsigned_byte
      ::data;
  /*external generateMipmap : context::contextT => target::int => unit = "generateMipmap" [@@bs.send];*/
  external bufferData : context::contextT =>
                        target::int =>
                        data::Bigarray.t 'a 'b =>
                        usage::int =>
                        unit = "bufferData" [@@bs.send];
  external viewport : context::contextT => x::int => y::int => width::int => height::int => unit = "viewport" [@@bs.send];
  external clear : context::contextT => mask::int => unit = "clear" [@@bs.send];
  external getUniformLocation : context::contextT => program::programT => name::string => uniformT = "getUniformLocation" [@@bs.send];
  external getAttribLocation : context::contextT => program::programT => name::string => attributeT = "getAttribLocation" [@@bs.send];
  external enableVertexAttribArray : context::contextT => attribute::attributeT => unit = "enableVertexAttribArray" [@@bs.send];
  external _vertexAttribPointer : context::contextT =>
                                  attribute::attributeT =>
                                  size::int =>
                                  type_::int =>
                                  normalized::Js.boolean =>
                                  stride::int =>
                                  offset::int =>
                                  unit = "vertexAttribPointer" [@@bs.send];
  let vertexAttribPointer ::context ::attribute ::size ::type_ ::normalized ::stride ::offset => {
    let normalized = if normalized {Js.true_} else {Js.false_};
    _vertexAttribPointer ::context ::attribute ::size ::type_ ::normalized ::stride ::offset
  };
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
  module Mat4: Mat4T = {
    type t = array float;
    let to_array a => a;
    external create : unit => t = "" [@@bs.scope "mat4"] [@@bs.module "gl-matrix"];
    external identity : out::t => unit = "" [@@bs.scope "mat4"] [@@bs.module "gl-matrix"];
    external translate : out::t => matrix::t => vec::array float => unit = "" [@@bs.scope "mat4"] [@@bs.module
                                                                    "gl-matrix"
                                                                    ];
    external scale : out::t => matrix::t => vec::array float => unit = "" [@@bs.scope "mat4"] [@@bs.module
                                                                    "gl-matrix"
                                                                    ];
    external rotate : out::t => matrix::t => rad::float => vec::array float => unit = "" [@@bs.scope
                                                                    "mat4"
                                                                    ] [@@bs.module "gl-matrix"];
    external ortho : out::t =>
                     left::float =>
                     right::float =>
                     bottom::float =>
                     top::float =>
                     near::float =>
                     far::float =>
                     unit = "" [@@bs.scope "mat4"] [@@bs.module "gl-matrix"];
  };
  external uniform1i : context::contextT => location::uniformT => val::int => unit = "uniform1i" [@@bs.send];
  external uniform1f : context::contextT => location::uniformT => val::float => unit = "uniform1f" [@@bs.send];
  external _uniformMatrix4fv : context::contextT =>
                               location::uniformT =>
                               transpose::Js.boolean =>
                               value::Mat4.t =>
                               unit = "uniformMatrix4fv" [@@bs.send];
  let uniformMatrix4fv ::context ::location ::value =>
    _uniformMatrix4fv ::context ::location transpose::Js.false_ ::value;
  /* Can return other value types as well, see https://developer.mozilla.org/en-US/docs/Web/API/WebGL_API/Types */
  type shaderParamsInternalT 'a =
    | Shader_delete_status_internal :shaderParamsInternalT bool
    | Compile_status_internal :shaderParamsInternalT bool
    | Shader_type_internal :shaderParamsInternalT int;
  type programParamsInternalT 'a =
    | Program_delete_status_internal :programParamsInternalT bool
    | Link_status_internal :programParamsInternalT bool
    | Validate_status_internal :programParamsInternalT bool;
  /* | Attached_shaders_internal :programParamsInternalT int
     | Active_attributes_internal :programParamsInternalT int
     | Active_uniforms_internal :programParamsInternalT int */
  type shaderParamsT =
    | Shader_delete_status
    | Compile_status
    | Shader_type;
  type programParamsT =
    | Program_delete_status
    | Link_status
    | Validate_status;
  /* | Attached_shaders
     | Active_attributes
     | Active_uniforms */
  external deleteStatus : context::contextT => int = "DELETE_STATUS" [@@bs.get];
  external compileStatus : context::contextT => int = "COMPILE_STATUS" [@@bs.get];
  external linkStatus : context::contextT => int = "LINK_STATUS" [@@bs.get];
  external validateStatus : context::contextT => int = "VALIDATE_STATUS" [@@bs.get];
  external shaderType : context::contextT => int = "SHADER_TYPE" [@@bs.get];
  external _getProgramParameter : context::contextT =>
                                  program::programT =>
                                  paramName::int =>
                                  (programParamsInternalT 'a) [@bs.ignore] =>
                                  'a = "getProgramParameter" [@@bs.send];
  let getProgramParameter ::context ::program ::paramName =>
    switch paramName {
    | Program_delete_status =>
      if (
        _getProgramParameter
          ::context ::program paramName::(deleteStatus ::context) Program_delete_status_internal
      ) {
        1
      } else {
        0
      }
    | Link_status =>
      if (
        _getProgramParameter
          ::context ::program paramName::(linkStatus ::context) Link_status_internal
      ) {
        1
      } else {
        0
      }
    | Validate_status =>
      if (
        _getProgramParameter
          ::context ::program paramName::(validateStatus ::context) Validate_status_internal
      ) {
        1
      } else {
        0
      }
    };
  external _getShaderParameter : context::contextT =>
                                 shader::shaderT =>
                                 paramName::int =>
                                 (shaderParamsInternalT 'a) [@bs.ignore] =>
                                 'a = "getShaderParameter" [@@bs.send];
  let getShaderParameter ::context ::shader ::paramName =>
    switch paramName {
    | Shader_delete_status =>
      if (
        _getShaderParameter
          ::context ::shader paramName::(deleteStatus ::context) Shader_delete_status_internal
      ) {
        1
      } else {
        0
      }
    | Compile_status =>
      if (
        _getShaderParameter
          ::context ::shader paramName::(compileStatus ::context) Compile_status_internal
      ) {
        1
      } else {
        0
      }
    | Shader_type =>
      _getShaderParameter ::context ::shader paramName::(shaderType ::context) Shader_type_internal
    };
  external getShaderInfoLog : context::contextT => shaderT => string = "getShaderInfoLog" [@@bs.send];
  external getProgramInfoLog : context::contextT => programT => string = "getProgramInfoLog" [@@bs.send];
  external getShaderSource : context::contextT => shaderT => string = "getShaderSource" [@@bs.send];
  external drawArrays : context::contextT => mode::int => first::int => count::int => unit = "drawArrays" [@@bs.send];
  external drawElements : context::contextT =>
                          mode::int =>
                          count::int =>
                          type_::int =>
                          offset::int =>
                          unit = "drawElements" [@@bs.send];
};
