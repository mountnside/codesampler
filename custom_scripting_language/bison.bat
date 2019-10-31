@echo off

rem Run Bison to generate the parser
bison --defines --verbose -o parse.cpp my_c.y

rem Clobber lexSymbol.h
if exist lexSymbol.h del lexSymbol.h

rem Rename parse.cpp.h to lexSymbol.h
ren parse.cpp.h lexSymbol.h

:End
