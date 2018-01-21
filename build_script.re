open Bsb_internals;


let ( +/ ) = Filename.concat;

gcc("lib" +/ "reasongl.o", ["src" +/ "native" +/ "reasongl.c"]);
