@echo off
@REM Call this batch with your target python environment active
@REM to launch Visual Studio with appropriate environment variable values.
@REM - PYTHON_INCLUDE_DIR
@REM - PYTHON_LIBS_DIR
setlocal

pushd %~dp0

set PYTHON_INCLUDE_DIR="E:\Program Files\Python\3.6\include"
set PYTHON_LIBS_DIR="E:\Program Files\Python\3.6\libs"

start python_binding.sln

popd
endlocal
