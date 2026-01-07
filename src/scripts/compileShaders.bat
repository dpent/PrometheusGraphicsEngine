@echo off
setlocal EnableDelayedExpansion

IF NOT EXIST "..\..\bin" mkdir "..\..\bin"

SET "SHADER_DIR=C:\Users\Dimitris\source\repos\Prometheus\src\shaders"
SET "BUILD_DIR=C:\Users\Dimitris\source\repos\Prometheus\bin"

FOR %%f IN ("%SHADER_DIR%\*") DO (
    IF NOT EXIST "%%f\" (
        SET "filename=%%~nxf"
        SET "extension=%%~xf"
        SET "name=%%~nf"
        SET "output="

        ECHO Processing file: %%f

        IF /I "!extension!"==".vert" SET "output=!name!Vert.spv"
        IF /I "!extension!"==".frag" SET "output=!name!Frag.spv"
        IF /I "!extension!"==".comp" SET "output=!name!Comp.spv"
        IF /I "!extension!"==".geom" SET "output=!name!Geom.spv"
        IF /I "!extension!"==".tesc" SET "output=!name!Tesc.spv"
        IF /I "!extension!"==".tese" SET "output=!name!Tese.spv"

        IF "!output!"=="" (
            ECHO Skipping unknown file type: !filename!
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
)

ECHO All shaders compiled successfully!