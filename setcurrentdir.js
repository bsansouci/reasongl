var fs = require('fs');
var path = require('path');

var cur__install = path.join(__dirname, "_build/");
var cur__lib     = path.join(__dirname, "_build/");
var includedir   = path.join(cur__install, 'include');
var libdirsoil   = path.join(cur__install, 'lib');

var pcfileforpkgconfig = `
Name: reglnative
Description: C dependencies of Reglnative.
Version: 1.0.0
Cflags: -I${includedir}
Libs: -L${libdirsoil} -lSOIL -lsoil_wrapper -framework CoreFoundation
`;

fs.writeFileSync(path.join(cur__lib, 'reasongl-native.pc'), pcfileforpkgconfig);
