@echo off
@set IDE=vs2008
@if not exist %IDE% mkdir %IDE%
@cd %IDE%
@xcopy /q /y ..\premake4\*.lua .\
..\premake4\premake4.exe %IDE%
@pause