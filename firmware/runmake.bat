@set MAKE=make.exe

@cd /d %~dp0
::@%MAKE% clean
@echo CPUs: %NUMBER_OF_PROCESSORS%
@%MAKE% -j %NUMBER_OF_PROCESSORS%
@if %errorlevel%==0 %MAKE% upload
pause
