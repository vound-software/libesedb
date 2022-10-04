call "c:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat"

@echo --------------------------------
@echo --- Building libesedb-x86.dll ---
@echo --------------------------------
msbuild msvscpp\libesedb.sln /verbosity:minimal /t:libesedb /p:Configuration=Release;Platform=Win32

@echo --------------------------------
@echo --- Building libesedb-x64.dll ---
@echo --------------------------------
msbuild msvscpp\libesedb.sln /verbosity:minimal /t:libesedb /p:Configuration=Release;Platform=x64