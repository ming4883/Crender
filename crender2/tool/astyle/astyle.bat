@echo off
@set SRC_DIR=../../../src
@set OPTIONS=--style=ansi --indent=tab --pad-oper --pad-paren-in --pad-header --lineend=linux --suffix=none
@cd bin

astyle %OPTIONS% %SRC_DIR%/core/*.cpp %SRC_DIR%/core/*.h
astyle %OPTIONS% %SRC_DIR%/core/private/*.cpp %SRC_DIR%/core/private/*.h

astyle %OPTIONS% %SRC_DIR%/framework/*.cpp %SRC_DIR%/framework/*.h
astyle %OPTIONS% %SRC_DIR%/framework/private/*.h

astyle %OPTIONS% --recursive ../../../test/*.cpp ../../../test/*.h

@pause