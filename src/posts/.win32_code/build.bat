@echo OFF

rem this is the build file for windows

set "PROJECT_DIR=C:\Users\myguy\OneDrive\Desktop\James822.github.io\src\posts\.win32_code\"
set "BUILDS_DIR=%PROJECT_DIR%builds\x64_windows\"
set "DEBUG_BUILD_DIR=%BUILDS_DIR%debug\"
set "RELEASE_BUILD_DIR=%BUILDS_DIR%release\"

rem set to "DEBUG" for debug build, "RELEASE" for release build.
set "BUILD_TYPE=DEBUG"




rem EXPLANATION OF COMPILER OPTIONS:
rem ----------------------------------
rem "/Wall":
rem    - turns on all warnings similar to the gcc -Wall option.
rem "/wd4820":
rem    - turns off warning for C4820 which is for alignment with padding bytes being added to a struct
rem "/wd5045":
rem    - turns off warning for C5045 which indicates that the code will be changed if Spectre mitigation is turned on (/Qspectre)

rem EXPLANATION OF LINKER OPTIONS:
rem ----------------------------------
rem "/ENTRY:mainCRTStartup":
rem    - specifies that our entry point is the windows CRT "mainCRTStartup" which calls the normal "main()" function. This is used in conjunction with "/SUBSYSTEM:<type>" so that the subsytem type is not automatically chosen as a default based on our choice of ENTRY.




if "%BUILD_TYPE%"=="DEBUG" (
set "OUTPUT_DIR=%DEBUG_BUILD_DIR%"
)

if "%BUILD_TYPE%"=="RELEASE" (
set "OUTPUT_DIR=%RELEASE_BUILD_DIR%"
)

pushd %OUTPUT_DIR%




if "%BUILD_TYPE%"=="DEBUG" (

echo:
echo ========== DEBUG BUILD ==========
echo:

rem EXPLANATION OF DEBUG COMPILER OPTIONS:
rem ----------------------------------
rem "/Od":
rem    - turns off all optimizations which is good for debugging
rem "/Zi":
rem    - generates debug output in the form of a .pdb file, this flag generates debug information so that we can debug the output executable with "devenv output.exe" for example.

rem EXPLANATION OF DEBUG LINKER OPTIONS:
rem ----------------------------------
rem "/SUBSYSTEM:CONSOLE":
rem    - specifies the subsystem to be building with. We set this to the CONSOLE subsystem so that we can have console output to print to for debugging purposes. When we launch the application exe from a CMD, it will use that CMD for output. If we just launch the application exe otherwise, it will open a CMD window for us and use it for output if no CMD already is attached.

cl /MDd /Od /Zi /Wall /wd4820 /wd5045 /Fe"%OUTPUT_DIR%output" User32.lib Gdi32.lib Opengl32.lib "%PROJECT_DIR%win32_window.c" /link /SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup

)


if "%BUILD_TYPE%"=="RELEASE" (

echo:
echo ========== RELEASE BUILD ==========
echo:

rem EXPLANATION OF RELEASE COMPILER OPTIONS:
rem ----------------------------------

rem EXPLANATION OF RELEASE LINKER OPTIONS:
rem ----------------------------------
rem "/SUBSYSTEM:WINDOWS":
rem	- specifies the subsystem to be building with. We set this to the "WINDOWS" subsystem so that it doesn't auto use the "CONSOLE" subsystem application and thus open a console window when we run the program.

cl /MD /O2 /Wall /wd4820 /wd5045 /Fe"%OUTPUT_DIR%output" User32.lib Gdi32.lib Opengl32.lib "%PROJECT_DIR%win32_window.c" /link /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup

)




rem after we are done compiling/building we return to the prev directory. This is done BEFORE we run the program because otherwise, if we terminate the batch job to end the program (if the program isn't normally responding), then we may not reach the popd and the user will get stuck in the OUTPUT_DIR in their cmd
popd %OUTPUT_DIR%




if %ERRORLEVEL%==0 (
"%OUTPUT_DIR%output.exe"
) else (

echo -------------------------
echo -------------------------
echo -------------------------
echo -------------------------
echo PROGRAM FAILED TO COMPILE!
echo -------------------------
echo -------------------------
echo -------------------------
echo -------------------------
)
