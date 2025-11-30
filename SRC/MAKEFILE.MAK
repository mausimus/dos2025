.AUTODEPEND

.PATH.obj = C:\BIN

#		*Translator Definitions*
CC = bcc +QUIDPROQ.CFG
TASM = TASM
TLIB = tlib
TLINK = tlink
LIBPATH = C:\BORLANDC\LIB
INCLUDEPATH = C:\BORLANDC\INCLUDE


#		*Implicit Rules*
.c.obj:
  $(CC) -c {$< }

.cpp.obj:
  $(CC) -c {$< }

#		*List Macros*


EXE_dependencies =  \
 adlib.obj \
 assets.obj \
 audio.obj \
 ega.obj \
 game.obj \
 io.obj \
 main.obj \
 mouse.obj \
 quiz.obj \
 scratch.obj \
 util.obj \
 video.obj

#		*Explicit Rules*
c:\bin\quidproq.exe: quidproq.cfg $(EXE_dependencies)
  $(TLINK) /v/s/c/P-/L$(LIBPATH) @&&|
c0s.obj+
c:\bin\adlib.obj+
c:\bin\assets.obj+
c:\bin\audio.obj+
c:\bin\ega.obj+
c:\bin\game.obj+
c:\bin\io.obj+
c:\bin\main.obj+
c:\bin\mouse.obj+
c:\bin\quiz.obj+
c:\bin\scratch.obj+
c:\bin\util.obj+
c:\bin\video.obj
c:\bin\quidproq,c:\bin\quidproq
cs.lib
|


#		*Individual File Dependencies*
adlib.obj: quidproq.cfg adlib.c 

assets.obj: quidproq.cfg assets.c 

audio.obj: quidproq.cfg audio.c 

ega.obj: quidproq.cfg ega.c 

game.obj: quidproq.cfg game.c 

io.obj: quidproq.cfg io.c 

main.obj: quidproq.cfg main.c 

mouse.obj: quidproq.cfg mouse.c 

quiz.obj: quidproq.cfg quiz.c 

scratch.obj: quidproq.cfg scratch.c 

util.obj: quidproq.cfg util.c 

video.obj: quidproq.cfg video.c 

#		*Compiler Configuration File*
quidproq.cfg: makefile.mak
  copy &&|
-2
-a
-f-
-v
-G
-O
-Og
-Om
-Ov
-Ol
-Op
-Oi
-Z
-d
-b-
-vi
-w-ret
-w-nci
-w-inl
-wpin
-wamb
-wamp
-wasm
-wcln
-w-cpt
-wdef
-w-dup
-w-pia
-wsig
-wnod
-w-aus
-w-ill
-w-sus
-wstv
-wucp
-wuse
-w-ext
-w-ias
-w-ibc
-w-pre
-w-nst
-nC:\BIN
-I$(INCLUDEPATH)
-L$(LIBPATH)
| quidproq.cfg


