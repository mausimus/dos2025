@echo off
del bin\*.exe
del bin\*.obj
cd src
cls
make -s
cd ..
