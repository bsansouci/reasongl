#include <caml/alloc.h>
#include <caml/custom.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/callback.h>
#include <caml/bigarray.h>

#include <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>
#import <AVFoundation/AVFoundation.h>

#import "Reasongl.h"
#import <OpenGLES/ES2/glext.h>
#import <stdint.h>

#import <GLKit/GLKit.h>

void bigarray_unsafe_blit(value arr1, value arr2, value offset, value numOfBytes) {
  char *arr1Data = Caml_ba_data_val(arr1);
  char *arr2Data = Caml_ba_data_val(arr2);
  memcpy(arr2Data + Int_val(offset) * Int_val(numOfBytes), arr1Data, caml_ba_byte_size(Caml_ba_array_val(arr1)));
}

static value oreturn(void *v) {
  CAMLparam0();
  CAMLlocal1(ret);
  ret = caml_alloc_small(1, 0);
  Field(ret, 0) = (long)v;
  CAMLreturn(ret);
}

static value unboxed(GLuint i) {
  return Val_int(i);
}

// CAMLprim value isNil(value s) {
//   CAMLparam1(s);
//   CAMLreturn(Val_int((int)((id)Field(s, 0) == nil)));
// }

// CAMLprim value setProgram(value s, value program) {
//   CAMLparam2(s, program);
//   id s2 = (id)(void *)Field(s, 0);
//   ReasonglViewController *game = (ReasonglViewController *)s2;
//   game.program = (GLuint)Int_val(program);
//   CAMLreturn(Val_int(0));
// }

// CAMLprim value setUmvp(value s, value umvp) {
//   CAMLparam2(s, umvp);
//   id s2 = (id)(void *)Field(s, 0);
//   ReasonglViewController *game = (ReasonglViewController *)s2;
//   game.umvp = (GLint)Int_val(umvp);
//   CAMLreturn(Val_int(0));
// }

// CAMLprim value setVertexArray(value s, value vertexArray) {
//   CAMLparam2(s, vertexArray);
//   id s2 = (id)(void *)Field(s, 0);
//   ReasonglViewController *game = (ReasonglViewController *)s2;
//   game.vertexArray = (GLuint)Int_val(vertexArray);
//   CAMLreturn(Val_int(0));
// }

// CAMLprim value setVertexBuffer(value s, value vertexBuffer) {
//   CAMLparam2(s, vertexBuffer);
//   id s2 = (id)(void *)Field(s, 0);
//   ReasonglViewController *game = (ReasonglViewController *)s2;
//   game.vertexBuffer = (GLuint)Int_val(vertexBuffer);
//   CAMLreturn(Val_int(0));
// }

/** These are unique to ios */

// CAMLprim value getTimeMs() {
//   CAMLparam0();
//   CAMLreturn(caml_copy_double([[NSDate date] timeIntervalSince1970] * 1000.0));
// }

void setContext(value s, value c) {
  id s2 = (id)(void *)Field(s, 0);
  ReasonglViewController *game = (ReasonglViewController *)s2;
  game.context = (EAGLContext *)Field(c, 0);
}

void setPreferredFramesPerSecond(value s, value f) {
  id s2 = (id)(void *)Field(s, 0);
  ReasonglViewController *game = (ReasonglViewController *)s2;
  game.preferredFramesPerSecond = Int_val(f);
}

void setDrawableDepthFormat(value s, value f) {
  GLKView *view = (GLKView *)Field(s, 0);
  view.drawableDepthFormat = Int_val(f);
}

void setCurrentContext(value c) {
  [EAGLContext setCurrentContext:(EAGLContext *)Field(c, 0)];
}

CAMLprim value newEAGLContext(value num) {
  CAMLparam1(num);
  EAGLContext *ctx = [[EAGLContext alloc] initWithAPI:(Int_val(num) + 1)];
  if (ctx == NULL) {
    CAMLreturn(Val_int(0));
  }
  CAMLreturn(oreturn((void *)oreturn(ctx)));
}

CAMLprim value getGLKView(value s) {
  CAMLparam1(s);
  id s2 = (id)(void *)Field(s, 0);
  ReasonglViewController *game = (ReasonglViewController *)s2;
  GLKView *v = (GLKView *)game.view;
  CAMLreturn(oreturn(v));
}

CAMLprim value getWidth(value s) {
  CAMLparam1(s);
  id s2 = (id)(void *)Field(s, 0);
  ReasonglViewController *game = (ReasonglViewController *)s2;
  CAMLreturn(Val_int(game.viewSize.width));
}

CAMLprim value getHeight(value s) {
  CAMLparam1(s);
  id s2 = (id)(void *)Field(s, 0);
  ReasonglViewController *game = (ReasonglViewController *)s2;
  CAMLreturn(Val_int(game.viewSize.height));
}

void reasonglWindowResize() {
  static dispatch_once_t onceToken;
  static value *closure_f;
  dispatch_once(&onceToken, ^{
    closure_f = caml_named_value("reasonglWindowResize");
  });

  caml_callback(*closure_f, Val_int(0));
}

void reasonglMain(ReasonglViewController *viewController) {
  CAMLparam0();
  CAMLlocal1(ocamlViewController);
  value *reasongl_main = caml_named_value("reasonglMain");
  ocamlViewController = caml_alloc_small(1, 0);
  Field(ocamlViewController, 0) = (long)viewController;
  caml_callback(*reasongl_main, ocamlViewController);
  CAMLreturn0;
}

void reasonglUpdate(ReasonglViewController *s) {
  static dispatch_once_t onceToken;
  static value *closure_f;
  dispatch_once(&onceToken, ^{
    closure_f = caml_named_value("reasonglUpdate");
  });

  caml_callback(*closure_f, caml_copy_double(s.timeSinceLastDraw));
}

void reasonglTouchPress(double* touchDoubles, int count) {
  CAMLparam0();
  CAMLlocal3(cli, cons, element);

  int i;

  cli = Val_emptylist;

  for (i = count * 3 - 1; i >= 0; i-=3)
  {
      element = caml_alloc_small(3, Double_array_tag);

      double y =  touchDoubles[i];
      double x =  touchDoubles[i - 1];
      double id =  touchDoubles[i - 2];

      Double_field(element, 0) = (id);
      Double_field(element, 1) = (x);
      Double_field(element, 2) = (y);

      cons = caml_alloc_small(2, 0);
      Field(cons, 0) = element;  // head
      Field(cons, 1) = cli;              // tail

      cli = cons;
  }

  static dispatch_once_t onceToken;
  static value *closure_f;
  dispatch_once(&onceToken, ^{
    closure_f = caml_named_value("reasonglTouchPress");
  });
  caml_callback(*closure_f, cli);

  CAMLreturn0;
}

void reasonglTouchDrag(double* touchDoubles, int count) {
  CAMLparam0();
  CAMLlocal3(cli, cons, element);

  int i;

  cli = Val_emptylist;

  for (i = count * 3 - 1; i >= 0; i-=3)
  {
      element = caml_alloc_small(3, Double_array_tag);

      double y =  touchDoubles[i];
      double x =  touchDoubles[i - 1];
      double id =  touchDoubles[i - 2];

      Double_field(element, 0) = (id);
      Double_field(element, 1) = (x);
      Double_field(element, 2) = (y);

      cons = caml_alloc_small(2, 0);
      Field(cons, 0) = element;  // head
      Field(cons, 1) = cli;              // tail

      cli = cons;
  }

  static dispatch_once_t onceToken;
  static value *closure_f;
  dispatch_once(&onceToken, ^{
    closure_f = caml_named_value("reasonglTouchDrag");
  });
  caml_callback(*closure_f, cli);

  CAMLreturn0;
}

void reasonglTouchRelease(double* touchDoubles, int count) {
  CAMLparam0();
  CAMLlocal3(cli, cons, element);

  int i;

  cli = Val_emptylist;

  for (i = count * 3 - 1; i >= 0; i-=3)
  {
      element = caml_alloc_small(3, Double_array_tag);

      double y =  touchDoubles[i];
      double x =  touchDoubles[i - 1];
      double id =  touchDoubles[i - 2];

      Double_field(element, 0) = (id);
      Double_field(element, 1) = (x);
      Double_field(element, 2) = (y);

      cons = caml_alloc_small(2, 0);
      Field(cons, 0) = element;  // head
      Field(cons, 1) = cli;              // tail

      cli = cons;
  }

  static dispatch_once_t onceToken;
  static value *closure_f;
  dispatch_once(&onceToken, ^{
    closure_f = caml_named_value("reasonglTouchRelease");
  });
  caml_callback(*closure_f, cli);

  CAMLreturn0;
}

void Mat4_ortho_native(value out, value left, value right, value bottom, value top, value near, value far) {
  CAMLparam5(out, left, right, bottom, top);
  CAMLxparam2(near, far);
  GLKMatrix4 *m = (GLKMatrix4 *)Field(out, 0);
  GLKMatrix4 c = GLKMatrix4MakeOrtho(Double_val(left), Double_val(right), Double_val(bottom), Double_val(top), Double_val(near), Double_val(far));

  m->m[0] = c.m[0];
  m->m[1] = c.m[1];
  m->m[2] = c.m[2];
  m->m[3] = c.m[3];
  m->m[4] = c.m[4];
  m->m[5] = c.m[5];
  m->m[6] = c.m[6];
  m->m[7] = c.m[7];
  m->m[8] = c.m[8];
  m->m[9] = c.m[9];
  m->m[10] = c.m[10];
  m->m[11] = c.m[11];
  m->m[12] = c.m[12];
  m->m[13] = c.m[13];
  m->m[14] = c.m[14];
  m->m[15] = c.m[15];

  CAMLreturn0;
}

void Mat4_ortho_byte(value * argv, int argn) {
  Mat4_ortho_native(argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6]);
}

CAMLprim value Mat4_create() {
  CAMLparam0();
  CAMLlocal1(ret);
  
  // @MemoryLeak
  GLKMatrix4 *m = malloc(sizeof(GLKMatrix4));
  m->m[0] = 1.0;
  m->m[1] = 0.0;
  m->m[2] = 0.0;
  m->m[3] = 0.0;

  m->m[4] = 0.0;
  m->m[5] = 1.0;
  m->m[6] = 0.0;
  m->m[7] = 0.0;

  m->m[8] = 0.0;
  m->m[9] = 0.0;
  m->m[10] = 1.0;
  m->m[11] = 0.0;

  m->m[12] = 0.0;
  m->m[13] = 0.0;
  m->m[14] = 0.0;
  m->m[15] = 1.0;

  ret = caml_alloc_small(1, 0);
  Field(ret, 0) = (long)m;
  CAMLreturn(ret);
}


#define Val_none Val_int(0)

static value Val_some(value v) {
  CAMLparam1(v);
  CAMLlocal1(some);
  some = caml_alloc_small(1, 0);
  Field(some, 0) = v;
  CAMLreturn(some);
}

CAMLprim value loadFile(value filename) {
  CAMLparam1(filename);
  // @MemoryLeak
  NSString* name = [NSString stringWithUTF8String:String_val(filename)];
  if (!name) {
    return Val_none;
  }
  // @MemoryLeak
  NSString* path = [[NSBundle mainBundle] pathForResource:name ofType:@""];
  if (!path) {
    return Val_none;
  }
  // @MemoryLeak
  NSString* content = [NSString stringWithContentsOfFile:path
                                              encoding:NSUTF8StringEncoding
                                                 error:NULL];
  if (!content) {
    return Val_none;
  }
  CAMLreturn(Val_some(caml_copy_string([content UTF8String])));
}

CAMLprim value loadImageIntoOcamlRecord(UIImage *image) {
  CAMLparam0();
  CAMLlocal2(record_image_data, dataArr);
  
  CGImageRef spriteImage = image.CGImage;
  if (!spriteImage) {
    CAMLreturn(Val_none);
  } else {
    size_t width = CGImageGetWidth(spriteImage);
    size_t height = CGImageGetHeight(spriteImage);

    uint channels = 4;
    GLubyte * spriteData = (GLubyte *) calloc(width*height*channels, sizeof(GLubyte));

    // copy image into the data array
    CGContextRef spriteContext = CGBitmapContextCreate(
      spriteData, width, height, 8, width * channels,
      CGImageGetColorSpace(spriteImage),
      kCGImageAlphaPremultipliedLast
    );
    CGContextDrawImage(spriteContext, CGRectMake(0, 0, width, height), spriteImage);
    CGContextRelease(spriteContext);

    record_image_data = caml_alloc_small(4, 0);
    Field(record_image_data, 0) = Val_int(width);
    Field(record_image_data, 1) = Val_int(height);
    Field(record_image_data, 2) = Val_int(channels);
    
    // @MemoryLeak
    // put the data in a bigarray
    intnat *size = malloc(sizeof(intnat));
    *size = width * height * channels;
    dataArr = caml_ba_alloc(CAML_BA_UINT8, 1, spriteData, size);
    Field(record_image_data, 3) = dataArr;

    CAMLreturn(Val_some(record_image_data));
  }
}

CAMLprim value loadImage(value filename) {
  CAMLparam1(filename);
  
  // @MemoryLeak
  NSString* name = [NSString stringWithUTF8String:String_val(filename)];
  CAMLreturn(loadImageIntoOcamlRecord([UIImage imageNamed:name]));
}

CAMLprim value loadImageFromMemory(value data) {
  CAMLparam1(data);
  UIImage *image = [UIImage imageWithData:[[NSData alloc] initWithBytes:String_val(data) length:caml_string_length(data)]];
  CAMLreturn(loadImageIntoOcamlRecord(image));
}

NSArray<AVAudioPlayer *> *pool;
int lastUsablePlayer;

CAMLprim value bindings_loadSound(value path) {
  CAMLparam1(path);
  CAMLlocal1(ret);
  
  // @Hack we set the audio session's category to be ambient, which allows the iphone's music to be
  // played normally.
  AVAudioSession *audioSession = [AVAudioSession sharedInstance];
  [audioSession setCategory:AVAudioSessionCategoryAmbient error:nil];
    
  // @MemoryLeak
  NSArray<NSString *> *splitByDot = [[NSString stringWithUTF8String:String_val(path)] componentsSeparatedByString:@"."];
  NSInteger count = [splitByDot count];
  
  NSString *soundFilePath = [[NSBundle mainBundle] pathForResource:splitByDot[count - 2]  ofType:splitByDot[count - 1]];
  NSURL *soundFileURL = [NSURL fileURLWithPath:soundFilePath];

  // AVAudioPlayer *player = [[AVAudioPlayer alloc] initWithContentsOfURL:soundFileURL error:nil];
  
  ret = caml_alloc_small(1, 0);
  Field(ret, 0) = (long)[[NSData alloc] initWithContentsOfURL:soundFileURL];
  
  // free(player);

  CAMLreturn(ret);
}

void bindings_playSound(value sound, value volume, value loop) {
  CAMLparam3(sound, volume, loop);
  
  NSData *data = (NSData *)Field(sound, 0);
  
  int l = Int_val(loop);
  double v = Double_val(volume);
  dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
    AVAudioPlayer *player = [[AVAudioPlayer alloc] initWithData:data error:nil];
    
    player.numberOfLoops = l == 1 ? -1 : 0;
    player.volume = v;
    
    [player play];
  });
  
  CAMLreturn0;
}

// CAMLprim void saveDataToTmpFile(value context, value extension, value data) {
//   CAMLparam3(context, extension, data);

//   NSData* nsData = [NSData dataWithBytes:String_val(data) length:caml_string_length(data)];
//   NSString* keyString = [NSString stringWithUTF8String:String_val(key)];
//   [[NSUserDefaults standardUserDefaults] setObject:nsData forKey:keyString];

//   CAMLreturn0;
// }


void saveData(value context, value key, value data) {
  CAMLparam3(context, key, data);
  
  // @MemoryLeak
  NSData* nsData = [NSData dataWithBytes:String_val(data) length:caml_string_length(data)];
  NSString* keyString = [NSString stringWithUTF8String:String_val(key)];
  [[NSUserDefaults standardUserDefaults] setObject:nsData forKey:keyString];

  CAMLreturn0;
}

CAMLprim value loadData(value ocamlWindow, value key) {
  CAMLparam2(ocamlWindow, key);
  CAMLlocal1(ml_data);
  
  // @MemoryLeak
  NSString* keyString = [NSString stringWithUTF8String:String_val(key)];
  
  // @MemoryLeak
  NSData* nsData = [[NSUserDefaults standardUserDefaults] dataForKey:keyString];

  if (!nsData) {
    CAMLreturn(Val_none);
  } else {
    int len = nsData.length;
    // char buf[len];
    // (*g_env)->GetByteArrayRegion(g_env, array, 0, len, (jbyte*)buf);

    // from https://www.linux-nantes.org/~fmonnier/OCaml/ocaml-wrapping-c.html
    ml_data = caml_alloc_string(len);
    memcpy( String_val(ml_data), nsData.bytes, len );

    CAMLreturn(Val_some(ml_data));
  }
}

// CAMLprim void startHotReloading(value context, value host, value baseFile) {
//   CAMLparam3(context, host, baseFile);

//   NSString* hostString = [NSString stringWithUTF8String:String_val(host)];
//   NSString* baseFileString = [NSString stringWithUTF8String:String_val(baseFile)];

//   CAMLreturn0;
// }
