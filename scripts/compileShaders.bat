@echo off
SETLOCAL ENABLEDELAYEDEXPANSION

REM Create build directory if it doesn't exist
IF NOT EXIST ..\build mkdir ..\build

SET SHADER_DIR=C:\Users\Dimitris\source\repos\PrometheusGraphicsEngine\shaders
SET BUILD_DIR=C:\Users\Dimitris\source\repos\PrometheusGraphicsEngine\build

REM Loop through all shader files
FOR %%f IN (%SHADER_DIR%\*) DO (
    SET "filename=%%~nxf"
    SET "extension=%%~xf"
    SET "name=%%~nf"
    SET "output="

    ECHO Processing file: %%~f

    REM Determine shader type
    IF /I "!extension!"==".vert" SET "output=!name!Vert.spv"
    IF /I "!extension!"==".frag" SET "output=!name!Frag.spv"
    IF /I "!extension!"==".comp" SET "output=!name!Comp.spv"
    IF /I "!extension!"==".geom" SET "output=!name!Geom.spv"
    IF /I "!extension!"==".tesc" SET "output=!name!Tesc.spv"
    IF /I "!extension!"==".tese" SET "output=!name!Tese.spv"

    REM Skip unknown shader types
    IF "!output!"=="" (
        ECHO Unknown shader type: !filename!
    ) ELSE (
        "C:\Users\Dimitris\Documents\Libraries\VulkanSDK\1.4.335.0\Bin\glslc.exe" "%%f" -o "%BUILD_DIR%\!output!"
        IF ERRORLEVEL 1 (
            ECHO Failed to compile !filename!
            EXIT /B 1
        ) ELSE (
            ECHO SUCCESS
        )
    )   
)

ECHO All shaders compiled successfully!
