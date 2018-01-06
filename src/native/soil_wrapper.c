#include <unistd.h>

#include <caml/alloc.h>
#include <caml/bigarray.h>
#include <caml/custom.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include "../../SOIL/src/SOIL.h"

#define Val_none Val_int(0)

static value Val_some(value v) {
  CAMLparam1(v);
  CAMLlocal1(some);
  some = caml_alloc_small(1, 0);
  Field(some, 0) = v;
  CAMLreturn(some);
}

CAMLprim value load_image(value string, value force_channels) {
  CAMLparam2(string, force_channels);
  CAMLlocal2(record_image_data, dataArr);
  record_image_data = caml_alloc_small(4, 0);
  int force_channels_internal = Int_val(force_channels);
  int width, height, realChannels;
  unsigned char* data = SOIL_load_image(String_val(string), &width, &height, &realChannels, force_channels_internal);
  if (data == NULL) {
    CAMLreturn (Val_none);
  } else {
    // realChannels contains the number of channels that the image actually has.
    // We use force_channels_internal unless it's 0 (which means "auto") at which point we use realChannels.
    int channels = force_channels_internal != 0 ? force_channels_internal : realChannels;
    Field(record_image_data, 0) = Val_int(width);
    Field(record_image_data, 1) = Val_int(height);
    Field(record_image_data, 2) = Val_int(channels);

    // Try wrapping the `data` pointer in a box, so ctypes doesn't try to
    // dereference an unsigned char* but a boxed one.
    intnat *size = malloc(sizeof(intnat));
    *size = width * height * channels;

    dataArr = caml_ba_alloc(CAML_BA_UINT8, 1, data, size);
    Field(record_image_data, 3) = dataArr;
    CAMLreturn(Val_some(record_image_data));
  }
}

void reasongl_usleep(value time) {
  usleep(Int_val(time));
}
