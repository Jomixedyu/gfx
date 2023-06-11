#pragma once
#include "VulkanInclude.h"
#include <gfx/GFXTexture2D.h>

namespace gfx
{
    class GFXVulkanApplication;

    class GFXVulkanTexture2D : public GFXTexture2D
    {
    public:
        using DataDeleter = void(*)(uint8_t*);
    public:
        static std::shared_ptr<GFXVulkanTexture2D> CreateFromMemory(
            GFXVulkanApplication* app, const uint8_t* fileData, int32_t length, bool enableReadWrite, GFXTextureFormat format);
        //static std::shared_ptr<GFXVulkanTexture2D> CreateFromFile(
        //    GFXVulkanApplication* app, const char* filename, bool enableReadWrite, GFXTextureChannelMode mode);

    public:
        virtual ~GFXVulkanTexture2D() override;
        void Init(
            GFXVulkanApplication* app, 
            const uint8_t* imageData,
            int32_t width, int32_t height, int32_t channel, 
            VkFormat format,
            bool enableReadWrite);

    public:
        virtual const uint8_t* GetData() const override;
    public:

        VkImage GetVkImage() const { return m_textureImage; }
        VkImageView GetVkImageView() const { return m_textureImageView; }
        VkSampler GetVkSampler() const { return m_textureSampler; }
        VkFormat GetVkImageFormat() const { return m_imageFormat; }
    protected:
        GFXVulkanApplication* m_app;

        VkImage m_textureImage;
        VkDeviceMemory m_textureImageMemory;
        VkImageView m_textureImageView;
        VkSampler m_textureSampler;
        VkFormat m_imageFormat;

        bool m_inited = false;
    };
}