#include <string.h> // for memcpy

#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/bigarray.h>

void bigarray_unsafe_blit(value arr1, value arr2, value offset, value numOfBytes) {
  char *arr1Data = Caml_ba_data_val(arr1);
  char *arr2Data = Caml_ba_data_val(arr2);
  memcpy(arr2Data + Int_val(offset) * Int_val(numOfBytes), arr1Data, caml_ba_byte_size(Caml_ba_array_val(arr1)));
}
