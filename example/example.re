module Constants = RGLConstants;

module Gl = Reasongl.Gl;


/***
 * This program is an example of how to draw a square.
 * You can vary the number of vertices drawn, allowing you to draw triangles, squares and circles.
 */
type glCamera = {projectionMatrix: Gl.Mat4.t};

type glEnv = {
  camera: glCamera,
  window: Gl.Window.t,
  context: Gl.contextT
};


/***
 * Helper function which will initialize the shaders and attach them to the GL context.
 * Returns the program.
 */
let getProgram =
    (
      ~context: Gl.contextT,
      ~vertexShader as vertexShaderSource: string,
      ~fragmentShader as fragmentShaderSource: string
    )
    : option(Gl.programT) => {
  let vertexShader = Gl.createShader(~context, Constants.vertex_shader);
  Gl.shaderSource(~context, ~shader=vertexShader, ~source=vertexShaderSource);
  Gl.compileShader(~context, vertexShader);
  let compiledCorrectly =
    Gl.getShaderParameter(~context, ~shader=vertexShader, ~paramName=Gl.Compile_status) == 1;
  if (compiledCorrectly) {
    let fragmentShader = Gl.createShader(~context, Constants.fragment_shader);
    Gl.shaderSource(~context, ~shader=fragmentShader, ~source=fragmentShaderSource);
    Gl.compileShader(~context, fragmentShader);
    let compiledCorrectly =
      Gl.getShaderParameter(~context, ~shader=fragmentShader, ~paramName=Gl.Compile_status) == 1;
    if (compiledCorrectly) {
      let program = Gl.createProgram(~context);
      Gl.attachShader(~context, ~program, ~shader=vertexShader);
      Gl.deleteShader(~context, vertexShader);
      Gl.attachShader(~context, ~program, ~shader=fragmentShader);
      Gl.deleteShader(~context, fragmentShader);
      Gl.linkProgram(~context, program);
      let linkedCorrectly =
        Gl.getProgramParameter(~context, ~program, ~paramName=Gl.Link_status) == 1;
      if (linkedCorrectly) {
        Some(program)
      } else {
        print_endline @@ "Linking error: " ++ Gl.getProgramInfoLog(~context, program);
        None
      }
    } else {
      print_endline @@ "Fragment shader error: " ++ Gl.getShaderInfoLog(~context, fragmentShader);
      None
    }
  } else {
    print_endline @@ "Vertex shader error: " ++ Gl.getShaderInfoLog(~context, vertexShader);
    None
  }
};


/***
 * Dumb vertex shader which take for input a vertex position and a vertex color and maps the point onto
 * the screen.
 * Fragment shader simply applies the color to the pixel.
 */
let vertexShaderSource = {|
  attribute vec3 aVertexPosition;
  attribute vec4 aVertexColor;

  uniform mat4 uPMatrix;

  varying vec4 vColor;

  void main(void) {
    gl_Position = uPMatrix * vec4(aVertexPosition, 1.0);
    vColor = aVertexColor;
  }
|};

let fragmentShaderSource = {|
  varying vec4 vColor;

  void main(void) {
    gl_FragColor = vColor;
  }
|};


/*** This initializes the window **/
let window = Gl.Window.init(~screen="main", ~argv=Sys.argv);

let windowSize = 600;

Gl.Window.setWindowSize(~window, ~width=windowSize, ~height=windowSize);


/*** Initialize the Gl context **/
let context = Gl.Window.getContext(window);

Gl.viewport(~context, ~x=0, ~y=0, ~width=windowSize, ~height=windowSize);

/* Gl.clearColor context 1.0 1.0 1.0 1.0; */
Gl.clear(~context, ~mask=Constants.color_buffer_bit lor Constants.depth_buffer_bit);


/*** Camera is a simple record containing one matrix used to project a point in 3D onto the screen. **/
let camera = {projectionMatrix: Gl.Mat4.create()};


/***
 * Those buffers are basically pointers to chunks of memory on the graphics card. They're used to store the
 * vertex and color data.
 */
let vertexBuffer = Gl.createBuffer(~context);

let colorBuffer = Gl.createBuffer(~context);


/*** Compiles the shaders and gets the program with the shaders loaded into **/
let program =
  switch (
    getProgram(~context, ~vertexShader=vertexShaderSource, ~fragmentShader=fragmentShaderSource)
  ) {
  | None => failwith("Could not create the program and/or the shaders. Aborting.")
  | Some(program) => program
  };

Gl.useProgram(~context, program);


/*** Get the attribs ahead of time to be used inside the render function **/
let aVertexPosition = Gl.getAttribLocation(~context, ~program, ~name="aVertexPosition");

Gl.enableVertexAttribArray(~context, ~attribute=aVertexPosition);

let aVertexColor = Gl.getAttribLocation(~context, ~program, ~name="aVertexColor");

Gl.enableVertexAttribArray(~context, ~attribute=aVertexColor);

let pMatrixUniform = Gl.getUniformLocation(~context, ~program, ~name="uPMatrix");

Gl.uniformMatrix4fv(~context, ~location=pMatrixUniform, ~value=camera.projectionMatrix);


/***
 * Will mutate the projectionMatrix to be an ortho matrix with the given boundaries.
 * See this link for quick explanation of what this is.
 * https://shearer12345.github.io/graphics/assets/projectionPerspectiveVSOrthographic.png
 */
Gl.Mat4.ortho(
  ~out=camera.projectionMatrix,
  ~left=0.,
  ~right=float_of_int(Gl.Window.getWidth(window)),
  ~bottom=0.,
  ~top=float_of_int(Gl.Window.getHeight(window)),
  ~near=0.,
  ~far=100.
);


/***
 * Render simply draws a rectangle.
 */
let render = (_) => {
  /* 0,0 is the bottom left corner */
  let x = 150;
  let y = 150;
  let width = 300;
  let height = 300;

  /***
   * Setup vertices to be sent to the GPU and bind the data on the "register" called `array_buffer`.
   */
  let square_vertices = [|
    float_of_int @@ x + width,
    float_of_int @@ y + height,
    0.0,
    float_of_int(x),
    float_of_int @@ y + height,
    0.0,
    float_of_int @@ x + width,
    float_of_int(y),
    0.0,
    float_of_int(x),
    float_of_int(y),
    0.0
  |];
  Gl.bindBuffer(~context, ~target=Constants.array_buffer, ~buffer=vertexBuffer);
  Gl.bufferData(
    ~context,
    ~target=Constants.array_buffer,
    ~data=Gl.Bigarray.(of_array(Float32, square_vertices)),
    ~usage=Constants.static_draw
  );
  Gl.vertexAttribPointer(
    ~context,
    ~attribute=aVertexPosition,
    ~size=3,
    ~type_=Constants.float_,
    ~normalize=false,
    ~stride=0,
    ~offset=0
  );

  /*** Setup colors to be sent to the GPU **/
  let r = 1.;
  let g = 0.;
  let b = 0.;
  let a = 1.;
  let square_colors = [|r, g, b, a, r, g, b, a, r, g, b, a, r, g, b, a|];
  Gl.bindBuffer(~context, ~target=Constants.array_buffer, ~buffer=colorBuffer);
  Gl.bufferData(
    ~context,
    ~target=Constants.array_buffer,
    ~data=Gl.Bigarray.(of_array(Float32, square_colors)),
    ~usage=Constants.static_draw
  );
  Gl.vertexAttribPointer(
    ~context,
    ~attribute=aVertexColor,
    ~size=4,
    ~type_=Constants.float_,
    ~normalize=false,
    ~stride=0,
    ~offset=0
  );
  Gl.uniformMatrix4fv(~context, ~location=pMatrixUniform, ~value=camera.projectionMatrix);

  /*** Final call which actually does the "draw" **/
  Gl.drawArrays(~context, ~mode=Constants.triangle_strip, ~first=0, ~count=4)
};


/*** Start the render loop. **/
Gl.render(~window, ~displayFunc=render, ());
