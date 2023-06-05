#include <gfx/GFXApplication.h>
#include "PlatformImpl/Vulkan/GFXVulkanApplication.h"

namespace gfx
{

    GFXApplication* CreateGFXApplication(GFXApi Api, GFXGlobalConfig config)
    {
        if (Api == GFXApi::Vulkan)
        {
            return new GFXVulkanApplication(config);
        }
        return nullptr;
    }
}