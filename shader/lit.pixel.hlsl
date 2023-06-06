#include "Common.hlsl"

PixelOutAssembly main(PixelInAssembly v2f)
{
    PixelOutAssembly f2p;
    f2p.Color = v2f.Color *0.05;
    return f2p;
}