mkdir ..\out\build\x64-debug\shader\
dxc -spirv -T vs_6_0 -E main Lit.vs.hlsl -Fo ..\out\build\x64-debug\shader\Lit.vs.spv
dxc -spirv -T ps_6_0 -E main Lit.ps.hlsl -Fo ..\out\build\x64-debug\shader\Lit.ps.spv
pause