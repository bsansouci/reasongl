/*
 * vim: set ft=rust:
 * vim: set ft=reason:
 */
module Str = Str;

module Bigarray = Bigarray;

module Unix = Unix;

open Tsdl_packed.Tsdl;

open Tsdl_packed.Result;

open Tgl3_packed.Tgl3;

let (>>=) x f =>
  switch x {
  | Ok v => f v
  | Error _ as e => e
  };

let create_window gl::(maj, min) => {
  let w_atts = Sdl.Window.(opengl + resizable);
  let w_title = Printf.sprintf "OpenGL %d.%d (core profile)" maj min;
  let set a v => Sdl.gl_set_attribute a v;
  set Sdl.Gl.context_profile_mask Sdl.Gl.context_profile_compatibility >>= (
    fun () =>
      set Sdl.Gl.context_major_version maj >>= (
        fun () =>
          set Sdl.Gl.context_minor_version min >>= (
            fun () =>
              set Sdl.Gl.doublebuffer 1 >>= (
                fun () =>
                  Sdl.create_window w::640 h::480 w_title w_atts >>= (
                    fun win => Ok win
                  )
              )
          )
      )
  )
};

module Gl: ReasonglInterface.Gl.t = {
  let target = "native";
  type contextT = Sdl.gl_context;
  module type FileT = {
    type t;
    let readFile: filename::string => cb::(string => unit) => unit;
  };
  module File = {
    type t;
    let readFile ::filename ::cb => {
      let ic = open_in filename;
      let try_read () =>
        switch (input_line ic) {
        | exception End_of_file => None
        | x => Some x
        };
      let rec loop acc =>
        switch (try_read ()) {
        | Some s => loop [s, ...acc]
        | None =>
          close_in ic;
          List.rev acc
        };
      let text = loop [] |> String.concat (String.make 1 '\n');
      cb text
    };
  };
  module type WindowT = {
    type t;
    let getWidth: t => int;
    let getHeight: t => int;
    let init: argv::array string => t;
    let setWindowSize: window::t => width::int => height::int => unit;
    let initDisplayMode: window::t => double_buffer::bool => unit => unit;
    let getContext: t => contextT;
  };
  module Window = {
    type t = Sdl.window;
    let getWidth (window: t) => {
      let (width, _) = Sdl.get_window_size window;
      width
    };
    let getHeight (window: t) => {
      let (_, height) = Sdl.get_window_size window;
      height
    };

    /**
     * We create an OpenGL context at 2.1 because... it seems to be the only one that we can request that
     * osx will give us and one that has an API comparable to OpenGL ES 2.0 which is what WebGL uses.
     */
    let init argv::_ =>
      switch (
        Sdl.init Sdl.Init.video >>= (
          fun () => create_window gl::(2, 1) >>= (fun win => Ok win)
        )
      ) {
      | Ok win => win
      | Error (`Msg e) => failwith e
      };
    let setWindowSize window::(window: t) ::width ::height =>
      Sdl.set_window_size window width height;
    let initDisplayMode ::window double_buffer::_ () => ();
    let getContext (window: t) :contextT =>
      switch (
        Sdl.gl_create_context window >>= (
          fun ctx => Sdl.gl_make_current window ctx >>= (fun () => Ok ctx)
        )
      ) {
      | Ok ctx => ctx
      | Error (`Msg e) => failwith e
      };
  };
  module Events = Events;
  type mouseButtonEventT =
    button::Events.buttonStateT =>
    state::Events.stateT =>
    x::int =>
    y::int =>
    unit;

  /** See Gl.re for explanation. **/
  let render
      window::(window: Window.t)
      mouseDown::(mouseDown: option mouseButtonEventT)=?
      mouseUp::(mouseUp: option mouseButtonEventT)=?
      mouseMove::(mouseMove: option (x::int => y::int => unit))=?
      keyDown::
        (keyDown: option (keycode::Events.keycodeT => repeat::bool => unit))=?
      keyUp::(keyUp: option (keycode::Events.keycodeT => unit))=?
      windowResize::(windowResize: option (unit => unit))=?
      displayFunc::(displayFunc: float => unit)
      () => {
    let e = Sdl.Event.create ();
    let rec checkEvents () :bool => {
      let shouldQuit = ref false;
      while (Sdl.poll_event (Some e)) {
        switch Sdl.Event.(enum (get e typ)) {
        | `Quit => shouldQuit := true
        | `Mouse_button_down =>
          switch mouseDown {
          | None => ()
          | Some cb =>
            let x = Sdl.Event.(get e mouse_button_x);
            let y = Sdl.Event.(get e mouse_button_y);
            let button =
              switch Sdl.Event.(get e mouse_button_button) {
              | 1 => Events.LeftButton
              | 2 => Events.MiddleButton
              | 3 => Events.RightButton
              | _ => failwith "Button not supported"
              };
            cb ::button state::Events.MouseDown ::x ::y;
            ()
          }
        | `Mouse_button_up =>
          switch mouseUp {
          | None => ()
          | Some cb =>
            let x = Sdl.Event.(get e mouse_button_x);
            let y = Sdl.Event.(get e mouse_button_y);
            let button =
              switch Sdl.Event.(get e mouse_button_button) {
              | 1 => Events.LeftButton
              | 2 => Events.MiddleButton
              | 3 => Events.RightButton
              | _ => failwith "Button not supported"
              };
            cb ::button state::Events.MouseUp ::x ::y;
            ()
          }
        | `Mouse_motion =>
          switch mouseMove {
          | None => ()
          | Some cb =>
            let x = Sdl.Event.(get e mouse_motion_x);
            let y = Sdl.Event.(get e mouse_motion_y);
            cb ::x ::y;
            ()
          }
        | `Window_event =>
          switch windowResize {
          | None => ()
          | Some cb =>
            switch Sdl.Event.(window_event_enum (get e window_event_id)) {
            | `Resized
            | `Maximized
            | `Restored => cb ()
            | _ => ()
            }
          }
        | `Key_down =>
          switch keyDown {
          | None => ()
          | Some cb =>
            let (keycode, repeat) =
              Sdl.Event.(get e keyboard_keycode, get e keyboard_repeat);
            cb keycode::(Events.keycodeMap keycode) repeat::(repeat === 1)
          }
        | `Key_up =>
          switch keyUp {
          | None => ()
          | Some cb =>
            let keycode = Sdl.Event.(get e keyboard_keycode);
            cb keycode::(Events.keycodeMap keycode)
          }
        | _ => ()
        }
      };
      !shouldQuit
    };
    let prevTime = ref Int64.zero;
    let rec tick () => {
      let time = Sdl.get_performance_counter ();
      let diff =
        Int64.div
          (Int64.mul (Int64.sub time !prevTime) (Int64.of_float 1000.))
          (Sdl.get_performance_frequency ());
      let shouldQuit = checkEvents ();
      if (Int64.compare diff (Int64.of_float 16.666) == 1) {
        displayFunc (Int64.to_float diff);
        Sdl.gl_swap_window window;
        prevTime := time
      };
      if (not shouldQuit) {
        tick ()
      }
    };
    tick ()
  };
  type programT = int;
  type shaderT = int;
  let clearColor ::context ::r ::g ::b ::a => Gl.clear_color r g b a;
  let createProgram context::(context: contextT) :programT =>
    Gl.create_program ();
  let createShader context::(context: contextT) ::shaderType :shaderT =>
    Gl.create_shader shaderType;
  let attachShader ::context ::program ::shader =>
    Gl.attach_shader program shader;
  let deleteShader ::context ::shader => Gl.delete_shader shader;
  let shaderSource ::context ::shader ::source =>
    Gl.shader_source shader ("#version 120 \n" ^ source);
  let compileShader ::context ::shader => Gl.compile_shader shader;
  let linkProgram ::context ::program => Gl.link_program program;
  let useProgram ::context ::program => Gl.use_program program;
  type bufferT = Gl.enum;
  type attributeT = int;
  type uniformT = int;
  let createBuffer = {
    let a = Bigarray.Array1.create Bigarray.int32 Bigarray.c_layout 1;
    fun context::(context: contextT) => {
      Gl.gen_buffers 1 a;
      Int32.to_int a.{0}
    }
  };
  let bindBuffer context::(context: contextT) ::target ::buffer =>
    Gl.bind_buffer target buffer;
  type textureT = Gl.enum;
  let createTexture = {
    let a = Bigarray.Array1.create Bigarray.int32 Bigarray.c_layout 1;
    fun context::(context: contextT) => {
      Gl.gen_textures 1 a;
      Int32.to_int a.{0}
    }
  };
  let activeTexture ::context ::target => Gl.active_texture target;
  let bindTexture ::context ::target ::texture =>
    Gl.bind_texture target texture;
  let texParameteri context::contextT ::target ::pname ::param =>
    Gl.tex_parameteri target pname param;

  /** TODO: this causes bugs when loading big images, as ocaml arrays are limited in size. We need to replace
      this with a Bigarary. This will also improve general performance as we won't need to copy over data. */
  type rawTextureDataT = array int;
  let toTextureData data => data;
  let enable ::context i => Gl.enable i;
  let disable ::context i => Gl.disable i;
  let blendFunc ::context a b => Gl.blend_func a b;
  type frameBufferT = int;
  let createFrameBuffer = {
    let a = Bigarray.Array1.create Bigarray.int32 Bigarray.c_layout 1;
    fun context::(context: contextT) => {
      Gl.gen_framebuffers 1 a;
      Int32.to_int a.{0}
    }
  };
  let bindFrameBuffer ::context ::target ::frameBuffer =>
    switch frameBuffer {
    | None => Gl.bind_framebuffer target 0
    | Some frameBuffer => Gl.bind_framebuffer target frameBuffer
    };
  let framebufferTexture2d
      ::context
      ::target
      ::attachment
      ::texTarget
      ::texture
      ::level =>
    Gl.framebuffer_texture2d target attachment texTarget texture level;
  let readPixelsRGBA ::context ::x ::y ::width ::height => {
    /* pixel format: RGBA with 1 byte per color */
    let data =
      Bigarray.Array1.create
        Bigarray.int8_unsigned Bigarray.c_layout (width * height * 4);
    Gl.read_pixels
      0
      0
      width
      height
      ReasonglInterface.Constants.rgba
      ReasonglInterface.Constants.unsigned_byte
      (`Data data);
    let size = Bigarray.Array1.dim data;
    let ocam_array = Array.make size 0;
    for i in 0 to (size - 1) {
      ocam_array.(i) = data.{i}
    };
    ocam_array
  };
  type loadOptionT =
    | LoadAuto
    | LoadL
    | LoadLA
    | LoadRGB
    | LoadRGBA;
  type imageT = {width: int, height: int, channels: int, data: array int};
  let getImageWidth image => image.width;
  let getImageHeight image => image.height;

  /**
   * Internal dep on SOIL. This helps us load a bunch of different formats of image and get a `unsigned char*`
   * which we transform into an `array int` and then a bigarray before passing it to tgls.
   *
   * This is very unefficient as we end we 3 copies of the data (1 original and 2 copies). We should be able
   * to pass in the C `char*` directly to tgls if we can figure out how ctypes works.
   */
  external soilLoadImage : filename::string => loadOption::int => option imageT =
    "load_image";
  let loadImage
      ::filename
      ::loadOption=LoadAuto
      callback::(callback: option imageT => unit)
      () =>
    switch loadOption {
    | LoadAuto => callback (soilLoadImage ::filename loadOption::0)
    | LoadL => callback (soilLoadImage ::filename loadOption::1)
    | LoadLA => callback (soilLoadImage ::filename loadOption::2)
    | LoadRGB => callback (soilLoadImage ::filename loadOption::3)
    | LoadRGBA => callback (soilLoadImage ::filename loadOption::4)
    };
  let texImage2D
      context::(context: contextT)
      ::target
      ::level
      ::internalFormat
      ::width
      ::height
      ::format
      ::type_
      ::data =>
    Gl.tex_image2d
      target
      level
      internalFormat
      width
      height
      0
      format
      type_
      (
        `Data (
          Bigarray.Array1.of_array
            Bigarray.int8_unsigned Bigarray.c_layout data
        )
      );
  let texImage2DWithImage ::context ::target ::level ::image => {
    /* We only support rgb and rgba for now. */
    let format =
      switch image.channels {
      | 3 => ReasonglInterface.Constants.rgb
      | 4 => ReasonglInterface.Constants.rgba
      | _ => assert false
      };
    texImage2D
      context
      target
      level
      format
      image.width
      image.height
      format
      ReasonglInterface.Constants.unsigned_byte
      image.data
  };
  let uniform1i ::context ::location v0 => Gl.uniform1i location v0;
  let uniform1f ::context ::location v0 => Gl.uniform1f location v0;
  let generateMipmap ::context ::target => Gl.generate_mipmap target;
  /* type float32Array = Bigarray.float32_elt;
     type uint16Array = Bigarray.int16_unsigned_elt;
     type uint8Array = Bigarray.int8_unsigned_elt; */
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
    type t 'a 'b = Bigarray.Array1.t 'a 'b Bigarray.c_layout;
    type float64_elt = Bigarray.float64_elt;
    type float32_elt = Bigarray.float32_elt;
    type int16_unsigned_elt = Bigarray.int16_unsigned_elt;
    type int16_signed_elt = Bigarray.int16_signed_elt;
    type int8_unsigned_elt = Bigarray.int8_unsigned_elt;
    type int8_signed_elt = Bigarray.int8_signed_elt;
    type int_elt = Bigarray.int_elt;
    type int32_elt = Bigarray.int32_elt;
    type int64_elt = Bigarray.int64_elt;
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
      | Float64 =>
        Bigarray.Array1.create Bigarray.Float64 Bigarray.c_layout size
      | Float32 =>
        Bigarray.Array1.create Bigarray.Float32 Bigarray.c_layout size
      | Int16 =>
        Bigarray.Array1.create Bigarray.Int16_signed Bigarray.c_layout size
      | Uint16 =>
        Bigarray.Array1.create Bigarray.Int16_unsigned Bigarray.c_layout size
      | Int8 =>
        Bigarray.Array1.create Bigarray.Int8_signed Bigarray.c_layout size
      | Uint8 =>
        Bigarray.Array1.create Bigarray.Int8_unsigned Bigarray.c_layout size
      | Char => Bigarray.Array1.create Bigarray.Char Bigarray.c_layout size
      | Int => Bigarray.Array1.create Bigarray.Int Bigarray.c_layout size
      | Int64 => Bigarray.Array1.create Bigarray.Int64 Bigarray.c_layout size
      | Int32 => Bigarray.Array1.create Bigarray.Int32 Bigarray.c_layout size
      };
    let of_array (type a) (type b) (kind: kind a b) (arr: array a) :t a b =>
      switch kind {
      | Float64 =>
        Bigarray.Array1.of_array Bigarray.Float64 Bigarray.c_layout arr
      | Float32 =>
        Bigarray.Array1.of_array Bigarray.Float32 Bigarray.c_layout arr
      | Int16 =>
        Bigarray.Array1.of_array Bigarray.Int16_signed Bigarray.c_layout arr
      | Uint16 =>
        Bigarray.Array1.of_array Bigarray.Int16_unsigned Bigarray.c_layout arr
      | Int8 =>
        Bigarray.Array1.of_array Bigarray.Int8_signed Bigarray.c_layout arr
      | Uint8 =>
        Bigarray.Array1.of_array Bigarray.Int8_unsigned Bigarray.c_layout arr
      | Char => Bigarray.Array1.of_array Bigarray.Char Bigarray.c_layout arr
      | Int => Bigarray.Array1.of_array Bigarray.Int Bigarray.c_layout arr
      | Int64 => Bigarray.Array1.of_array Bigarray.Int64 Bigarray.c_layout arr
      | Int32 => Bigarray.Array1.of_array Bigarray.Int32 Bigarray.c_layout arr
      };
    let dim = Bigarray.Array1.dim;
    let get = Bigarray.Array1.get;
    let set = Bigarray.Array1.set;
    let sub (type a) (type b) (arr: t a b) ::offset ::len :t a b =>
      Bigarray.Array1.sub arr offset len;
  };
  /* type float32 = Bigarray.float32_elt;
     type uint16 = Bigarray.int16_unsigned_elt;
     type uint8 = Bigarray.int8_unsigned_elt;
     type bigarray 'a 'b = Bigarray.Array1.t 'a 'b Bigarray.c_layout;
     let createFloat32Array size => Bigarray.Array1.create Bigarray.float32 Bigarray.c_layout size;
     let createFloat32ArrayWithArray arr =>
       Bigarray.Array1.of_array Bigarray.float32 Bigarray.c_layout arr;
     let createUint16Array size =>
       Bigarray.Array1.create Bigarray.int16_unsigned Bigarray.c_layout size;
     let arrayLength arr => Bigarray.Array1.dim arr;
     let createUint8Array size =>
       Bigarray.Array1.create Bigarray.int8_unsigned Bigarray.c_layout size;
     let get arr i => arr.{i};
     let set arr i v => arr.{i} = v; */
  let bufferData
      context::(context: contextT)
      ::target
      data::(data: Bigarray.t 'a 'b)
      ::usage =>
    Gl.buffer_data target (Gl.bigarray_byte_size data) (Some data) usage;
  /* let bufferData context::(context: contextT) ::target data::(data: dataKind) ::usage =>
     switch data {
     | Float32 d =>
       let bigData = createFloat32Array d;
       Gl.buffer_data target (Gl.bigarray_byte_size bigData) (Some bigData) usage
     | UInt16 d =>
       let bigData = createUint16Array d;
       Gl.buffer_data target (Gl.bigarray_byte_size bigData) (Some bigData) usage
     }; */
  let viewport context::(context: contextT) ::x ::y ::width ::height =>
    Gl.viewport x y width height;
  let clear context::(context: contextT) ::mask => Gl.clear mask;
  let getUniformLocation
      context::(context: contextT)
      program::(program: programT)
      ::name
      :uniformT =>
    Gl.get_uniform_location program name;
  let getAttribLocation
      context::(context: contextT)
      program::(program: programT)
      ::name
      :attributeT =>
    Gl.get_attrib_location program name;
  let enableVertexAttribArray context::(context: contextT) ::attribute =>
    Gl.enable_vertex_attrib_array attribute;
  let vertexAttribPointer
      context::(context: contextT)
      ::attribute
      ::size
      ::type_
      ::normalize
      ::stride
      ::offset =>
    /* For now `offset` is only going to be an offset (limited by the webgl API?). */
    Gl.vertex_attrib_pointer
      attribute size type_ normalize stride (`Offset offset);
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
    let epsilon = 0.00001;
    let create () => [|
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
    let identity out::(out: t) => {
      out.(0) = 1.0;
      out.(1) = 0.0;
      out.(2) = 0.0;
      out.(3) = 0.0;
      out.(4) = 0.0;
      out.(5) = 1.0;
      out.(6) = 0.0;
      out.(7) = 0.0;
      out.(8) = 0.0;
      out.(9) = 0.0;
      out.(10) = 1.0;
      out.(11) = 0.0;
      out.(12) = 0.0;
      out.(13) = 0.0;
      out.(14) = 0.0;
      out.(15) = 1.0
    };
    let translate out::(out: t) matrix::(matrix: t) vec::(vec: array float) => {
      let x = vec.(0);
      let y = vec.(1);
      let z = vec.(2);
      if (matrix === out) {
        out.(12) =
          matrix.(0) *. x +. matrix.(4) *. y +. matrix.(8) *. z +. matrix.(12);
        out.(13) =
          matrix.(1) *. x +. matrix.(5) *. y +. matrix.(9) *. z +. matrix.(13);
        out.(14) =
          matrix.(2) *. x +. matrix.(6) *. y +. matrix.(10) *. z +. matrix.(14);
        out.(15) =
          matrix.(3) *. x +. matrix.(7) *. y +. matrix.(11) *. z +. matrix.(15)
      } else {
        let a00 = matrix.(0);
        let a01 = matrix.(1);
        let a02 = matrix.(2);
        let a03 = matrix.(3);
        let a10 = matrix.(4);
        let a11 = matrix.(5);
        let a12 = matrix.(6);
        let a13 = matrix.(7);
        let a20 = matrix.(8);
        let a21 = matrix.(9);
        let a22 = matrix.(10);
        let a23 = matrix.(11);
        out.(0) = a00;
        out.(1) = a01;
        out.(2) = a02;
        out.(3) = a03;
        out.(4) = a10;
        out.(5) = a11;
        out.(6) = a12;
        out.(7) = a13;
        out.(8) = a20;
        out.(9) = a21;
        out.(10) = a22;
        out.(11) = a23;
        out.(12) = a00 *. x +. a10 *. y +. a20 *. z +. matrix.(12);
        out.(13) = a01 *. x +. a11 *. y +. a21 *. z +. matrix.(13);
        out.(14) = a02 *. x +. a12 *. y +. a22 *. z +. matrix.(14);
        out.(15) = a03 *. x +. a13 *. y +. a23 *. z +. matrix.(15)
      }
    };
    let scale out::(out: t) matrix::(matrix: t) vec::(vec: array float) => {
      let x = vec.(0);
      let y = vec.(1);
      let z = vec.(2);
      out.(0) = matrix.(0) *. x;
      out.(1) = matrix.(1) *. x;
      out.(2) = matrix.(2) *. x;
      out.(3) = matrix.(3) *. x;
      out.(4) = matrix.(4) *. y;
      out.(5) = matrix.(5) *. y;
      out.(6) = matrix.(6) *. y;
      out.(7) = matrix.(7) *. y;
      out.(8) = matrix.(8) *. z;
      out.(9) = matrix.(9) *. z;
      out.(10) = matrix.(10) *. z;
      out.(11) = matrix.(11) *. z;
      out.(12) = matrix.(12);
      out.(13) = matrix.(13);
      out.(14) = matrix.(14);
      out.(15) = matrix.(15)
    };
    let rotate
        out::(out: t)
        matrix::(matrix: t)
        rad::(rad: float)
        vec::(vec: array float) => {
      let x = matrix.(0);
      let y = matrix.(1);
      let z = matrix.(2);
      let len = sqrt (x *. x +. y *. y +. z *. z);
      if (abs_float len > epsilon) {
        let len = 1. /. sqrt (x *. x +. y *. y +. z *. z);
        let x = matrix.(0) *. len;
        let y = matrix.(1) *. len;
        let z = matrix.(2) *. len;
        let s = sin rad;
        let c = cos rad;
        let t = 1. -. c;
        let a00 = matrix.(0);
        let a01 = matrix.(1);
        let a02 = matrix.(2);
        let a03 = matrix.(3);
        let a10 = matrix.(4);
        let a11 = matrix.(5);
        let a12 = matrix.(6);
        let a13 = matrix.(7);
        let a20 = matrix.(8);
        let a21 = matrix.(9);
        let a22 = matrix.(10);
        let a23 = matrix.(11);
        let b00 = x *. x *. t +. c;
        let b01 = y *. x *. t +. z *. s;
        let b02 = z *. x *. t -. y *. s;
        let b10 = x *. y *. t -. y *. s;
        let b11 = y *. y *. t -. c;
        let b12 = z *. y *. t +. x *. s;
        let b20 = x *. z *. t +. y *. s;
        let b21 = y *. z *. t -. x *. s;
        let b22 = z *. z *. t +. c;
        matrix.(0) = a00 *. b00 +. a10 *. b01 +. a20 *. b02;
        matrix.(1) = a01 *. b00 +. a11 *. b01 +. a21 *. b02;
        matrix.(2) = a02 *. b00 +. a12 *. b01 +. a22 *. b02;
        matrix.(3) = a03 *. b00 +. a13 *. b01 +. a23 *. b02;
        matrix.(4) = a00 *. b10 +. a10 *. b11 +. a20 *. b12;
        matrix.(5) = a01 *. b10 +. a11 *. b11 +. a21 *. b12;
        matrix.(6) = a02 *. b10 +. a12 *. b11 +. a22 *. b12;
        matrix.(7) = a03 *. b10 +. a13 *. b11 +. a23 *. b12;
        matrix.(8) = a00 *. b20 +. a10 *. b21 +. a20 *. b22;
        matrix.(9) = a01 *. b20 +. a11 *. b21 +. a21 *. b22;
        matrix.(10) = a02 *. b20 +. a12 *. b21 +. a22 *. b22;
        matrix.(11) = a03 *. b20 +. a13 *. b21 +. a23 *. b22
      };
      if (matrix !== out) {
        out.(12) = matrix.(12);
        out.(13) = matrix.(13);
        out.(14) = matrix.(14);
        out.(15) = matrix.(15)
      }
    };
    let ortho
        out::(out: t)
        left::(left: float)
        right::(right: float)
        bottom::(bottom: float)
        top::(top: float)
        near::(near: float)
        far::(far: float) => {
      let lr = 1. /. (left -. right);
      let bt = 1. /. (bottom -. top);
      let nf = 1. /. (near -. far);
      out.(0) = (-2.) *. lr;
      out.(1) = 0.;
      out.(2) = 0.;
      out.(3) = 0.;
      out.(4) = 0.;
      out.(5) = (-2.) *. bt;
      out.(6) = 0.;
      out.(7) = 0.;
      out.(8) = 0.;
      out.(9) = 0.;
      out.(10) = 2. *. nf;
      out.(11) = 0.;
      out.(12) = (left +. right) *. lr;
      out.(13) = (top +. bottom) *. bt;
      out.(14) = (far +. near) *. nf;
      out.(15) = 1.
    };
  };

  /** count = 1 for now https://www.opengl.org/sdk/docs/man/html/glUniform.xhtml
   * and transform = false because "Must be GL_FALSE"...
   */
  let uniformMatrix4fv context::(context: contextT) ::location ::value =>
    Gl.uniform_matrix4fv
      location
      1
      false
      (Bigarray.of_array Bigarray.Float32 (Mat4.to_array value));
  type shaderParamsT =
    | Shader_delete_status
    | Compile_status
    | Shader_type;
  type programParamsT =
    | Program_delete_status
    | Link_status
    | Validate_status;

  /**
   * We use Bigarray here as some sort of pointer.
   */
  let _getProgramParameter = {
    let a = Bigarray.create Bigarray.Int32 1;
    fun context::(context: contextT) program::(program: programT) ::paramName => {
      Gl.get_programiv program paramName a;
      Int32.to_int (Bigarray.get a 0)
    }
  };
  let getProgramParameter
      context::(context: contextT)
      program::(program: programT)
      ::paramName =>
    switch paramName {
    | Program_delete_status =>
      _getProgramParameter ::context ::program paramName::Gl.delete_status
    | Link_status =>
      _getProgramParameter ::context ::program paramName::Gl.link_status
    | Validate_status =>
      _getProgramParameter ::context ::program paramName::Gl.validate_status
    };
  let _getShaderParameter = {
    let a = Bigarray.create Bigarray.Int32 1;
    fun context::(context: contextT) ::shader ::paramName => {
      Gl.get_shaderiv shader paramName a;
      Int32.to_int (Bigarray.get a 0)
    }
  };
  let getShaderParameter context::(context: contextT) ::shader ::paramName =>
    switch paramName {
    | Shader_delete_status =>
      _getShaderParameter ::context ::shader paramName::Gl.delete_status
    | Compile_status =>
      _getShaderParameter ::context ::shader paramName::Gl.compile_status
    | Shader_type =>
      _getShaderParameter ::context ::shader paramName::Gl.shader_type
    };
  let getShaderInfoLog context::(context: contextT) ::shader => {
    let len =
      _getShaderParameter ::context ::shader paramName::Gl.info_log_length;
    let logData = Bigarray.create Bigarray.Char len;
    Gl.get_shader_info_log shader len None logData;
    Gl.string_of_bigarray logData
  };
  let getProgramInfoLog context::(context: contextT) ::program => {
    let len =
      _getProgramParameter ::context ::program paramName::Gl.info_log_length;
    let logData = Bigarray.create Bigarray.Char len;
    Gl.get_program_info_log program len None logData;
    Gl.string_of_bigarray logData
  };
  let getShaderSource context::(context: contextT) shader::(shader: shaderT) => {
    let len =
      _getShaderParameter
        ::context ::shader paramName::Gl.shader_source_length;
    let logData = Bigarray.create Bigarray.Char len;
    Gl.get_shader_source shader len None logData;
    Gl.string_of_bigarray logData
  };
  let drawArrays context::(context: contextT) ::mode ::first ::count =>
    Gl.draw_arrays mode first count;
  let drawElements context::(context: contextT) ::mode ::count ::type_ ::offset =>
    Gl.draw_elements mode count type_ (`Offset offset);
};
