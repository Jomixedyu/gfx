#pragma once
#include "GFXGlobalConfig.h"
#include "GFXExtensions.h"
#include "GFXBuffer.h"
#include "GFXDescriptor.h"
#include "GFXCommandBuffer.h"
#include "GFXTexture2D.h"
#include "GFXVertexLayoutDescription.h"
#include "GFXImage.h"

#include <functional>
#include <memory>

namespace gfx
{
    enum class GFXApi
    {
        Unknown,
        D3D12,
        Vulkan,
    };

    class GFXApplication
    {
    public:
        using LoopEvent = std::function<void(float)>;
        //using LoopEvent = void(*)(GFXApplication*, float);
        using ExitWindowEvent = bool(*)();
    public:
        virtual void Initialize() {}
        virtual void ExecLoop() {}
        virtual void RequestStop() {}
        virtual void Terminate() {}

        const GFXGlobalConfig& GetConfig() const { return m_config; }
        virtual GFXExtensions GetExtensionNames() = 0;

        LoopEvent OnLoop = nullptr;
        ExitWindowEvent OnExitWindow = nullptr;
    public:
        virtual GFXBuffer* CreateBuffer(GFXBufferUsage usage, size_t bufferSize) = 0;
        virtual std::shared_ptr<GFXCommandBuffer> CreateCommandBuffer() = 0;
        virtual std::shared_ptr<GFXVertexLayoutDescription> CreateVertexLayoutDescription() = 0;
        virtual std::shared_ptr<GFXImage> CreateImage() = 0;

        virtual std::shared_ptr<GFXTexture2D> CreateTexture2DFromMemory(
            const uint8_t* data, int32_t length,
            bool enableReadWrite = false, GFXTextureFormat format = GFXTextureFormat::R8G8B8A8_SRGB) = 0;

        virtual intptr_t GetWindowHandle() = 0;
    protected:
        GFXApplication() {}
    protected:
        GFXGlobalConfig m_config{};
    };



    GFXApplication* CreateGFXApplication(GFXApi Api, GFXGlobalConfig config);
}