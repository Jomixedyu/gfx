cd shader
call compile.bat
cd ..
xcopy textures\ out\build\x64-Debug\textures\ /E /Y
pause