#pragma once
#include <gfx/GFXDescriptorSet.h>
#include "VulkanInclude.h"

namespace gfx
{
    class GFXVulkanApplication;
    class GFXVulkanDescriptorSet;

    class GFXVulkanDescriptorSetLayout : public GFXDescriptorSetLayout
    {
        using base = GFXDescriptorSetLayout;
    public:
        GFXVulkanDescriptorSetLayout(
            GFXVulkanApplication* app,
            const std::vector<GFXDescriptorSetLayoutInfo>& layout);

        virtual ~GFXVulkanDescriptorSetLayout() override;
    public:
        std::shared_ptr<GFXVulkanDescriptorSet> CreateVkDescriptorSet();
    public:
        const VkDescriptorSetLayout& GetVkDescriptorSetLayout() const { return m_descriptorSetLayout; }

    protected:
        VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
        GFXVulkanApplication* m_app;
    };

    class GFXVulkanDescriptor : public GFXDescriptor
    {
    public:

        GFXVulkanDescriptor(GFXVulkanDescriptorSet* set, uint32_t bindingPoint)
            : m_descriptorSet(set), m_bindingPoint(bindingPoint)
        {

        }
        virtual void SetConstantBuffer(size_t size, GFXBuffer* buffer) override;
        virtual void SetTextureSampler2D(GFXTexture2D* texture) override;
    public:
        VkDescriptorBufferInfo BufferInfo{};
        VkDescriptorImageInfo ImageInfo{};
        VkWriteDescriptorSet WriteInfo{};
    protected:
        GFXVulkanDescriptorSet* m_descriptorSet;
        uint32_t m_bindingPoint;
    };


    class GFXVulkanDescriptorSet : public GFXDescriptorSet
    {
        using base = GFXDescriptorSet;
    public:
        GFXVulkanDescriptorSet(GFXVulkanApplication* app, GFXDescriptorSetLayout* layout);
        virtual ~GFXVulkanDescriptorSet() override;
        GFXVulkanDescriptorSet(const GFXVulkanDescriptorSet&) = delete;
    public:
        GFXVulkanDescriptor* AddDescriptor(uint32_t bindingPoint);
        void Submit();
    public:
        GFXVulkanApplication* GetApplication() const { return m_app; }
        VkDescriptorSet GetVkDescriptorSet() const { return m_descriptorSet; }
    protected:
        std::vector<GFXVulkanDescriptor*> m_descriptors;
        VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
        GFXVulkanDescriptorSetLayout* m_setlayout;
        GFXVulkanApplication* m_app;
    };
}