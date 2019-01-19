module Document = {
  type t;
  type element;
  type window;
  let window: window = [%bs.raw "window"];
  let document: t = [%bs.raw "document"];
  /* external setGlDebug : window => GlT.context => unit = "debugContext" [@@bs.set]; */
  [@bs.val]
  external getElementById : string => Js.nullable(element) =
    "document.getElementById";
  [@bs.send]
  external getContext : (element, string) => 'context = "getContext";
  [@bs.get] external getWidth : element => int = "width";
  [@bs.get] external getHeight : element => int = "height";
  [@bs.val]
  external requestAnimationFrame : (unit => unit) => int =
    "window.requestAnimationFrame";
  [@bs.val]
  external cancelAnimationFrame : int => unit = "window.cancelAnimationFrame";
  [@bs.val] external now : unit => float = "Date.now";
  [@bs.send]
  external addEventListener : ('window, string, 'eventT => unit) => unit =
    "addEventListener";
  [@bs.val] external devicePixelRatio : float = "window.devicePixelRatio";
  [@bs.set] external setTitle : (t, string) => unit = "title";
};

type canvasT;

[@bs.set] external setHiddenRAFID : ('a, int) => unit = "__hiddenrafid";

[@bs.get] external getButton : 'eventT => int = "button";

[@bs.get] external getClientX : 'eventT => int = "clientX";

[@bs.get] external getClientY : 'eventT => int = "clientY";

[@bs.get]
external getChangedTouches : 'eventT => 'touchListT = "changedTouches";

[@bs.val] external convertToArray : 'notarray => array('thing) = "Array.prototype.slice.call";

[@bs.get] external getTouchIdentifier : 'touchT => int = "identifier";

[@bs.send] external preventDefault : 'eventT => unit = "preventDefault";

[@bs.get] external getWhich : 'eventT => int = "which";

[@bs.send]
external getBoundingClientRect : canvasT => 'leftAndTop =
  "getBoundingClientRect";

[@bs.get] external getTop : 'a => int = "top";

[@bs.get] external getLeft : 'a => int = "left";

let getTouch0 = (e, canvas) => {
  let touches = convertToArray(getChangedTouches(e));
  switch (touches) {
  | [|t|] =>
    let rect = getBoundingClientRect(canvas);
    let x = getClientX(t) - getLeft(rect);
    let y = getClientY(t) - getTop(rect);
    Some((getTouchIdentifier(t), x, y));
  | _ => None
  };
};

[@bs.get] external getCanvasWidth : canvasT => int = "width";

[@bs.get] external getCanvasHeight : canvasT => int = "height";

[@bs.set] external setWidth : (canvasT, int) => unit = "width";

[@bs.set] external setHeight : (canvasT, int) => unit = "height";

[@bs.val]
external createElement : string => canvasT = "document.createElement";

let createCanvas = () => createElement("canvas");

[@bs.val] external addToBody : canvasT => unit = "document.body.appendChild";

[@bs.send]
external getContext : (canvasT, string, 'options) => 'context = "getContext";

type styleT;

[@bs.get] external getStyle : canvasT => styleT = "style";

[@bs.set] external setWidthStyle : (styleT, string) => unit = "width";

[@bs.set] external setHeightStyle : (styleT, string) => unit = "height";

[@bs.set]
external setBackgroundColor : (styleT, string) => unit = "backgroundColor";

type httpRequestT;

[@bs.new]
external makeXMLHttpRequest : unit => httpRequestT = "XMLHttpRequest";

[@bs.send]
external openFile :
  (httpRequestT, ~kind: string, ~filename: string, ~whatIsThis: bool) => unit =
  "open";

[@bs.set]
external onreadystatechange : (httpRequestT, unit => unit) => unit =
  "onreadystatechange";

[@bs.set]
external setResponseType : (httpRequestT, string) => unit = "responseType";

[@bs.get] external getReadyState : httpRequestT => int = "readyState";

[@bs.get] external getStatus : httpRequestT => int = "status";

[@bs.get] external getResponseText : httpRequestT => string = "responseText";

type arrayBufferT;

type soundT;

type audioContextT;

type audioLocT;

type audioGainT;

let makeAudioContext: unit => audioContextT = [%bs.raw
  {| function(_) { return new (window.AudioContext || window.webkitAudioContext)(); } |}
];

[@bs.get] external getResponse : httpRequestT => arrayBufferT = "response";

[@bs.send]
external decodeAudioData :
  (audioContextT, arrayBufferT, soundT => unit) => unit =
  "decodeAudioData";

[@bs.send]
external createBufferSource : audioContextT => audioLocT =
  "createBufferSource";

[@bs.send] external createGain : audioContextT => audioLocT = "createGain";

[@bs.get] external getGain : 'a => audioGainT = "gain";

[@bs.set] external setGainValue : (audioGainT, float) => unit = "value";

[@bs.set]
external setAudioSourceBuffer : (audioLocT, soundT) => unit = "buffer";

[@bs.get]
external getAudioContextDestination : audioContextT => audioLocT =
  "destination";

[@bs.send]
external audioSourceConnect : (audioLocT, audioLocT) => unit = "connect";

[@bs.send] external audioSourceStart : (audioLocT, float) => unit = "start";

[@bs.set] external setAudioSourceLoop : (audioLocT, bool) => unit = "loop";

[@bs.send]
external sendRequest : (httpRequestT, Js.null('a)) => unit = "send";

module Gl: RGLInterface.t = {
  let target = "web";
  type contextT;
  module type FileT = {
    type t;
    let readFile: (~filename: string, ~cb: string => unit) => unit;
  };
  module File = {
    type t;
    let readFile = (~filename, ~cb) => {
      let rawFile = makeXMLHttpRequest();
      openFile(rawFile, ~kind="GET", ~filename, ~whatIsThis=false);
      onreadystatechange(rawFile, () =>
        if (getReadyState(rawFile) === 4
            && (getStatus(rawFile) === 200 || getStatus(rawFile) === 0)) {
          cb(getResponseText(rawFile));
        }
      );
      sendRequest(rawFile, Js.null);
    };
  };
  module type WindowT = {
    type t;
    let getWidth: t => int;
    let getHeight: t => int;
    let getPixelWidth: t => int;
    let getPixelHeight: t => int;
    let getPixelScale: t => float;
    let init: (~screen: string=?, ~argv: array(string)) => t;
    let setWindowSize: (~window: t, ~width: int, ~height: int) => unit;
    let setWindowTitle: (~window: t, ~title: string) => unit;
    let getContext: t => contextT;
  };
  module Window = {
    type t = (canvasT, audioContextT);
    let getWidth = ((window, _ac)) =>
      int_of_float @@
      float_of_int(getCanvasWidth(window))
      /. Document.devicePixelRatio;
    let getHeight = ((window, _ac)) =>
      int_of_float @@
      float_of_int(getCanvasHeight(window))
      /. Document.devicePixelRatio;
    let getPixelWidth = ((window, _ac)) =>
      int_of_float @@ float_of_int @@ getCanvasWidth(window);
    let getPixelHeight = ((window, _ac)) =>
      int_of_float @@ float_of_int @@ getCanvasHeight(window);
    let getPixelScale = (_: t) => Document.devicePixelRatio;
    let init = (~screen=?, ~argv as _) => {
      let node =
        switch (screen) {
        | None => None
        | Some(id) => Js.Nullable.to_opt(Document.getElementById(id))
        };
      let canvas =
        switch (node) {
        | Some(node) => Obj.magic(node)
        | None =>
          let canvas = createCanvas();
          addToBody(canvas);
          canvas;
        };
      setBackgroundColor(getStyle(canvas), "black");
      (canvas, makeAudioContext());
    };
    let setWindowSize = (~window as (w, _), ~width, ~height) => {
      setWidth(
        w,
        int_of_float @@ float_of_int(width) *. Document.devicePixelRatio,
      );
      setHeight(
        w,
        int_of_float @@ float_of_int(height) *. Document.devicePixelRatio,
      );
      setWidthStyle(getStyle(w), string_of_int(width) ++ "px");
      setHeightStyle(getStyle(w), string_of_int(height) ++ "px");
    };
    let setWindowTitle = (~window as _, ~title) =>
      Document.setTitle(Document.document, title);
    let getContext = ((window, _ac)) : contextT =>
      getContext(
        window,
        "webgl",
        {"preserveDrawingBuffer": true, "antialias": true},
      );
  };
  module type AudioT = {
    type t;
    let loadSound: (Window.t, string, t => unit) => unit;
    let playSound: (Window.t, t, ~volume: float, ~loop: bool) => unit;
  };
  module Audio = {
    type t = soundT;
    let loadSound = ((_window, audioctx), path, cb) => {
      let rawFile = makeXMLHttpRequest();
      setResponseType(rawFile, "arraybuffer");
      openFile(rawFile, ~kind="GET", ~filename=path, ~whatIsThis=true);
      onreadystatechange(rawFile, () =>
        if (getReadyState(rawFile) === 4
            && (getStatus(rawFile) === 200 || getStatus(rawFile) === 0)) {
          decodeAudioData(audioctx, getResponse(rawFile), cb);
        }
      );
      sendRequest(rawFile, Js.null);
    };
    let playSound = ((_window, audioctx), sound, ~volume, ~loop) => {
      let src = createBufferSource(audioctx);
      let gain = createGain(audioctx);
      setGainValue(getGain(gain), volume);
      setAudioSourceBuffer(src, sound);
      audioSourceConnect(src, gain);
      audioSourceConnect(gain, getAudioContextDestination(audioctx));
      audioSourceStart(src, 0.0);
      setAudioSourceLoop(src, loop);
    };
  };
  module Events = Events;
  type mouseButtonEventT =
    (~button: Events.buttonStateT, ~state: Events.stateT, ~x: int, ~y: int) =>
    unit;

  /*** See Gl.re for explanation. **/
  let render =
      (
        ~window as (canvas, _ac): Window.t,
        ~mouseDown: option(mouseButtonEventT)=?,
        ~mouseUp: option(mouseButtonEventT)=?,
        ~mouseMove: option((~x: int, ~y: int) => unit)=?,
        ~keyDown: option((~keycode: Events.keycodeT, ~repeat: bool) => unit)=?,
        ~keyUp: option((~keycode: Events.keycodeT) => unit)=?,
        ~windowResize: option(unit => unit)=?,
        ~displayFunc: float => unit,
        (),
      ) => {
    let singleTouchId = ref(None);
    switch (mouseDown) {
    | None => ()
    | Some(cb) =>
      Document.addEventListener(canvas, "touchstart", e =>
        switch (getTouch0(e, canvas)) {
        | Some((touchId, x, y)) =>
          switch (singleTouchId^) {
          | None =>
            singleTouchId := Some(touchId);
            preventDefault(e);
            cb(~button=Events.LeftButton, ~state=Events.MouseDown, ~x, ~y);
          | _ => singleTouchId := None
          }
        | None => ()
        }
      );
      Document.addEventListener(
        canvas,
        "mousedown",
        e => {
          let button =
            switch (getButton(e)) {
            | 0 => Events.LeftButton
            | 1 => Events.MiddleButton
            | 2 => Events.RightButton
            | _ => assert false
            };
          let state = Events.MouseDown;
          let rect = getBoundingClientRect(canvas);
          let x = getClientX(e) - getLeft(rect);
          let y = getClientY(e) - getTop(rect);
          cb(~button, ~state, ~x, ~y);
        },
      );
    };
    switch (mouseUp) {
    | None => ()
    | Some(cb) =>
      Document.addEventListener(canvas, "touchend", e =>
        switch (getTouch0(e, canvas)) {
        | Some((touchId, x, y)) =>
          switch (singleTouchId^) {
          | Some(id) when id == touchId =>
            singleTouchId := None;
            preventDefault(e);
            cb(~button=Events.LeftButton, ~state=Events.MouseUp, ~x, ~y);
          | _ => ()
          }
        | None => ()
        }
      );
      Document.addEventListener(canvas, "touchcancel", e =>
        switch (getTouch0(e, canvas)) {
        | Some((touchId, x, y)) =>
          switch (singleTouchId^) {
          | Some(id) when id == touchId =>
            singleTouchId := None;
            preventDefault(e);
            cb(~button=Events.LeftButton, ~state=Events.MouseUp, ~x, ~y);
          | _ => ()
          }
        | None => ()
        }
      );
      Document.addEventListener(
        canvas,
        "mouseup",
        e => {
          let button =
            switch (getButton(e)) {
            | 0 => Events.LeftButton
            | 1 => Events.MiddleButton
            | 2 => Events.RightButton
            | _ => assert false
            };
          let state = Events.MouseUp;
          let rect = getBoundingClientRect(canvas);
          let x = getClientX(e) - getLeft(rect);
          let y = getClientY(e) - getTop(rect);
          cb(~button, ~state, ~x, ~y);
        },
      );
    };
    switch (mouseMove) {
    | None => ()
    | Some(cb) =>
      Document.addEventListener(canvas, "touchmove", e =>
        switch (getTouch0(e, canvas)) {
        | Some((touchId, x, y)) =>
          switch (singleTouchId^) {
          | Some(id) when id == touchId =>
            preventDefault(e);
            cb(~x, ~y);
          | _ => ()
          }
        | None => ()
        }
      );
      Document.addEventListener(
        canvas,
        "mousemove",
        e => {
          let rect = getBoundingClientRect(canvas);
          let x = getClientX(e) - getLeft(rect);
          let y = getClientY(e) - getTop(rect);
          cb(~x, ~y);
        },
      );
    };
    let keyLastPressed = ref([]);
    switch (keyDown) {
    | None => ()
    | Some(cb) =>
      Document.addEventListener(
        Document.window, /* This is the real window. The "window" param is the canvas. */
        "keydown",
        e => {
          let keycode = Int32.of_int(getWhich(e));
          let repeat =
            List.fold_left(
              (acc, k) => acc || k === keycode,
              false,
              keyLastPressed^,
            );
          if (! repeat) {
            keyLastPressed := [keycode, ...keyLastPressed^];
          };
          cb(~keycode=Events.keycodeMap(keycode), ~repeat);
        },
      )
    };
    switch (keyUp) {
    | None => ()
    | Some(cb) =>
      Document.addEventListener(
        Document.window,
        "keyup",
        e => {
          let keycode = Int32.of_int(getWhich(e));
          keyLastPressed := List.filter(k => k !== keycode, keyLastPressed^);
          cb(~keycode=Events.keycodeMap(keycode));
        },
      )
    };
    switch (windowResize) {
    | None => ()
    | Some(cb) =>
      Document.addEventListener(Document.window, "resize", (_) => cb())
    };
    let frame = ref(None);
    let rec tick = (prev, ()) => {
      let now = Document.now();
      displayFunc(now -. prev);
      let id = Document.requestAnimationFrame(tick(now));
      frame := Some(id);
      setHiddenRAFID(canvas, id);
    };
    let id = Document.requestAnimationFrame(tick(Document.now()));
    frame := Some(id);
    setHiddenRAFID(canvas, id);
    play =>
      switch (frame^) {
      | None =>
        if (play) {
          let id = Document.requestAnimationFrame(tick(Document.now()));
          frame := Some(id);
          setHiddenRAFID(canvas, id);
          true;
        } else {
          false;
        }
      | Some(id) =>
        if (! play) {
          Document.cancelAnimationFrame(id);
          frame := None;
          false;
        } else {
          true;
        }
      };
  };
  type programT;
  type shaderT;
  [@bs.send]
  external clearColor :
    (~context: contextT, ~r: float, ~g: float, ~b: float, ~a: float) => unit =
    "clearColor";
  [@bs.send]
  external createProgram : (~context: contextT) => programT = "createProgram";
  [@bs.send]
  external createShader : (~context: contextT, int) => shaderT =
    "createShader";
  [@bs.send]
  external _shaderSource :
    (~context: contextT, ~shader: shaderT, ~source: string) => unit =
    "shaderSource";
  let shaderSource = (~context, ~shader, ~source) =>
    _shaderSource(
      ~context,
      ~shader,
      ~source="#version 100 \n precision highp float; \n" ++ source,
    );
  [@bs.send]
  external compileShader : (~context: contextT, shaderT) => unit =
    "compileShader";
  [@bs.send]
  external attachShader :
    (~context: contextT, ~program: programT, ~shader: shaderT) => unit =
    "attachShader";
  [@bs.send]
  external deleteShader : (~context: contextT, shaderT) => unit =
    "deleteShader";
  [@bs.send]
  external linkProgram : (~context: contextT, programT) => unit =
    "linkProgram";
  [@bs.send]
  external useProgram : (~context: contextT, programT) => unit = "useProgram";
  type bufferT;
  type attributeT;
  type uniformT;
  [@bs.send]
  external createBuffer : (~context: contextT) => bufferT = "createBuffer";
  [@bs.send]
  external bindBuffer :
    (~context: contextT, ~target: int, ~buffer: bufferT) => unit =
    "bindBuffer";
  type textureT;
  [@bs.send]
  external createTexture : (~context: contextT) => textureT = "createTexture";
  [@bs.send]
  external activeTexture : (~context: contextT, int) => unit = "activeTexture";
  [@bs.send]
  external bindTexture :
    (~context: contextT, ~target: int, ~texture: textureT) => unit =
    "bindTexture";
  [@bs.send]
  external texParameteri :
    (~context: contextT, ~target: int, ~pname: int, ~param: int) => unit =
    "texParameteri";
  type framebufferT;
  [@bs.send]
  external createFramebuffer : (~context: contextT) => framebufferT = "";
  [@bs.send]
  external bindFramebuffer :
    (~context: contextT, ~target: int, ~framebuffer: framebufferT) => unit =
    "";
  [@bs.send]
  external bindDefaultFramebuffer :
    (~context: contextT, ~target: int, [@bs.as {json|null|json}] _) => unit =
    "bindFramebuffer";
  [@bs.send]
  external framebufferTexture2D :
    (
      ~context: contextT,
      ~target: int,
      ~attachment: int,
      ~texTarget: int,
      ~texture: textureT,
      [@bs.as {json|0|json}] _
    ) =>
    unit =
    "";
  [@bs.send] external enable : (~context: contextT, int) => unit = "enable";
  [@bs.send] external disable : (~context: contextT, int) => unit = "disable";
  [@bs.send]
  external blendFunc : (~context: contextT, int, int) => unit = "blendFunc";

  /*** Those externals are used for bufferData to instantiate what gl.bufferData actually expects, because JS
   *  doesn't differentiate between float and int but the GL backend needs to know the types precisely.
   **/
  [@bs.new]
  external createFloat32ArrayOfArray : array(float) => 'flot32array =
    "Float32Array";
  [@bs.new]
  external createFloat32Array : int => 'float32array = "Float32Array";
  [@bs.new]
  external createFloat64ArrayOfArray : array(float) => 'flot64array =
    "Float64Array";
  [@bs.new]
  external createFloat64Array : int => 'float64array = "Float64Array";
  [@bs.new]
  external createIntArrayOfArray : array(int) => 'int32array = "Int32Array";
  [@bs.new]
  external createInt32ArrayOfArray : array(int32) => 'int32array =
    "Int32Array";
  [@bs.new] external createIntArray : int => 'int32array = "Int32Array";
  [@bs.new] external createInt32Array : int => 'int32array = "Int32Array";
  [@bs.new]
  external createUint16ArrayOfArray : array(int) => 'uint16array =
    "Uint16Array";
  [@bs.new] external createUint16Array : int => 'uint16array = "Uint16Array";
  [@bs.new]
  external createInt16ArrayOfArray : array(int) => 'int16array = "Int16Array";
  [@bs.new] external createInt16Array : int => 'int16array = "Int16Array";
  [@bs.new]
  external createUint8ArrayOfArray : array(int) => 'uint8array = "Uint8Array";
  [@bs.new] external createUint8Array : int => 'uint8array = "Uint8Array";
  [@bs.new]
  external createInt8ArrayOfArray : array(int) => 'int8array = "Int8Array";
  [@bs.new] external createInt8Array : int => 'int8array = "Int8Array";
  [@bs.new]
  external createCharArrayOfArray : array(char) => 'uint8array = "Uint8Array";
  [@bs.send] external sub : ('a, int, int) => 'a = "subarray";
  module type Bigarray = {
    type t('a, 'b);
    type float64_elt;
    type float32_elt;
    type int16_unsigned_elt;
    type int16_signed_elt;
    type int8_unsigned_elt;
    type int8_signed_elt;
    type int_elt;
    type int32_elt;
    type int64_elt;
    type kind('a, 'b) =
      | Float64: kind(float, float64_elt)
      | Float32: kind(float, float32_elt)
      | Int16: kind(int, int16_signed_elt)
      | Uint16: kind(int, int16_unsigned_elt)
      | Int8: kind(int, int8_signed_elt)
      | Uint8: kind(int, int8_unsigned_elt)
      | Char: kind(char, int8_unsigned_elt)
      | Int: kind(int, int_elt)
      | Int64: kind(int64, int64_elt)
      | Int32: kind(int32, int32_elt);
    let create: (kind('a, 'b), int) => t('a, 'b);
    let of_array: (kind('a, 'b), array('a)) => t('a, 'b);
    let dim: t('a, 'b) => int;
    let blit: (t('a, 'b), t('a, 'b)) => unit;
    let unsafe_blit:
      (t('a, 'b), t('a, 'b), ~offset: int, ~numOfBytes: int) => unit;
    let get: (t('a, 'b), int) => 'a;
    let unsafe_get: (t('a, 'b), int) => 'a;
    let set: (t('a, 'b), int, 'a) => unit;
    let unsafe_set: (t('a, 'b), int, 'a) => unit;
    let sub: (t('a, 'b), ~offset: int, ~len: int) => t('a, 'b);
  };
  module Bigarray = {
    type t('a, 'b);
    type float64_elt;
    type float32_elt;
    type int16_unsigned_elt;
    type int16_signed_elt;
    type int8_unsigned_elt;
    type int8_signed_elt;
    type int_elt;
    type int32_elt;
    type int64_elt;
    type kind('a, 'b) =
      | Float64: kind(float, float64_elt)
      | Float32: kind(float, float32_elt)
      | Int16: kind(int, int16_signed_elt)
      | Uint16: kind(int, int16_unsigned_elt)
      | Int8: kind(int, int8_signed_elt)
      | Uint8: kind(int, int8_unsigned_elt)
      | Char: kind(char, int8_unsigned_elt)
      | Int: kind(int, int_elt)
      | Int64: kind(int64, int64_elt)
      | Int32: kind(int32, int32_elt);
    let create = (type a, type b, kind: kind(a, b), size) : t(a, b) =>
      switch (kind) {
      | Float64 => createFloat64Array(size)
      | Float32 => createFloat32Array(size)
      | Int16 => createInt16Array(size)
      | Uint16 => createUint16Array(size)
      | Int8 => createInt8Array(size)
      | Uint8 => createUint8Array(size)
      | Char => createUint8Array(size)
      | Int => createIntArray(size)
      | Int32 => createInt32Array(size)
      | Int64 => assert false
      };
    let of_array =
        (type a, type b, kind: kind(a, b), arr: array(a))
        : t(a, b) =>
      switch (kind) {
      | Float64 => createFloat64ArrayOfArray(arr)
      | Float32 => createFloat32ArrayOfArray(arr)
      | Int16 => createInt16ArrayOfArray(arr)
      | Uint16 => createUint16ArrayOfArray(arr)
      | Int8 => createInt8ArrayOfArray(arr)
      | Uint8 => createUint8ArrayOfArray(arr)
      | Char => createCharArrayOfArray(arr)
      | Int => createIntArrayOfArray(arr)
      | Int32 => createInt32ArrayOfArray(arr)
      | Int64 => assert false
      };
    [@bs.get] external dim : 'a => int = "length";
    [@bs.send] external blit : (t('a, 'b), t('a, 'b)) => unit = "set";
    [@bs.send]
    external unsafe_blit : (t('a, 'b), t('a, 'b), ~offset: int) => unit =
      "set";
    let unsafe_blit:
      (t('a, 'b), t('a, 'b), ~offset: int, ~numOfBytes: int) => unit =
      (arr, arr2, ~offset, ~numOfBytes as _) =>
        unsafe_blit(arr2, arr, ~offset);
    [@bs.get_index] external get : (t('a, 'b), int) => 'a = "";
    [@bs.get_index] external unsafe_get : (t('a, 'b), int) => 'a = "";
    [@bs.set_index] external set : (t('a, 'b), int, 'a) => unit = "";
    [@bs.set_index] external unsafe_set : (t('a, 'b), int, 'a) => unit = "";
    let sub = (arr, ~offset, ~len) => sub(arr, offset, offset + len);
  };
  [@bs.send]
  external texSubImage2D :
    (
      ~context: contextT,
      ~target: int,
      ~level: int,
      ~xoffset: int,
      ~yoffset: int,
      ~width: int,
      ~height: int,
      ~format: int,
      ~type_: int,
      ~pixels: Bigarray.t('a, 'b)
    ) =>
    unit =
    "texSubImage2D";
  [@bs.send]
  external readPixels :
    (
      ~context: contextT,
      ~x: int,
      ~y: int,
      ~width: int,
      ~height: int,
      ~format: int,
      ~type_: int,
      ~pixels: Bigarray.t(int, Bigarray.int8_unsigned_elt)
    ) =>
    unit =
    "readPixels";
  let readPixels_RGBA = (~context, ~x, ~y, ~width, ~height) => {
    let data = createUint8Array(width * height * 4);
    readPixels(
      ~context,
      ~x,
      ~y,
      ~width,
      ~height,
      ~format=RGLConstants.rgba,
      ~type_=RGLConstants.unsigned_byte,
      ~pixels=data,
    );
    data;
  };
  type imageT;
  [@bs.get] external getImageWidth : imageT => int = "width";
  [@bs.get] external getImageHeight : imageT => int = "height";
  type loadOptionT =
    | LoadAuto
    | LoadL
    | LoadLA
    | LoadRGB
    | LoadRGBA;

  /*** makeImage, setSrc and addEventListener are three helpers for loadImage. */
  [@bs.new] external makeImage : unit => imageT = "Image";
  /* TODO: when the type of bs.set is `imageT => unit` you get something like

      node_modules/reglweb/src/webgl.re:
        Bsppx.Location.Error(_)
        File "node_modules/reglweb/src/webgl.re", line 1:
        Error: Error while running external preprocessor
        Command line: bsppx.exe '/var/folders/g_/v45pqsrn65xbszd33yvd8lj40000gn/T/camlppx129341' '/var/folders/g_/v45pqsrn65xbszd33yvd8lj40000gn/T/camlppx438846'

        This is due to the fact that bsppx expects 2 args, the "this" and the new value to set.

        We should open an issue in Buckelscript.
     */
  [@bs.set] external setSrc : (imageT, string) => unit = "src";
  [@bs.send]
  external addEventListener : (imageT, string, unit => unit) => unit =
    "addEventListener";
  [@bs.val] external btoa : string => string = "";

  /*** TODO: We don't care about forcing load option for web images (we do allow it for native as SOIL supports
       it). We should probably not do this... */
  let loadImage = (~filename, ~loadOption=?, ~callback, ()) =>
    switch (loadOption) {
    | _ =>
      let image = makeImage();
      setSrc(image, filename);
      addEventListener(image, "load", () => callback(Some(image)));
    };
  let loadImageFromMemory = (~data, ~loadOption=?, ~callback, ()) => {
    let image = makeImage();
    setSrc(image, "data:image/png;base64," ++ btoa(data));
    addEventListener(image, "load", () => callback(Some(image)));
  };
  [@bs.send]
  external _texImage2DWithImage :
    (
      ~context: contextT,
      ~target: int,
      ~level: int,
      ~internalFormat: int,
      ~format: int,
      ~type_: int,
      ~image: imageT
    ) =>
    unit =
    "texImage2D";
  let texImage2DWithImage = (~context, ~target, ~level, ~image) =>
    _texImage2DWithImage(
      ~context,
      ~target,
      ~level,
      ~internalFormat=RGLConstants.rgba,
      ~format=RGLConstants.rgba,
      ~type_=RGLConstants.unsigned_byte,
      ~image,
    );
  [@bs.send]
  external _texImage2D :
    (
      ~context: contextT,
      ~target: int,
      ~level: int,
      ~internalFormat: int,
      ~width: int,
      ~height: int,
      ~border: int,
      ~format: int,
      ~type_: int,
      ~data: Bigarray.t('a, 'b)
    ) =>
    unit =
    "texImage2D";
  let texImage2D_RGBA =
      (~context, ~target, ~level, ~width, ~height, ~border, ~data) =>
    _texImage2D(
      ~context,
      ~target,
      ~level,
      ~internalFormat=RGLConstants.rgba,
      ~width,
      ~height,
      ~border,
      ~format=RGLConstants.rgba,
      ~type_=RGLConstants.unsigned_byte,
      ~data,
    );
  let texImage2D_null = [%bs.raw
    {| function(gl, target, level, width, height) {
    gl.texImage2D(target, level, gl.RGBA, width, height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null)
  } |}
  ];
  [@bs.send]
  external vertexAttribDivisor :
    (~context: contextT, ~attribute: attributeT, ~divisor: int) => unit =
    "vertexAttribDivisor";
  /*external generateMipmap : context::contextT => target::int => unit = "generateMipmap" [@@bs.send];*/
  [@bs.send]
  external bufferData :
    (
      ~context: contextT,
      ~target: int,
      ~data: Bigarray.t('a, 'b),
      ~usage: int
    ) =>
    unit =
    "bufferData";
  [@bs.send]
  external viewport :
    (~context: contextT, ~x: int, ~y: int, ~width: int, ~height: int) => unit =
    "viewport";
  [@bs.send]
  external clear : (~context: contextT, ~mask: int) => unit = "clear";
  [@bs.send]
  external getUniformLocation :
    (~context: contextT, ~program: programT, ~name: string) => uniformT =
    "getUniformLocation";
  [@bs.send]
  external getAttribLocation :
    (~context: contextT, ~program: programT, ~name: string) => attributeT =
    "getAttribLocation";
  [@bs.send]
  external enableVertexAttribArray :
    (~context: contextT, ~attribute: attributeT) => unit =
    "enableVertexAttribArray";
  [@bs.send]
  external _vertexAttribPointer :
    (
      ~context: contextT,
      ~attribute: attributeT,
      ~size: int,
      ~type_: int,
      ~normalize: bool,
      ~stride: int,
      ~offset: int
    ) =>
    unit =
    "vertexAttribPointer";
  let vertexAttribPointer =
      (~context, ~attribute, ~size, ~type_, ~normalize, ~stride, ~offset) => {
    let normalize = if (normalize) {true} else {false};
    _vertexAttribPointer(
      ~context,
      ~attribute,
      ~size,
      ~type_,
      ~normalize,
      ~stride,
      ~offset,
    );
  };
  module type Mat4T = {
    type t;
    let to_array: t => array(float);
    let create: unit => t;
    let identity: (~out: t) => unit;
    let translate: (~out: t, ~matrix: t, ~vec: array(float)) => unit;
    let scale: (~out: t, ~matrix: t, ~vec: array(float)) => unit;
    let rotate:
      (~out: t, ~matrix: t, ~rad: float, ~vec: array(float)) => unit;
    let ortho:
      (
        ~out: t,
        ~left: float,
        ~right: float,
        ~bottom: float,
        ~top: float,
        ~near: float,
        ~far: float
      ) =>
      unit;
    let perspective:
      (
        ~out: t,
        ~fovy: float,
        ~aspect: float,
        ~near: float,
        ~far: float,
      ) =>
      unit;
    let lookAt:
      (
        ~out: t,
        ~eye: array(float),
        ~center: array(float),
        ~up: array(float),
      ) =>
      unit;
  };
  module Mat4: Mat4T = {
    type t = array(float);
    let to_array = a => a;
    [@bs.scope "mat4"] [@bs.module "gl-matrix"]
    external create : unit => t = "";
    [@bs.scope "mat4"] [@bs.module "gl-matrix"]
    external identity : (~out: t) => unit = "";
    [@bs.scope "mat4"] [@bs.module "gl-matrix"]
    external translate : (~out: t, ~matrix: t, ~vec: array(float)) => unit =
      "";
    [@bs.scope "mat4"] [@bs.module "gl-matrix"]
    external scale : (~out: t, ~matrix: t, ~vec: array(float)) => unit = "";
    [@bs.scope "mat4"] [@bs.module "gl-matrix"]
    external rotate :
      (~out: t, ~matrix: t, ~rad: float, ~vec: array(float)) => unit =
      "";
    [@bs.scope "mat4"] [@bs.module "gl-matrix"]
    external ortho :
      (
        ~out: t,
        ~left: float,
        ~right: float,
        ~bottom: float,
        ~top: float,
        ~near: float,
        ~far: float
      ) =>
      unit =
      "";
    [@bs.scope "mat4"] [@bs.module "gl-matrix"]
    external perspective:
      (
        ~out: t,
        ~fovy: float,
        ~aspect: float,
        ~near: float,
        ~far: float
      ) =>
      unit =
      "";
    [@bs.scope "mat4"] [@bs.module "gl-matrix"]
    external lookAt:
      (
        ~out: t,
        ~eye: array(float),
        ~center: array(float),
        ~up: array(float),
      ) =>
      unit =
      "";
  };
  [@bs.send]
  external uniform1i :
    (~context: contextT, ~location: uniformT, ~value: int) => unit =
    "uniform1i";
  [@bs.send]
  external uniform1f :
    (~context: contextT, ~location: uniformT, ~value: float) => unit =
    "uniform1f";
  [@bs.send]
  external uniform2f :
    (~context: contextT, ~location: uniformT, ~v1: float, ~v2: float) => unit =
    "uniform2f";
  [@bs.send]
  external uniform3f :
    (
      ~context: contextT,
      ~location: uniformT,
      ~v1: float,
      ~v2: float,
      ~v3: float
    ) =>
    unit =
    "uniform3f";
  [@bs.send]
  external uniform4f :
    (
      ~context: contextT,
      ~location: uniformT,
      ~v1: float,
      ~v2: float,
      ~v3: float,
      ~v4: float
    ) =>
    unit =
    "uniform4f";
  [@bs.send]
  external _uniformMatrix4fv :
    (
      ~context: contextT,
      ~location: uniformT,
      ~transpose: bool,
      ~value: Mat4.t
    ) =>
    unit =
    "uniformMatrix4fv";
  let uniformMatrix4fv = (~context, ~location, ~value) =>
    _uniformMatrix4fv(~context, ~location, ~transpose=false, ~value);
  /* Can return other value types as well, see https://developer.mozilla.org/en-US/docs/Web/API/WebGL_API/Types */
  type shaderParamsInternalT('a) =
    | Shader_delete_status_internal: shaderParamsInternalT(bool)
    | Compile_status_internal: shaderParamsInternalT(bool)
    | Shader_type_internal: shaderParamsInternalT(int);
  type programParamsInternalT('a) =
    | Program_delete_status_internal: programParamsInternalT(bool)
    | Link_status_internal: programParamsInternalT(bool)
    | Validate_status_internal: programParamsInternalT(bool);
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
  [@bs.get]
  external deleteStatus : (~context: contextT) => int = "DELETE_STATUS";
  [@bs.get]
  external compileStatus : (~context: contextT) => int = "COMPILE_STATUS";
  [@bs.get] external linkStatus : (~context: contextT) => int = "LINK_STATUS";
  [@bs.get]
  external validateStatus : (~context: contextT) => int = "VALIDATE_STATUS";
  [@bs.get] external shaderType : (~context: contextT) => int = "SHADER_TYPE";
  [@bs.send]
  external _getProgramParameter :
    (
      ~context: contextT,
      ~program: programT,
      ~paramName: int,
      [@bs.ignore] programParamsInternalT('a)
    ) =>
    'a =
    "getProgramParameter";
  let getProgramParameter = (~context, ~program, ~paramName) =>
    switch (paramName) {
    | Program_delete_status =>
      if (_getProgramParameter(
            ~context,
            ~program,
            ~paramName=deleteStatus(~context),
            Program_delete_status_internal,
          )) {
        1;
      } else {
        0;
      }
    | Link_status =>
      if (_getProgramParameter(
            ~context,
            ~program,
            ~paramName=linkStatus(~context),
            Link_status_internal,
          )) {
        1;
      } else {
        0;
      }
    | Validate_status =>
      if (_getProgramParameter(
            ~context,
            ~program,
            ~paramName=validateStatus(~context),
            Validate_status_internal,
          )) {
        1;
      } else {
        0;
      }
    };
  [@bs.send]
  external _getShaderParameter :
    (
      ~context: contextT,
      ~shader: shaderT,
      ~paramName: int,
      [@bs.ignore] shaderParamsInternalT('a)
    ) =>
    'a =
    "getShaderParameter";
  let getShaderParameter = (~context, ~shader, ~paramName) =>
    switch (paramName) {
    | Shader_delete_status =>
      if (_getShaderParameter(
            ~context,
            ~shader,
            ~paramName=deleteStatus(~context),
            Shader_delete_status_internal,
          )) {
        1;
      } else {
        0;
      }
    | Compile_status =>
      if (_getShaderParameter(
            ~context,
            ~shader,
            ~paramName=compileStatus(~context),
            Compile_status_internal,
          )) {
        1;
      } else {
        0;
      }
    | Shader_type =>
      _getShaderParameter(
        ~context,
        ~shader,
        ~paramName=shaderType(~context),
        Shader_type_internal,
      )
    };
  [@bs.send]
  external getShaderInfoLog : (~context: contextT, shaderT) => string =
    "getShaderInfoLog";
  [@bs.send]
  external getProgramInfoLog : (~context: contextT, programT) => string =
    "getProgramInfoLog";
  [@bs.send]
  external getShaderSource : (~context: contextT, shaderT) => string =
    "getShaderSource";
  [@bs.send]
  external drawArrays :
    (~context: contextT, ~mode: int, ~first: int, ~count: int) => unit =
    "drawArrays";
  [@bs.send]
  external drawElements :
    (~context: contextT, ~mode: int, ~count: int, ~type_: int, ~offset: int) =>
    unit =
    "drawElements";
  [@bs.send]
  external drawElementsInstanced :
    (
      ~context: contextT,
      ~mode: int,
      ~count: int,
      ~type_: int,
      ~indices: int,
      ~primcount: int
    ) =>
    unit =
    "drawElementsInstanced";
};
