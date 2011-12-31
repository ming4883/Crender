@echo off
@mkdir vs2008
@cd vs2008
@copy ..\premake4\*.lua .\
..\premake4.exe vs2008
@pause