@echo off

set name="b-rush.nes"

set CC65_HOME=..\

cc65 -Oi b-rush.c --add-source
ca65 crt0.s
ca65 b-rush.s

ld65 -C nrom_128_horz.cfg -o %name% crt0.o b-rush.o nes.lib

pause

del *.o
del b-rush.s

C:\dev\nes\fceux\fceux.exe %name%