#include <caml/alloc.h>
#include <caml/custom.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/callback.h>
#include <caml/bigarray.h>

#include <Foundation/Foundation.h>

#import <OpenGLES/ES2/glext.h>
#import <stdint.h>

#import <GLKit/GLKit.h>

//static value oreturn(void *v) {
////  CAMLparam0();
//  CAMLlocal1(ret);
//  ret = caml_alloc_small(1, 0);
//  Field(ret, 0) = (long)v;
//  return ret;
//}

//static value unboxed(GLuint i) {
//  return Val_int(i);
//}

CAMLprim value TglGetError() {
  CAMLparam0();
  CAMLreturn(Val_int(glGetError()));
}

CAMLprim value TglCreateProgram(value context) {
  CAMLparam1(context);
  CAMLreturn(Val_int(glCreateProgram()));
}

CAMLprim value TglCreateShader(value context, value shaderType) {
  CAMLparam2(context, shaderType);
  CAMLreturn(Val_int(glCreateShader(Int_val(shaderType))));
}

void TglAttachShader(value context, value program, value shader) {
  CAMLparam3(context, program, shader);
  glAttachShader(Int_val(program), Int_val(shader));
  CAMLreturn0;
}

void TglDeleteShader(value context, value shader) {
  CAMLparam2(context, shader);
  glDeleteShader(Int_val(shader));
  CAMLreturn0;
}

void TglShaderSource(value context, value shader, value stringArray) {
  CAMLparam3(context, shader, stringArray);
  int numOfElements = (int) Wosize_val(stringArray);

  const GLchar **arrOfElements = malloc(sizeof(GLchar *) * numOfElements);
  GLint *arrOfLengths = malloc(sizeof(GLint) * numOfElements);
  for(int i = 0; i < numOfElements; ++i) {
    value e = Field(stringArray, i);
    arrOfElements[i] = String_val(e);
    arrOfLengths[i] = (int) caml_string_length(e);
  }
  glShaderSource(Int_val(shader), numOfElements, arrOfElements, arrOfLengths);
  free(arrOfElements);
  free(arrOfLengths);
  CAMLreturn0;
}

void TglCompileShader(value context, value shader) {
  CAMLparam2(context, shader);
  glCompileShader(Int_val(shader));
  CAMLreturn0;
}

void TglLinkProgram(value context, value program) {
  CAMLparam2(context, program);
  glLinkProgram(Int_val(program));
  CAMLreturn0;
}

void TglUseProgram(value context, value program) {
  CAMLparam2(context, program);
  glUseProgram(Int_val(program));
  CAMLreturn0;
}

CAMLprim value TglGenBuffers(value context, value count) {
  CAMLparam2(context, count);
  CAMLlocal1(ret);

  int size = Int_val(count);
  // @MemoryLeak
  unsigned int *buffers = malloc(sizeof(unsigned int) * size);
  glGenBuffers(size, buffers);

  ret = caml_alloc_small(size, 0);
  for (int i = 0; i < size; ++i) {
    Field(ret, i) = Val_int(buffers[i]);
  }
  CAMLreturn(ret);
}

CAMLprim value TglGenBuffer(value context) {
  CAMLparam1(context);
  unsigned int buffers = 0;
  glGenBuffers(1, &buffers);
  CAMLreturn(Val_int(buffers));
}

void TglClearColor(value context, value r, value g, value b, value a) {
  glClearColor(Double_val(r), Double_val(g), Double_val(b), Double_val(a));
}

void TglBindBuffer(value context, value kind, value buffer) {
  glBindBuffer(Int_val(kind), Int_val(buffer));
}

CAMLprim value TglGenTextures(value context, value count) {
  CAMLparam2(context, count);
  CAMLlocal1(ret);

  int size = Int_val(count);
  // @MemoryLeak
  unsigned int *textures = malloc(sizeof(unsigned int) * size);
  glGenTextures(size, textures);

  ret = caml_alloc_small(size, 0);
  for (int i = 0; i < size; ++i) {
    Field(ret, i) = Val_int(textures[i]);
  }

  CAMLreturn(ret);
}

CAMLprim value TglGenTexture(value context) {
  CAMLparam1(context);
  unsigned int textures = 0;
  glGenTextures(1, &textures);
  CAMLreturn(Val_int(textures));
}

void TglActiveTexture(value context, value textureUnit) {
  CAMLparam2(context, textureUnit);
  glActiveTexture(Int_val(textureUnit));
  CAMLreturn0;
}

void TglBindTexture(value context, value kind, value texture) {
  CAMLparam3(context, kind, texture);
  glBindTexture(Int_val(kind), Int_val(texture));
  CAMLreturn0;
}

/* missing TglTexSubImage2D */
void TglTexSubImage2D_native(value context, value target, value level, value xoffset, value yoffset, value width, value height, value format, value type, value pixels) {
  glTexSubImage2D(Int_val(target), Int_val(level), Int_val(xoffset), Int_val(yoffset), Int_val(width), Int_val(height), Int_val(format), Int_val(type), Caml_ba_data_val(pixels));
}

void TglTexSubImage2D_bytecode(value * argv, int argn) {
  TglTexSubImage2D_native(argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8], argv[9]);
}

void TglTexParameteri(value context, value kind, value pname, value param) {
  CAMLparam4(context, kind, pname, param);
  glTexParameteri(Int_val(kind), Int_val(pname), Int_val(param));
  CAMLreturn0;
}

void TglEnable(value context, value thing) {
  glEnable(Int_val(thing));
}

void TglDisable(value context, value thing) {
  glDisable(Int_val(thing));
}

void TglBlendFunc(value context, value sfactor, value dfactor) {
  glBlendFunc(Int_val(sfactor), Int_val(dfactor));
}

CAMLprim value TglReadPixels_RGBA(value x, value y, value width, value height) {
  CAMLparam4(x, y, width, height);
  CAMLlocal1(ret);
  
  // @MemoryLeak
  // Allocate a pointer for caml_ba_alloc's sake.
  intnat *size = malloc(sizeof(intnat));
  *size = Int_val(width) * Int_val(height) * 4;

  char *data = malloc(*size * sizeof(char));

  glReadPixels(Int_val(x), Int_val(y), Int_val(width), Int_val(height), GL_RGBA, GL_UNSIGNED_BYTE, data);

  // return array of size `size` of dimension 1 of uint8 (char).
  ret = caml_ba_alloc(CAML_BA_UINT8, 1, data, size);
  CAMLreturn(ret);
}

void TglTexImage2D_RGBA_native(value target, value level, value width, value height, value border, value data) {
  CAMLparam5(target, level, width, height, border);
  CAMLxparam1(data);
  glTexImage2D(Int_val(target), Int_val(level), GL_RGBA, Int_val(width), Int_val(height), Int_val(border), GL_RGBA, GL_UNSIGNED_BYTE, Caml_ba_data_val(data));
  CAMLreturn0;
}

void TglTexImage2D_RGBA_bytecode(value * argv, int argn) {
  TglTexImage2D_RGBA_native(argv[0], argv[1], argv[2], argv[3], argv[4], argv[5]);
}

void TglTexImage2D_null(value target, value level, value width, value height, value border) {
  glTexImage2D(Int_val(target), Int_val(level), 4, Int_val(width), Int_val(height), Int_val(border), GL_RGBA, GL_UNSIGNED_BYTE, NULL);
}

void TglUniform1i(value context, value location, value v) {
  glUniform1i(Int_val(location), Int_val(v));
}

void TglUniform1f(value context, value location, value v) {
  glUniform1f(Int_val(location), Double_val(v));
}

void TglUniform2f(value location, value v1, value v2) {
  glUniform2f(Int_val(location), Double_val(v1), Double_val(v2));
}

void TglUniform3f(value location, value v1, value v2, value v3) {
  glUniform3f(Int_val(location), Double_val(v1), Double_val(v2), Double_val(v3));
}

void TglUniform4f(value location, value v1, value v2, value v3, value v4) {
  glUniform4f(Int_val(location), Double_val(v1), Double_val(v2), Double_val(v3), Double_val(v4));
}

void bufferData(value context, value constant, value data, value usage) {
  glBufferData(Int_val(constant), caml_ba_byte_size(Caml_ba_array_val(data)), Caml_ba_data_val(data), Int_val(usage));
}

void TglViewport(value context, value x, value y, value width, value height) {
  glViewport(Int_val(x), Int_val(y), Int_val(width), Int_val(height));
}

void TglClear(value context, value mask) {
  glClear(Int_val(mask));
}

CAMLprim value TglGetUniformLocation(value context, value program, value name) {
  CAMLparam3(context, program, name);
  CAMLreturn(Val_int(glGetUniformLocation(Int_val(program), String_val(name))));
}

CAMLprim value TglGetAttribLocation(value context, value program, value name) {
  CAMLparam3(context, program, name);
  CAMLreturn(Val_int(glGetAttribLocation(Int_val(program), String_val(name))));
}

void TglEnableVertexAttribArray(value context, value attrib) {
  glEnableVertexAttribArray(Int_val(attrib));
}

// OpenGL ES 2.0 doesn't have vertexattribdivisor
// void TglVertexAttribDivisor(value context, value attrib, value divisor) {
//   glVertexAttribDivisor(Int_val(attrib), Int_val(divisor));
// }

void TglVertexAttribPointer_native(value context, value index, value size, value typ, value normalized, value stride, value offset) {
  long o = (long)Int_val(offset);
  glVertexAttribPointer(Int_val(index), Int_val(size), Int_val(typ), Bool_val(normalized), Int_val(stride), (const GLvoid *)o);
}

void TglVertexAttribPointer_bytecode(value * argv, int argn) {
  TglVertexAttribPointer_native(argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6]);
}

CAMLprim value TglGetProgramiv(value context, value program, value pname) {
  CAMLparam3(context, program, pname);
  GLint ret;
  glGetProgramiv(Int_val(program), Int_val(pname), &ret);
  CAMLreturn(Val_int(ret));
}

CAMLprim value TglGetShaderiv(value context, value shader, value pname) {
  CAMLparam3(context, shader, pname);
  GLint ret;
  glGetShaderiv(Int_val(shader), Int_val(pname), &ret);
  CAMLreturn(Val_int(ret));
}

CAMLprim value TglGetShaderInfoLog(value context, value shader) {
  CAMLparam2(context, shader);
  CAMLlocal1(ret);

  GLint exactLength;
  glGetShaderiv(Int_val(shader), GL_INFO_LOG_LENGTH, &exactLength);
  // @MemoryLeak
  GLchar *buffer = malloc(exactLength * sizeof(char));
  glGetShaderInfoLog(Int_val(shader), exactLength - 1, NULL, buffer);

  ret = caml_copy_string(buffer);
  CAMLreturn(ret);
}

CAMLprim value TglGetProgramInfoLog(value context, value program) {
  CAMLparam2(context, program);
  CAMLlocal1(ret);

  GLint exactLength;
  glGetProgramiv(Int_val(program), GL_INFO_LOG_LENGTH, &exactLength);
  
  // @MemoryLeak
  GLchar *buffer = malloc(exactLength * sizeof(char));
  glGetProgramInfoLog(Int_val(program), exactLength - 1, NULL, buffer);
  ret = caml_copy_string(buffer);
  CAMLreturn(ret);
}

CAMLprim value TglGetShaderSource(value context, value shader) {
  CAMLparam2(context, shader);
  CAMLlocal1(ret);

  GLint exactLength;
  glGetShaderiv(Int_val(shader), GL_SHADER_SOURCE_LENGTH, &exactLength);
  
  // @MemoryLeak
  GLchar *buffer = malloc(exactLength * sizeof(char));
  glGetShaderSource(Int_val(shader), exactLength - 1, NULL, buffer);
  ret = caml_copy_string(buffer);
  CAMLreturn(ret);
}

void TglDrawArrays(value context, value mode, value first, value count) {
  glDrawArrays(Int_val(mode), Int_val(first), Int_val(count));
}

void TglDrawElements(value context, value mode, value first, value typ, value offset) {
  long o = (long)Int_val(offset);
  glDrawElements(Int_val(mode), Int_val(first), Int_val(typ), (const GLvoid *)o);
}

// OpenGL ES 2.0 doesn't have support for glDrawElementsInstanced.
// void TglDrawElementsInstanced(value mode, value first, value typ, value indices, value primcount) {
//   long o = (long)Int_val(indices);
//   glDrawElementsInstanced(Int_val(mode), Int_val(first), Int_val(typ), (const GLvoid *)o, Int_val(primcount));
// }

void TglUniformMatrix4fv_glk(value context, value location, value transpose, value val) {
  glUniformMatrix4fv(Int_val(location), 1, Bool_val(transpose), ((GLKMatrix4 *)Field(val, 0))->m);
}

void TglUniformMatrix4fv(value context, value location, value transpose, value val) {
  float matrix[16];
  for (int i = 0; i < 16; ++i){
    matrix[i] = Double_field(val, i);
  }
  glUniformMatrix4fv(Int_val(location), 1, Bool_val(transpose), matrix);
}

/* =========================================================== */

// CAMLprim value makeFloat32Array(value size, value arr) {
//   CAMLparam2(arr, size);
//   CAMLlocal1(ret);
//   int s = Int_val(size);
//   float *data = (float *)malloc(s * sizeof(float));
//   for(int i = 0; i < s; i++) {
//     data[i] = Double_field(arr, i);
//   }
//   ret = caml_alloc_small(2, 0);
//   Field(ret, 0) = s * sizeof(float);
//   Field(ret, 1) = (long)data;
//   CAMLreturn(ret);
// }

// Vertex Array stuff
CAMLprim value TglGenVertexArrayOES(value c) {
  CAMLparam1(c);
  // @MemoryLeak
  GLuint *vertexArray = malloc(sizeof(GLuint));
  glGenVertexArraysOES(1, vertexArray);
  CAMLreturn(Val_int(*vertexArray));
}

void TglBindVertexArrayOES(value c, value vertexArray) {
  glBindVertexArrayOES(Int_val(vertexArray));
}




CAMLprim value TglGenFramebuffers(value count) {
  CAMLparam1(count);
  CAMLlocal1(ret);

  int size = Int_val(count);
  unsigned int framebuffers[size];
  glGenFramebuffers(size, framebuffers);

  ret = caml_alloc_small(size, 0);
  for (int i = 0; i < size; ++i) {
    Field(ret, i) = Val_int(framebuffers[i]);
  }

  CAMLreturn(ret);
}

value TglGenFramebuffer() {
  unsigned int framebuffers = 0;
  glGenFramebuffers(1, &framebuffers);
  return (Val_int(framebuffers));
}

void TglBindFramebuffer(value kind, value framebuffer) {
  glBindFramebuffer(Int_val(kind), Int_val(framebuffer));
}

void TglBindDefaultFramebuffer(value kind) {
  glBindFramebuffer(Int_val(kind), 0);
}

void TglFramebufferTexture2D(value kind, value color, value textureKind, value texture, value level) {
  glFramebufferTexture2D(Int_val(kind), Int_val(color), Int_val(textureKind), Int_val(texture), Int_val(level));
}
