#include <gfx/GFXBuffer.h>
#include <vulkan/vulkan.h>

namespace gfx
{
    class GFXVulkanApplication;

    class GFXVulkanBuffer : public GFXBuffer
    {
    public:
        GFXVulkanBuffer(GFXVulkanApplication* app);
        virtual ~GFXVulkanBuffer() override;
    public:
        virtual void Fill(GFXBufferUsage usage, const void* data, size_t size) override;
        virtual void Release() override;
        const VkBuffer& GetVkBuffer() const { return m_vkBuffer; }
        GFXVulkanApplication* GetApplication() const { return m_app; }
    public:
        /* GFXBuffer */
        virtual bool IsValid() const override;
        virtual size_t GetSize() const override { return this->m_bufferSize; }
    public:

    protected:
        size_t m_bufferSize = 0;
        bool m_hasData = false;
        VkBuffer m_vkBuffer;
        VkDeviceMemory m_vkBufferMemory;
        GFXVulkanApplication* m_app;
    };
}