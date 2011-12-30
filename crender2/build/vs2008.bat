@echo off
mkdir vs2008
cd vs2008
copy ..\*.lua .\
..\premake4.exe vs2008
@pause