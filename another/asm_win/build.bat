del %1.exe
tasm /zn /w2 %1.asm
tlink /x /3 /Gn %1.obj
del %1.obj
REM %1.exe