
cc_defs = /inc=$(ZLIBSRC)
c_deb =

.ifdef __DECC__
pref = /prefix=all
.endif



OBJS = png.obj, pngset.obj, pngget.obj, pngrutil.obj, pngtrans.obj,\
	pngwutil.obj, pngread.obj, pngmem.obj, pngwrite.obj, pngrtran.obj,\
	pngwtran.obj, pngrio.obj, pngwio.obj, pngerror.obj, pngpread.obj


CFLAGS= $(C_DEB) $(CC_DEFS) $(PREF)

all : pngtest.exe libpng.olb
		@ write sys$output " pngtest available"

libpng.olb : libpng.olb($(OBJS))
	@ write sys$output " Libpng available"


pngtest.exe : pngtest.obj libpng.olb
              link pngtest,libpng.olb/lib,$(ZLIBSRC)libz.olb/lib

test : pngtest.exe
   run pngtest

clean :
	delete *.obj;*,*.exe;


# Other dependencies.
png.obj : png.h, pngconf.h, pnglibconf.h, pngpriv.h, pngstruct.h, pnginfo.h, pngdebug.h, pngchunk.h 
pngpread.obj : png.h, pngconf.h, pnglibconf.h, pngpriv.h, pngstruct.h, pnginfo.h, pngdebug.h, pngchunk.h 
pngset.obj : png.h, pngconf.h, pnglibconf.h, pngpriv.h, pngstruct.h, pnginfo.h, pngdebug.h, pngchunk.h 
pngget.obj : png.h, pngconf.h, pnglibconf.h, pngpriv.h, pngstruct.h, pnginfo.h, pngdebug.h, pngchunk.h 
pngread.obj : png.h, pngconf.h, pnglibconf.h, pngpriv.h, pngstruct.h, pnginfo.h, pngdebug.h, pngchunk.h 
pngrtran.obj : png.h, pngconf.h, pnglibconf.h, pngpriv.h, pngstruct.h, pnginfo.h, pngdebug.h, pngchunk.h 
pngrutil.obj : png.h, pngconf.h, pnglibconf.h, pngpriv.h, pngstruct.h, pnginfo.h, pngdebug.h, pngchunk.h 
pngerror.obj : png.h, pngconf.h, pnglibconf.h, pngpriv.h, pngstruct.h, pnginfo.h, pngdebug.h, pngchunk.h 
pngmem.obj : png.h, pngconf.h, pnglibconf.h, pngpriv.h, pngstruct.h, pnginfo.h, pngdebug.h, pngchunk.h 
pngrio.obj : png.h, pngconf.h, pnglibconf.h, pngpriv.h, pngstruct.h, pnginfo.h, pngdebug.h, pngchunk.h 
pngwio.obj : png.h, pngconf.h, pnglibconf.h, pngpriv.h, pngstruct.h, pnginfo.h, pngdebug.h, pngchunk.h 
pngtrans.obj : png.h, pngconf.h, pnglibconf.h, pngpriv.h, pngstruct.h, pnginfo.h, pngdebug.h, pngchunk.h 
pngwrite.obj : png.h, pngconf.h, pnglibconf.h, pngpriv.h, pngstruct.h, pnginfo.h, pngdebug.h, pngchunk.h 
pngwtran.obj : png.h, pngconf.h, pnglibconf.h, pngpriv.h, pngstruct.h, pnginfo.h, pngdebug.h, pngchunk.h 
pngwutil.obj : png.h, pngconf.h, pnglibconf.h, pngpriv.h, pngstruct.h, pnginfo.h, pngdebug.h, pngchunk.h 

pngtest.obj : png.h, pngconf.h, pnglibconf.h
