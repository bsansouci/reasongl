# SOIL makefile for linux (based on the AngelScript makefile)
# Type 'make' then 'make install' to complete the installation of the library

# For 'make install' to work, set LOCAL according to your system configuration
# LOCAL = $(cur__install)

LIB = libSOIL.a
INC = SOIL.h

SRCDIR = SOIL/src
LIBDIR = _build/lib/
INCDIR = SOIL/src
OBJDIR = _build/

CXX = gcc
CXXFLAGS =
DELETER = rm -f
COPIER = cp

SRCNAMES = \
  image_helper.c \
  stb_image_aug.c  \
  image_DXT.c \
  SOIL.c \

OBJ = $(addprefix $(OBJDIR)/, $(notdir $(SRCNAMES:.c=.o)))
BIN = $(LIBDIR)/$(LIB)

# This is the build rule for opam, we do the same first step as npm and then do the step that bsb
# would do
build: npm lib src/soil_wrapper.c
	gcc -I`ocamlc -where` -c src/soil_wrapper.c -o _build/lib/libsoil_wrapper.a

npm: prebuild $(BIN)

$(BIN): $(OBJ)
	ar r $(BIN) $(OBJ)
	ranlib $(BIN)
	@echo -------------------------------------------------------------------
	@echo Done. As root, type 'make install' to install the library.

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CXX) $(CXXFLAGS) -o $@ -c $<


clean:
	$(DELETER) $(OBJ) $(BIN)

install: $(BIN)
	@echo Installing to: $(LOCAL)/lib and $(LOCAL)/include...
	@echo -------------------------------------------------------------------
	$(COPIER) $(BIN) $(LOCAL)/lib
	$(COPIER) $(INCDIR)/$(INC) $(LOCAL)/include
	@echo -------------------------------------------------------------------
	@echo SOIL library installed. Enjoy!

uninstall:
	$(DELETER) $(LOCAL)/include/$(INC) $(LOCAL)/lib/$(LIB)
	@echo -------------------------------------------------------------------
	@echo SOIL library uninstalled.

.PHONY: all clean install uninstall

prebuild:
	mkdir -p _build/lib/
	mkdir -p _build/include
	cp SOIL/src/SOIL.h _build/include/SOIL.h

lib: _build/lib/libsoil_wrapper.a reasongl.o

_build/lib/libsoil_wrapper.a:	src/soil_wrapper.c
	gcc -I$(OCAML_LIB) -c src/soil_wrapper.c -o _build/lib/libsoil_wrapper.a

reasongl.o: src/reasongl.c
	ocamlopt.opt -c src/reasongl.c
