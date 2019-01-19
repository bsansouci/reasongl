/* Hack to handle wayland https://github.com/Schmavery/reprocessing/issues/74 */
try (
  if (Sys.unix && Sys.getenv("WAYLAND_DISPLAY") != "") {
    Unix.putenv("SDL_VIDEODRIVER", "wayland");
  }
) {
| Not_found => ()
};

 
module Str = Str;

module Bigarray = Bigarray;

module Unix = Unix;

module Sdl = Tsdl_new;

let (>>=) = (t, f) =>
  switch t {
  | 0 => f()
  | _ => failwith @@ Sdl.error()
  };

let create_window = (~gl as (maj, min)) => {
  let w_atts = Sdl.Window.(opengl + resizable + allow_highdpi);
  let w_title = Printf.sprintf("OpenGL %d.%d (core profile)", maj, min);
  let set = (a, v) => Sdl.Gl.gl_set_attribute(~attr=a, ~value=v);
  set(Sdl.Gl.context_profile_mask, Sdl.Gl.context_profile_compatibility)
  >>= (
    () =>
      set(Sdl.Gl.context_major_version, maj)
      >>= (
        () =>
          set(Sdl.Gl.context_minor_version, min)
          >>= (
            () =>
              set(Sdl.Gl.doublebuffer, 1)
              >>= (
                () =>
                  set(Sdl.Gl.multisamplebuffers, 1)
                  >>= (
                    () =>
                      set(Sdl.Gl.multisamplesamples, 8)
                      >>= (
                        () =>
                          Sdl.create_window(
                            ~title=w_title,
                            ~x=Sdl.Window.pos_centered,
                            ~y=Sdl.Window.pos_centered,
                            ~w=640,
                            ~h=480,
                            ~flags=w_atts
                          )
                      )
                  )
              )
          )
      )
  )
};

module Gl: ReasonglInterface.Gl.t = {
  module Gl = Tgls_new;
  let target = "native";
  type contextT = Sdl.glContextT;
  module type FileT = {type t; let readFile: (~filename: string, ~cb: string => unit) => unit;};
  module File = {
    type t;
    let readFile = (~filename, ~cb) => {
      let ic = open_in(filename);
      let try_read = () =>
        switch (input_line(ic)) {
        | exception End_of_file => None
        | x => Some(x)
        };
      let rec loop = (acc) =>
        switch (try_read()) {
        | Some(s) => loop([s, ...acc])
        | None =>
          close_in(ic);
          List.rev(acc)
        };
      let text = loop([]) |> String.concat(String.make(1, '\n'));
      cb(text)
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
    type t = Sdl.windowT;
    let getWidth = (window: t) => {
      let (width, _) = Sdl.get_window_size(window);
      width
    };
    let getHeight = (window: t) => {
      let (_, height) = Sdl.get_window_size(window);
      height
    };
    let getPixelWidth = (window: t) => {
      let (width, _) = Sdl.get_drawable_size(window);
      width
    };
    let getPixelHeight = (window: t) => {
      let (_, height) = Sdl.get_drawable_size(window);
      height
    };
    /* 72 is the magic number */
    let getPixelScale = (window: t) => {
      let {Sdl.hdpi} = Sdl.get_window_dpi(window);
      hdpi /. 72.
    };

    /***
     * We create an OpenGL context at 2.1 because... it seems to be the only one that we can request that
     * osx will give us and one that has an API comparable to OpenGL ES 2.0 which is what WebGL uses.
     */
    let init = (~screen=?, ~argv as _) => {
      /* Screen is ignored for now, we always just create a new window. */
      if (Sdl.Init.init(Sdl.Init.video lor Sdl.Init.audio) != 0) {
        failwith @@ Sdl.error()
      };
      create_window(~gl=(2, 1))
    };
    let setWindowSize = (~window: t, ~width, ~height) =>
      Sdl.set_window_size(window, ~width, ~height);
    let getContext = (window: t) : contextT => {
      let ctx = Sdl.gl_create_context(window);
      Gl.gladLoadGL();
      let e = Sdl.gl_make_current(window, ctx);
      if (e != 0) {
        failwith @@ Sdl.error()
      };
      ctx
    };
    let setWindowTitle = (~window: t, ~title) =>
      Sdl.set_window_title(window, title);
  };
  module type AudioT = {
    type t;
    let loadSound: (Sdl.windowT, string, t => unit) => unit;
    let playSound: (Sdl.windowT, t, ~volume: float, ~loop: bool) => unit;
  };
  module Audio = {
    type t = Sdl.soundT;
    let loadSound = (w, s, cb) => cb(Sdl.load_audio(w, s));
    let playSound = Sdl.play_audio;
  };
  module Events = Events;
  type mouseButtonEventT =
    (~button: Events.buttonStateT, ~state: Events.stateT, ~x: int, ~y: int) => unit;
  [@noalloc] external usleep : int => unit = "reasongl_usleep";

  /*** See Gl.re for explanation. **/
  let render =
      (
        ~window: Window.t,
        ~mouseDown: option(mouseButtonEventT)=?,
        ~mouseUp: option(mouseButtonEventT)=?,
        ~mouseMove: option(((~x: int, ~y: int) => unit))=?,
        ~keyDown: option(((~keycode: Events.keycodeT, ~repeat: bool) => unit))=?,
        ~keyUp: option(((~keycode: Events.keycodeT) => unit))=?,
        ~windowResize: option((unit => unit))=?,
        ~displayFunc: float => unit,
        ()
      ) => {
    let checkEvents = () : bool => {
      open Sdl.Event;
      let shouldQuit = ref(false);
      let shouldPoll = ref(true);
      while (shouldPoll^) {
        switch (Sdl.Event.poll_event()) {
        | None => shouldPoll := false
        | Some(e) =>
          let eventType = e.typ;
          if (eventType == Sdl.Event.quit) {
            shouldQuit := true
          } else if (eventType == Sdl.Event.mousebuttondown) {
            switch mouseDown {
            | None => ()
            | Some(cb) =>
              let x = e.mouse_button_x;
              let y = e.mouse_button_y;
              let button =
                switch e.mouse_button_button {
                | 1 => Events.LeftButton
                | 2 => Events.MiddleButton
                | 3 => Events.RightButton
                | _ => failwith("Button not supported")
                };
              cb(~button, ~state=Events.MouseDown, ~x, ~y);
              ()
            }
          } else if (eventType == Sdl.Event.mousebuttonup) {
            switch mouseUp {
            | None => ()
            | Some(cb) =>
              let x = e.mouse_button_x;
              let y = e.mouse_button_y;
              let button =
                switch e.mouse_button_button {
                | 1 => Events.LeftButton
                | 2 => Events.MiddleButton
                | 3 => Events.RightButton
                | _ => failwith("Button not supported")
                };
              cb(~button, ~state=Events.MouseUp, ~x, ~y);
              ()
            }
          } else if (eventType == Sdl.Event.mousemotion) {
            switch mouseMove {
            | None => ()
            | Some(cb) =>
              let x = e.mouse_motion_x;
              let y = e.mouse_motion_y;
              cb(~x, ~y);
              ()
            }
          } else if (eventType == Sdl.Event.windowevent) {
            switch windowResize {
            | None => ()
            | Some(cb) =>
              if (e.window_event_enum == Sdl.Event.window_resized
                  || e.window_event_enum == Sdl.Event.window_maximized
                  || e.window_event_enum == Sdl.Event.window_restored) {
                cb()
              }
            }
          } else if (eventType == Sdl.Event.keydown) {
            switch keyDown {
            | None => ()
            | Some(cb) =>
              let (keycode, repeat) = (e.keyboard_keycode, e.keyboard_repeat);
              cb(~keycode=Events.keycodeMap(keycode), ~repeat=repeat === 1)
            }
          } else if (eventType == Sdl.Event.keyup) {
            switch keyUp {
            | None => ()
            | Some(cb) =>
              let keycode = e.keyboard_keycode;
              cb(~keycode=Events.keycodeMap(keycode))
            }
          }
        }
      };
      shouldQuit^
    };
    let timeSinceLastDraw = ref(Sdl.get_performance_counter());
    let oneFrame = 1000. /. 60.;
    let shouldQuit = ref(false);
    let rec tick = () => {
      let time = Sdl.get_performance_counter();
      let diff = Sdl.get_time_diff(timeSinceLastDraw^, time);
      if (diff > oneFrame) {
        timeSinceLastDraw := time;
        shouldQuit := shouldQuit^ || checkEvents();
        displayFunc(diff);
        Sdl.gl_swap_window(window)
      };
      if (! shouldQuit^) {
        /* only sleep if we had extra time this frame and we have 3ms to spare or more because of
           the lack of precision of sleep (and ocaml overhead) */
        let timeToSleep = mod_float(oneFrame -. diff, oneFrame) -. 2.;
        if (timeToSleep > 1.) {
          usleep(int_of_float(1000. *. timeToSleep))
        };
        tick()
      }
    };
    tick();
    /* No way to pause this externally, because we're the main loop */
    (_ignored) => false
  };
  type programT = Gl.programT;
  type shaderT = Gl.shaderT;
  let clearColor = (~context as _, ~r, ~g, ~b, ~a) =>
    Gl.clearColor(~red=r, ~green=g, ~blue=b, ~alpha=a);
  let createProgram = (~context as _) : programT => Gl.createProgram();
  let createShader = (~context as _, shaderType) : shaderT => Gl.createShader(shaderType);
  let attachShader = (~context as _, ~program, ~shader) => Gl.attachShader(~program, ~shader);
  let deleteShader = (~context as _, shader) => Gl.deleteShader(shader);
  let shaderSource = (~context as _, ~shader, ~source) =>
    Gl.shaderSource(shader, [|"#version 120 \n", source|]);
  let compileShader = (~context as _, shader) => Gl.compileShader(shader);
  let linkProgram = (~context as _, program) => Gl.linkProgram(program);
  let useProgram = (~context as _, program) => Gl.useProgram(program);
  type bufferT = Gl.bufferT;
  type attributeT = Gl.attribT;
  type uniformT = Gl.uniformT;
  let createBuffer = (~context as _) => Gl.genBuffer();
  let bindBuffer = (~context as _, ~target, ~buffer) => Gl.bindBuffer(~target, ~buffer);
  type textureT = Gl.textureT;
  let createTexture = (~context as _) => Gl.genTexture();
  let activeTexture = (~context as _, target) => Gl.activeTexture(target);
  let bindTexture = (~context as _, ~target, ~texture) => Gl.bindTexture(~target, ~texture);
  let texParameteri = (~context as _, ~target, ~pname, ~param) =>
    Gl.texParameteri(~target, ~pname, ~param);
  type framebufferT = Gl.framebufferT;
  let createFramebuffer = (~context as _) => Gl.genFramebuffer();
  let bindFramebuffer = (~context as _, ~target, ~framebuffer) => Gl.bindFramebuffer(target, framebuffer);
  let bindDefaultFramebuffer = (~context as _, ~target) => Gl.bindDefaultFramebuffer(target);
  let framebufferTexture2D = (~context as _, ~target, ~attachment, ~texTarget, ~texture) =>
    Gl.framebufferTexture2D(~target, ~attachment, ~texTarget, ~texture, ~level=0);
  let enable = (~context as _, i) => Gl.enable(i);
  let disable = (~context as _, i) => Gl.disable(i);
  let blendFunc = (~context as _, a, b) => Gl.blendFunc(~sfactor=a, ~dfactor=b);
  let readPixels_RGBA = (~context as _, ~x, ~y, ~width, ~height) =>
    Gl.readPixels_RGBA(~x, ~y, ~width, ~height);
  type loadOptionT =
    | LoadAuto
    | LoadL
    | LoadLA
    | LoadRGB
    | LoadRGBA;

  /* We do this because Tgls can load images through SOIL */
  type imageT = Gl.imageT = {
    width: int,
    height: int,
    channels: int,
    data: Bigarray.Array1.t(int, Bigarray.int8_unsigned_elt, Bigarray.c_layout)
  };
  let getImageWidth = (image) => image.width;
  let getImageHeight = (image) => image.height;

  let loadImage = (~filename, ~loadOption=LoadAuto, ~callback: option(imageT) => unit, ()) =>
    switch loadOption {
    | LoadAuto => callback(Gl.soilLoadImage(~filename, ~loadOption=0))
    | LoadL => callback(Gl.soilLoadImage(~filename, ~loadOption=1))
    | LoadLA => callback(Gl.soilLoadImage(~filename, ~loadOption=2))
    | LoadRGB => callback(Gl.soilLoadImage(~filename, ~loadOption=3))
    | LoadRGBA => callback(Gl.soilLoadImage(~filename, ~loadOption=4))
    };

  let loadImageFromMemory = (~data, ~loadOption=LoadAuto, ~callback: option(imageT) => unit, ()) =>
    switch loadOption {
    | LoadAuto => callback(Gl.soilLoadImageFromMemory(~data, ~loadOption=0))
    | LoadL => callback(Gl.soilLoadImageFromMemory(~data, ~loadOption=1))
    | LoadLA => callback(Gl.soilLoadImageFromMemory(~data, ~loadOption=2))
    | LoadRGB => callback(Gl.soilLoadImageFromMemory(~data, ~loadOption=3))
    | LoadRGBA => callback(Gl.soilLoadImageFromMemory(~data, ~loadOption=4))
    };
  let texImage2D_RGBA = (~context as _, ~target, ~level, ~width, ~height, ~border, ~data) =>
    Gl.texImage2D_RGBA(~target, ~level, ~width, ~height, ~border, ~data);
  let texImage2D_null = (~context as _, ~target, ~level, ~width, ~height) =>
    Gl.texImage2D_null(~target, ~level, ~width, ~height, ~border=0);
  let texImage2DWithImage = (~context, ~target, ~level, ~image) =>
    texImage2D_RGBA(
      ~context,
      ~target,
      ~level,
      ~width=image.width,
      ~height=image.height,
      ~border=0,
      ~data=image.data
    );
  let uniform1i = (~context as _, ~location, ~value) => Gl.uniform1i(~location, ~value);
  let uniform1f = (~context as _, ~location, ~value) => Gl.uniform1f(~location, ~value);
  let uniform2f = (~context as _, ~location, ~v1, ~v2) => Gl.uniform2f(~location, ~v1, ~v2);
  let uniform3f = (~context as _, ~location, ~v1, ~v2, ~v3) =>
    Gl.uniform3f(~location, ~v1, ~v2, ~v3);
  let uniform4f = (~context as _, ~location, ~v1, ~v2, ~v3, ~v4) =>
    Gl.uniform4f(~location, ~v1, ~v2, ~v3, ~v4);
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
    let unsafe_blit: (t('a, 'b), t('a, 'b), ~offset: int, ~numOfBytes: int) => unit;
    let get: (t('a, 'b), int) => 'a;
    let unsafe_get: (t('a, 'b), int) => 'a;
    let set: (t('a, 'b), int, 'a) => unit;
    let unsafe_set: (t('a, 'b), int, 'a) => unit;
    let sub: (t('a, 'b), ~offset: int, ~len: int) => t('a, 'b);
  };
  module Bigarray = {
    type t('a, 'b) = Bigarray.Array1.t('a, 'b, Bigarray.c_layout);
    type float64_elt = Bigarray.float64_elt;
    type float32_elt = Bigarray.float32_elt;
    type int16_unsigned_elt = Bigarray.int16_unsigned_elt;
    type int16_signed_elt = Bigarray.int16_signed_elt;
    type int8_unsigned_elt = Bigarray.int8_unsigned_elt;
    type int8_signed_elt = Bigarray.int8_signed_elt;
    type int_elt = Bigarray.int_elt;
    type int32_elt = Bigarray.int32_elt;
    type int64_elt = Bigarray.int64_elt;
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
      switch kind {
      | Float64 => Bigarray.Array1.create(Bigarray.Float64, Bigarray.c_layout, size)
      | Float32 => Bigarray.Array1.create(Bigarray.Float32, Bigarray.c_layout, size)
      | Int16 => Bigarray.Array1.create(Bigarray.Int16_signed, Bigarray.c_layout, size)
      | Uint16 => Bigarray.Array1.create(Bigarray.Int16_unsigned, Bigarray.c_layout, size)
      | Int8 => Bigarray.Array1.create(Bigarray.Int8_signed, Bigarray.c_layout, size)
      | Uint8 => Bigarray.Array1.create(Bigarray.Int8_unsigned, Bigarray.c_layout, size)
      | Char => Bigarray.Array1.create(Bigarray.Char, Bigarray.c_layout, size)
      | Int => Bigarray.Array1.create(Bigarray.Int, Bigarray.c_layout, size)
      | Int64 => Bigarray.Array1.create(Bigarray.Int64, Bigarray.c_layout, size)
      | Int32 => Bigarray.Array1.create(Bigarray.Int32, Bigarray.c_layout, size)
      };
    let of_array = (type a, type b, kind: kind(a, b), arr: array(a)) : t(a, b) =>
      switch kind {
      | Float64 => Bigarray.Array1.of_array(Bigarray.Float64, Bigarray.c_layout, arr)
      | Float32 => Bigarray.Array1.of_array(Bigarray.Float32, Bigarray.c_layout, arr)
      | Int16 => Bigarray.Array1.of_array(Bigarray.Int16_signed, Bigarray.c_layout, arr)
      | Uint16 => Bigarray.Array1.of_array(Bigarray.Int16_unsigned, Bigarray.c_layout, arr)
      | Int8 => Bigarray.Array1.of_array(Bigarray.Int8_signed, Bigarray.c_layout, arr)
      | Uint8 => Bigarray.Array1.of_array(Bigarray.Int8_unsigned, Bigarray.c_layout, arr)
      | Char => Bigarray.Array1.of_array(Bigarray.Char, Bigarray.c_layout, arr)
      | Int => Bigarray.Array1.of_array(Bigarray.Int, Bigarray.c_layout, arr)
      | Int64 => Bigarray.Array1.of_array(Bigarray.Int64, Bigarray.c_layout, arr)
      | Int32 => Bigarray.Array1.of_array(Bigarray.Int32, Bigarray.c_layout, arr)
      };
    let dim = Bigarray.Array1.dim;
    let blit = Bigarray.Array1.blit;
    /* "What is going on here" you may ask.
       Well we kinda sorta profiled the app and noticed that ba_caml_XYZ was called a lot.
       This is an attempt at reducing the cost of those calls. We implemented our own C blit (
       which is just memcpy)
                Ben - August 28th 2017
          */
    [@noalloc]
    external unsafe_blit :
      (
        Bigarray.Array1.t('a, 'b, 'c),
        Bigarray.Array1.t('a, 'b, 'c),
        ~offset: int,
        ~numOfBytes: int
      ) =>
      unit =
      "bigarray_unsafe_blit";
    let get = Bigarray.Array1.get;
    let unsafe_get = Bigarray.Array1.unsafe_get;
    let set = Bigarray.Array1.set;
    let unsafe_set = Bigarray.Array1.unsafe_set;
    let sub = (type a, type b, arr: t(a, b), ~offset, ~len) : t(a, b) =>
      Bigarray.Array1.sub(arr, offset, len);
  };
  let texSubImage2D =
      (
        ~context as _,
        ~target,
        ~level,
        ~xoffset,
        ~yoffset,
        ~width,
        ~height,
        ~format,
        ~type_,
        ~pixels: Bigarray.t('a, 'b)
      ) =>
    Gl.texSubImage2D(
      ~target,
      ~level,
      ~xoffset,
      ~yoffset,
      ~width,
      ~height,
      ~format,
      ~type_,
      ~pixels
    );
  let bufferData = (~context as _, ~target, ~data: Bigarray.t('a, 'b), ~usage) =>
    Gl.bufferData(~target, ~data, ~usage);
  let viewport = (~context as _, ~x, ~y, ~width, ~height) => Gl.viewport(~x, ~y, ~width, ~height);
  let clear = (~context as _, ~mask) => Gl.clear(mask);
  let getUniformLocation = (~context as _, ~program: programT, ~name) : uniformT =>
    Gl.getUniformLocation(~program, ~name);
  let getAttribLocation = (~context as _, ~program: programT, ~name) : attributeT =>
    Gl.getAttribLocation(~program, ~name);
  let enableVertexAttribArray = (~context as _, ~attribute) =>
    Gl.enableVertexAttribArray(attribute);
  let vertexAttribPointer =
      (~context as _, ~attribute, ~size, ~type_, ~normalize, ~stride, ~offset) =>
    /* For now `offset` is only going to be an offset (limited by the webgl API?). */
    Gl.vertexAttribPointer(~index=attribute, ~size, ~typ=type_, ~normalize, ~stride, ~offset);
  let vertexAttribDivisor = (~context as _, ~attribute, ~divisor) =>
    Gl.vertexAttribDivisor(~attribute, ~divisor);
  module type Mat4T = {
    type t;
    let to_array: t => array(float);
    let create: unit => t;
    let identity: (~out: t) => unit;
    let translate: (~out: t, ~matrix: t, ~vec: array(float)) => unit;
    let scale: (~out: t, ~matrix: t, ~vec: array(float)) => unit;
    let rotate: (~out: t, ~matrix: t, ~rad: float, ~vec: array(float)) => unit;
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
      (~out: t, ~fovy: float, ~aspect: float, ~near: float, ~far: float) =>
      unit;
    let lookAt:
      (
        ~out: t,
        ~eye: array(float),
        ~center: array(float),
        ~up: array(float)
      ) =>
      unit;
  };
  module Mat4: Mat4T = {
    type t = array(float);
    let to_array = (a) => a;
    let epsilon = 0.00001;
    let create = () => [|
      1.0,
      0.0,
      0.0,
      0.0,
      0.0,
      1.0,
      0.0,
      0.0,
      0.0,
      0.0,
      1.0,
      0.0,
      0.0,
      0.0,
      0.0,
      1.0
    |];
    let identity = (~out: t) => {
      out[0] = 1.0;
      out[1] = 0.0;
      out[2] = 0.0;
      out[3] = 0.0;
      out[4] = 0.0;
      out[5] = 1.0;
      out[6] = 0.0;
      out[7] = 0.0;
      out[8] = 0.0;
      out[9] = 0.0;
      out[10] = 1.0;
      out[11] = 0.0;
      out[12] = 0.0;
      out[13] = 0.0;
      out[14] = 0.0;
      out[15] = 1.0
    };
    let translate = (~out: t, ~matrix: t, ~vec: array(float)) => {
      let x = vec[0];
      let y = vec[1];
      let z = vec[2];
      if (matrix === out) {
        out[12] = matrix[0] *. x +. matrix[4] *. y +. matrix[8] *. z +. matrix[12];
        out[13] = matrix[1] *. x +. matrix[5] *. y +. matrix[9] *. z +. matrix[13];
        out[14] = matrix[2] *. x +. matrix[6] *. y +. matrix[10] *. z +. matrix[14];
        out[15] = matrix[3] *. x +. matrix[7] *. y +. matrix[11] *. z +. matrix[15]
      } else {
        let a00 = matrix[0];
        let a01 = matrix[1];
        let a02 = matrix[2];
        let a03 = matrix[3];
        let a10 = matrix[4];
        let a11 = matrix[5];
        let a12 = matrix[6];
        let a13 = matrix[7];
        let a20 = matrix[8];
        let a21 = matrix[9];
        let a22 = matrix[10];
        let a23 = matrix[11];
        out[0] = a00;
        out[1] = a01;
        out[2] = a02;
        out[3] = a03;
        out[4] = a10;
        out[5] = a11;
        out[6] = a12;
        out[7] = a13;
        out[8] = a20;
        out[9] = a21;
        out[10] = a22;
        out[11] = a23;
        out[12] = a00 *. x +. a10 *. y +. a20 *. z +. matrix[12];
        out[13] = a01 *. x +. a11 *. y +. a21 *. z +. matrix[13];
        out[14] = a02 *. x +. a12 *. y +. a22 *. z +. matrix[14];
        out[15] = a03 *. x +. a13 *. y +. a23 *. z +. matrix[15]
      }
    };
    let scale = (~out: t, ~matrix: t, ~vec: array(float)) => {
      let x = vec[0];
      let y = vec[1];
      let z = vec[2];
      out[0] = matrix[0] *. x;
      out[1] = matrix[1] *. x;
      out[2] = matrix[2] *. x;
      out[3] = matrix[3] *. x;
      out[4] = matrix[4] *. y;
      out[5] = matrix[5] *. y;
      out[6] = matrix[6] *. y;
      out[7] = matrix[7] *. y;
      out[8] = matrix[8] *. z;
      out[9] = matrix[9] *. z;
      out[10] = matrix[10] *. z;
      out[11] = matrix[11] *. z;
      out[12] = matrix[12];
      out[13] = matrix[13];
      out[14] = matrix[14];
      out[15] = matrix[15];
    };
    let rotate = (~out: t, ~matrix: t, ~rad: float, ~vec: array(float)) => {
      let x = vec[0];
      let y = vec[1];
      let z = vec[2];
      let len = sqrt(x *. x +. y *. y +. z *. z);
      if (abs_float(len) >= epsilon) {
        let len = 1. /. len;
        let x = x *. len;
        let y = y *. len;
        let z = z *. len;
        let s = sin(rad);
        let c = cos(rad);
        let t = 1. -. c;
        let a00 = matrix[0];
        let a01 = matrix[1];
        let a02 = matrix[2];
        let a03 = matrix[3];
        let a10 = matrix[4];
        let a11 = matrix[5];
        let a12 = matrix[6];
        let a13 = matrix[7];
        let a20 = matrix[8];
        let a21 = matrix[9];
        let a22 = matrix[10];
        let a23 = matrix[11];
        let b00 = x *. x *. t +. c;
        let b01 = y *. x *. t +. z *. s;
        let b02 = z *. x *. t -. y *. s;
        let b10 = x *. y *. t -. z *. s;
        let b11 = y *. y *. t +. c;
        let b12 = z *. y *. t +. x *. s;
        let b20 = x *. z *. t +. y *. s;
        let b21 = y *. z *. t -. x *. s;
        let b22 = z *. z *. t +. c;
        out[0] = a00 *. b00 +. a10 *. b01 +. a20 *. b02;
        out[1] = a01 *. b00 +. a11 *. b01 +. a21 *. b02;
        out[2] = a02 *. b00 +. a12 *. b01 +. a22 *. b02;
        out[3] = a03 *. b00 +. a13 *. b01 +. a23 *. b02;
        out[4] = a00 *. b10 +. a10 *. b11 +. a20 *. b12;
        out[5] = a01 *. b10 +. a11 *. b11 +. a21 *. b12;
        out[6] = a02 *. b10 +. a12 *. b11 +. a22 *. b12;
        out[7] = a03 *. b10 +. a13 *. b11 +. a23 *. b12;
        out[8] = a00 *. b20 +. a10 *. b21 +. a20 *. b22;
        out[9] = a01 *. b20 +. a11 *. b21 +. a21 *. b22;
        out[10] = a02 *. b20 +. a12 *. b21 +. a22 *. b22;
        out[11] = a03 *. b20 +. a13 *. b21 +. a23 *. b22;
      };
      if (matrix !== out) {
        out[12] = matrix[12];
        out[13] = matrix[13];
        out[14] = matrix[14];
        out[15] = matrix[15];
      };
    };
    let ortho =
        (
          ~out: t,
          ~left: float,
          ~right: float,
          ~bottom: float,
          ~top: float,
          ~near: float,
          ~far: float
        ) => {
      let lr = 1. /. (left -. right);
      let bt = 1. /. (bottom -. top);
      let nf = 1. /. (near -. far);
      out[0] = (-2.) *. lr;
      out[1] = 0.;
      out[2] = 0.;
      out[3] = 0.;
      out[4] = 0.;
      out[5] = (-2.) *. bt;
      out[6] = 0.;
      out[7] = 0.;
      out[8] = 0.;
      out[9] = 0.;
      out[10] = 2. *. nf;
      out[11] = 0.;
      out[12] = (left +. right) *. lr;
      out[13] = (top +. bottom) *. bt;
      out[14] = (far +. near) *. nf;
      out[15] = 1.;
    };
    let perspective =
        (~out: t, ~fovy: float, ~aspect: float, ~near: float, ~far: float) => {
      let f = 1.0 /. tan(fovy /. 2.);
      out[0] = f /. aspect;
      out[1] = 0.;
      out[2] = 0.;
      out[3] = 0.;
      out[4] = 0.;
      out[5] = f;
      out[6] = 0.;
      out[7] = 0.;
      out[8] = 0.;
      out[9] = 0.;
      out[11] = (-1.);
      out[12] = 0.;
      out[13] = 0.;
      out[15] = 0.;
      if (far !== infinity) {
        let nf = 1. /. (near -. far);
        out[10] = (far +. near) *. nf;
        out[14] = 2. *. far *. near *. nf;
      } else {
        out[10] = (-1.);
        out[14] = (-2.) *. near;
      };
    };
    let lookAt = (~out, ~eye, ~center, ~up) => {
      let eyex = eye[0];
      let eyey = eye[1];
      let eyez = eye[2];
      let centerx = center[0];
      let centery = center[1];
      let centerz = center[2];
      let upx = up[0];
      let upy = up[1];
      let upz = up[2];
      if (abs_float(eyex -. centerx) < epsilon
          && abs_float(eyey -. centery) < epsilon
          && abs_float(eyez -. centerz) < epsilon) {
        identity(~out);
      } else {
        let z0 = eyex -. centerx;
        let z1 = eyey -. centery;
        let z2 = eyez -. centerz;
        let len = 1. /. sqrt(z0 *. z0 +. z1 *. z1 +. z2 *. z2);
        let z0 = z0 *. len;
        let z1 = z1 *. len;
        let z2 = z2 *. len;
        let x0 = upy *. z2 -. upz *. z1;
        let x1 = upz *. z0 -. upx *. z2;
        let x2 = upx *. z1 -. upy *. z0;
        let len = sqrt(x0 *. x0 +. x1 *. x1 +. x2 *. x2);
        let (len, x0, x1, x2) =
          if (len == 0.) {
            (len, 0., 0., 0.);
          } else {
            let len = 1. /. len;
            (len, x0 *. len, x1 *. len, x2 *. len);
          };
        let y0 = z1 *. x2 -. z2 *. x1;
        let y1 = z2 *. x0 -. z0 *. x2;
        let y2 = z0 *. x1 -. z1 *. x0;
        let len = sqrt(y0 *. y0 +. y1 *. y1 +. y2 *. y2);
        let (len, y0, y1, y2) =
          if (len == 0.) {
            (len, 0., 0., 0.);
          } else {
            let len = 1. /. len;
            (len, y0 *. len, y1 *. len, y2 *. len);
          };
        out[0] = x0;
        out[1] = y0;
        out[2] = z0;
        out[3] = 0.;
        out[4] = x1;
        out[5] = y1;
        out[6] = z1;
        out[7] = 0.;
        out[8] = x2;
        out[9] = y2;
        out[10] = z2;
        out[11] = 0.;
        out[12] = -. (x0 *. eyex +. x1 *. eyey +. x2 *. eyez);
        out[13] = -. (y0 *. eyex +. y1 *. eyey +. y2 *. eyez);
        out[14] = -. (z0 *. eyex +. z1 *. eyey +. z2 *. eyez);
        out[15] = 1.;
      };
    };
  };

  /*** count = 1 for now https://www.opengl.org/sdk/docs/man/html/glUniform.xhtml
   * and transform = false because "Must be GL_FALSE"...
   */
  let uniformMatrix4fv = (~context as _, ~location, ~value) =>
    Gl.uniformMatrix4fv(~location, ~transpose=false, ~value=Mat4.to_array(value));
  type shaderParamsT =
    | Shader_delete_status
    | Compile_status
    | Shader_type;
  type programParamsT =
    | Program_delete_status
    | Link_status
    | Validate_status;

  /***
   * We use Bigarray here as some sort of pointer.
   */
  let _getProgramParameter = (~context as _, ~program: programT, ~paramName) =>
    Gl.getProgramiv(~program, ~pname=paramName);
  let getProgramParameter = (~context, ~program: programT, ~paramName) =>
    switch paramName {
    | Program_delete_status =>
      _getProgramParameter(~context, ~program, ~paramName=Gl.gl_delete_status)
    | Link_status => _getProgramParameter(~context, ~program, ~paramName=Gl.gl_link_status)
    | Validate_status => _getProgramParameter(~context, ~program, ~paramName=Gl.gl_validate_status)
    };
  let _getShaderParameter = (~context as _, ~shader, ~paramName) =>
    Gl.getShaderiv(~shader, ~pname=paramName);
  let getShaderParameter = (~context, ~shader, ~paramName) =>
    switch paramName {
    | Shader_delete_status =>
      _getShaderParameter(~context, ~shader, ~paramName=Gl.gl_delete_status)
    | Compile_status => _getShaderParameter(~context, ~shader, ~paramName=Gl.gl_compile_status)
    | Shader_type => _getShaderParameter(~context, ~shader, ~paramName=Gl.gl_shader_type)
    };
  let getShaderInfoLog = (~context as _, shader) => Gl.getShaderInfoLog(shader);
  let getProgramInfoLog = (~context as _, program) => Gl.getProgramInfoLog(program);
  let getShaderSource = (~context as _, shader) => Gl.getShaderSource(shader);
  let drawArrays = (~context as _, ~mode, ~first, ~count) => Gl.drawArrays(~mode, ~first, ~count);
  let drawElements = (~context as _, ~mode, ~count, ~type_, ~offset) =>
    Gl.drawElements(~mode, ~count, ~typ=type_, ~offset);
  let drawElementsInstanced = (~context as _, ~mode, ~count, ~type_, ~indices, ~primcount) =>
    Gl.drawElementsInstanced(~mode, ~count, ~type_, ~indices, ~primcount);
};
