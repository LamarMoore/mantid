:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: WINDOWS SCRIPT TO DRIVE THE JENKINS BUILDS OF MANTID.
::
:: Notes:
::
:: WORKSPACE & JOB_NAME are environment variables that are set by Jenkins.
:: BUILD_THREADS & PARAVIEW_DIR should be set in the configuration of each slave.
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: If pvnext in JOB_NAME, use PARAVIEW_NEXT_DIR for PARAVIEW_DIR
:: Also, occasionally pvnext needs a different build type. Get it from 
:: PVNEXT_BUILD_TYPE
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
if NOT "%JOB_NAME%"=="%JOB_NAME:pvnext=%" (
    echo "A: %PVNEXT_BUILD_TYPE%"
    set PARAVIEW_DIR=%PARAVIEW_NEXT_DIR%
    set BUILD_TYPE=%PVNEXT_BUILD_TYPE%
) else (
    set BUILD_TYPE=Release
)

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: Get or update the third party dependencies
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
cd %WORKSPACE%\Code
call fetch_Third_Party win64
cd %WORKSPACE%

set PATH=%WORKSPACE%\Code\Third_Party\lib\win64;%WORKSPACE%\Code\Third_Party\lib\win64\Python27;%PARAVIEW_DIR%\bin\%BUILD_TYPE%;%PATH%

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: Check whether this is a clean build (must have 'clean' in the job name)
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
set DOC_IMAGES=
if "%JOB_NAME%"=="%JOB_NAME:clean=%" (
    set CLEANBUILD=no
) else  (
    set CLEANBUILD=yes
    rmdir /S /Q build
    if NOT "%JOB_NAME%"=="%JOB_NAME:master=%" (
        set DOC_IMAGES=-DQT_ASSISTANT_FETCH_IMAGES=ON
    )
)

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: Create the build directory if it doesn't exist
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
md %WORKSPACE%\build
cd %WORKSPACE%\build

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: CMake configuration
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
"C:\Program Files (x86)\CMake 2.8\bin\cmake.exe" -G "Visual Studio 11 Win64" -DCONSOLE=OFF -DENABLE_CPACK=ON -DMAKE_VATES=ON -DParaView_DIR=%PARAVIEW_DIR% -DUSE_PRECOMPILED_HEADERS=ON %DOC_IMAGES% ..\Code\Mantid

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: Build step
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
msbuild /nologo /m:%BUILD_THREADS% /nr:false /p:Configuration=%BUILD_TYPE% Mantid.sln
if ERRORLEVEL 1 exit /B %ERRORLEVEL%

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: Run the tests
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
"C:\Program Files (x86)\CMake 2.8\bin\ctest.exe" -C %BUILD_TYPE% -j%BUILD_THREADS% --schedule-random --output-on-failure -E MantidPlot
:: Run GUI tests serially
ctest -C %BUILD_TYPE% --output-on-failure -R MantidPlot

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: Create the install kit if this is a clean build
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
if "%CLEANBUILD%" EQU "yes" (
    msbuild /nologo /m:%BUILD_THREADS% /nr:false /p:Configuration=%BUILD_TYPE% docs/qtassistant/qtassistant.vcxproj
    if "%BUILD_TYPE%" == "Release" (
        cpack -C %BUILD_TYPE% --config CPackConfig.cmake
    )
    if "%BUILD_TYPE%" == "Debug" (
        cpack -C %BUILD_TYPE% --config CPackConfig.cmake -DWINDOWS_DEPLOYMENT_TYPE=%BUILD_TYPE%
    )
)
