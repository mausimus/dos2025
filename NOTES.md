## Real-mode DOS + EGA + AdLib programming notes

### Memory

* Real-mode DOS has a concept of segments and offsets for representing memory pointers.
* These translate to `(segment * 16) + offset` linear addresses but since offset is 16-bit they are not unique.
* When aquiring new "far" memory (i.e. outside of your data segment) you get a normalised pointer (i.e. offset < 16).
* You can aquire more than 64kb memory at a time but you will need to watch out for offset rollover or use `huge`
pointer type which normalises a pointer each time it's modified.
* The memory model (compact, small, medium etc.) doesn't matter that much since you will probably still
allocate far memory yourself, it only matters if you'd like to have compiled-in data or a more than 64k of code.

### EGA

* EGA video memory is mapped at A000:0000 (64k size) but it's a pretty indirect mapping.
* 16-bit color mode is represented as four bitplanes, so each byte write modifies 8 pixels at a time.
* Before writing you need to set EGA registers to determine which bitplanes you'd like to write to.
* So to write full 8 pixels in 16 colors, you will need to make 4 byte writes to the same memory address, but
with different register values.
* Same for reading, you need to set EGA register to determine which bitplane you'd like to read a byte from.
* 640x200x4bpp mode technically requires 64k of memory, but is only accessed via first 16k addresses of video RAM.
* EGA has a few different write modes, latches that can mix video memory with system memory, bit-shifters
and masking; all that is well covered by literature and I used these techniques extensively.
* I'm targetting expanded EGA (256k video memory) meaning it's possible to have 4 pages of video memory and
switching between them is instantaneous. They are laid out consecutively after each other in memory
(16k per page * 4 pages = full 64k mapped memory window).
* In the game, I use one page for game screen, one page for animated sprites, one page for storing backgrounds
when a new pop-up is shown and last page for quiz (so that switching between them is fast).

### AdLib

* From technical perspective, to program AdLib/OPL2 one just needs to write registers to it that set each
voice's synthesis parameters and trigger notes.
* I'm using IMF music format which contains raw registers to write to and delays between those writes, making
playing music extremely easy.
* This all happens inside the overridden timer interrupt (560 times/sec) which also handles timing for waits etc.
* And just like is common with IMF, voice 0 is reserved for sound effects. MIDI2IMF was used to convert
the music into IMF format.
* One quirk is that music file is >64k but rather than use huge pointers and 32-bit offsets everywhere, I decided
to split it into two parts and alternate between them.

### Assembly

* In-line assembly works quite well and you can tweak it, for example remember to explicitly mark jumps
as short otherwise compiler will default to near jumps and write a lot of nops to pad the instruction.
* The need for labels to be outside asm blocks is quirky but it doesn't generate unnecessary instructions.
* While this didn't make it into the final version, I had at some point a few .ASM files, compiled by TASM and
linked into the final .EXE, making it possible to call pure assembly routines from C with no problem.
* Borland C++ allows one to avoid assembly a lot, it has intrinsics for writing registers, calling as well as
overriding interrupts and so on.
* However hand-crafting the hot path in assembly turned out to be much faster for things like copying
between/within RAM and VRAM, decompressing assets (just RLE) or drawing text.

### Mouse

* Mouse deserves a mention because it's great on paper - the mouse driver handles cursor movement and
will even _draw the cursor_ for you.
* However it's a software cursor (writes to the same video memory as you) and it can decide to redraw
it at any moment, in an interrupt handler.
* Furthermore, on EGA (unlike on VGA) video registers cannot be read so the driver can change some
register values you were in the middle of using.
* This means if you're drawing anything you pretty much have to hide the cursor.
* DOSBox virtualizes the mouse driver so everything looks smooth but on real hardware or 86Box the cursor will
flicker quite a bit, but that's ok. I tried to optimise so drawing happens as soon after vsync,
but that only helped a bit.

### Borland C++

* While it's fun and nostalgic, the compiler is a bit buggy, especially when optimisations are enabled.
* I found at least two instances where a silly workaround was required like changing a type or order of instructions
to get the compiler to produce correct output, the option to generate assembly sources and/or IDA are your friends.
* The warnings are not exhaustive, you can mix up near and far pointers and not be told about it.
* Built-in BGI graphics library is great for prototyping but _terrible_ for performance. I used
it early on and tore out eventually.
* Some of the `_dos_*` functions were extremely slow under DOSBox and I had to rewrite them.

### Development setup

* I used VSCode for just code editing.
* For testing, I had a running DOSBox instance with code folder mounted and batch files to compile and run.
* This means I wasn't able to interactively debug at all (using Borland C++ IDE stopped working early on, I
must have upset it with some direct assembly code).
* Luckily restarting DOSBox after a crash takes just a few seconds :) And print statements are your friends.

### Resources used

* [Borland C++ Programmers Guide](https://bitsavers.trailing-edge.com/pdf/borland/borland_C++/Borland_C++_Version_3.1_Programmers_Guide_1992.pdf)
* [Michael Abrash's Graphics Programming Black Book](https://www.jagregory.com/abrash-black-book/)
* [Cosmodoc](https://cosmodoc.org/)
* [Tech Help Manual](http://www.techhelpmanual.com/3-tech_topics.html)
