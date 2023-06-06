mkdir ..\out\build\x64-debug\shader\
dxc -spirv -T vs_6_0 -E main lit.vert.hlsl -Fo ..\out\build\x64-debug\shader\lit.vert.spv
dxc -spirv -T ps_6_0 -E main lit.pixel.hlsl -Fo ..\out\build\x64-debug\shader\lit.pixel.spv
pause