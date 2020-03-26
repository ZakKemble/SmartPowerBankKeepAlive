
:: Set COMPILER to the location of AVR-GCC bin folder
:: File path must not contain spaces!
@set COMPILER=

@set MAKE=%COMPILER%make.exe
@set AVRDUDE=%COMPILER%avrdude.exe

@cd /d %~dp0
::@%MAKE% clean
@echo CPUs: %NUMBER_OF_PROCESSORS%
@%MAKE% -j %NUMBER_OF_PROCESSORS% COMPILER=%COMPILER% AVRDUDE=%AVRDUDE%
@if %errorlevel%==0 %MAKE% upload COMPILER=%COMPILER% AVRDUDE=%AVRDUDE%
pause
