type contextT;

type gameViewControllerT;

type openGLT =
  | EAGLRenderingAPIOpenGLES1
  | EAGLRenderingAPIOpenGLES2
  | EAGLRenderingAPIOpenGLES3;

type _GLKViewDrawableDepthFormatT =
  | GLKViewDrawableDepthFormatNone
  | GLKViewDrawableDepthFormat16
  | GLKViewDrawableDepthFormat24;

external makeContext : openGLT => option(contextT) = "newEAGLContext";

type _GLKViewT;

[@noalloc] external setContext : ('GLKViewT, contextT) => unit = "setContext";

[@noalloc] external setPreferredFramesPerSecond : ('GLKViewT, int) => unit =
  "setPreferredFramesPerSecond";

external getView : gameViewControllerT => _GLKViewT = "getGLKView";
external getWidth : gameViewControllerT => int = "getWidth";
external getHeight : gameViewControllerT => int = "getHeight";

[@noalloc] external setDrawableDepthFormat : (_GLKViewT, _GLKViewDrawableDepthFormatT) => unit =
  "setDrawableDepthFormat";

/*external isNil : 'a => bool = "isNil";*/

[@noalloc] external setCurrentContext : contextT => unit = "setCurrentContext";


external loadImage : (~filename: string) => option('a) = "loadImage";
external loadImageFromMemory : (~data: string) => option('a) = "loadImageFromMemory";
external loadFile : (~filename: string) => option(string) = "loadFile";
/*external getTimeMs : unit => float = "getTimeMs";*/

external loadData : (~context: contextT, ~key: string) => option(bytes) = "loadData";
external saveData : (~context: contextT, ~key: string, ~value: bytes) => unit = "saveData";

/* external startHotReloading : (~context: contextT, ~host: string, ~baseFile: string) => unit = "startHotReloading"; */

type soundT;
external loadSound : string => soundT = "bindings_loadSound";
external playSound: (soundT, float, bool) => unit = "bindings_playSound";
