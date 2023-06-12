#include "GFXVulkanDescriptorSet.h"
#include "GFXVulkanDescriptorSet.h"
#include "GFXVulkanApplication.h"
#include "GFXVulkanBuffer.h"
#include "GFXVulkanTexture2D.h"
#include <cassert>
#include <stdexcept>

namespace gfx
{

    static VkDescriptorType _GetDescriptorType(GFXDescriptorType type)
    {
        switch (type)
        {
        case gfx::GFXDescriptorType::ConstantBuffer:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            break;
        case gfx::GFXDescriptorType::CombinedImageSampler:
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            break;
        default:
            assert(false);
            break;
        }
        return {};
    }

    GFXVulkanDescriptorSetLayout::GFXVulkanDescriptorSetLayout(
        GFXVulkanApplication* app,
        const std::vector<GFXDescriptorSetLayoutInfo>& layout
    )
        : base(layout), m_app(app)
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        for (size_t i = 0; i < m_layout.size(); i++)
        {
            auto& layout = m_layout[i];
            VkDescriptorSetLayoutBinding binding{};
            binding.binding = layout.BindingPoint;
            binding.descriptorType = _GetDescriptorType(layout.Type);
            binding.descriptorCount = 1;
            binding.stageFlags = (uint32_t)layout.Stage;
            binding.pImmutableSamplers = nullptr;

            bindings.push_back(binding);
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(app->GetVkDevice(), &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    GFXVulkanDescriptorSetLayout::~GFXVulkanDescriptorSetLayout()
    {
        if (m_descriptorSetLayout != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorSetLayout(m_app->GetVkDevice(), m_descriptorSetLayout, nullptr);
            m_descriptorSetLayout = VK_NULL_HANDLE;
        }
    }

    std::shared_ptr<GFXVulkanDescriptorSet> GFXVulkanDescriptorSetLayout::CreateVkDescriptorSet()
    {
        return std::shared_ptr<GFXVulkanDescriptorSet>(new GFXVulkanDescriptorSet(m_app, this));
    }


    void GFXVulkanDescriptor::SetConstantBuffer(size_t size, GFXBuffer* buffer)
    {
        auto vkBuffer = static_cast<GFXVulkanBuffer*>(buffer);

        BufferInfo.buffer = vkBuffer->GetVkBuffer();
        BufferInfo.offset = 0;
        BufferInfo.range = size;

        VkWriteDescriptorSet write;
        WriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        WriteInfo.dstSet = m_descriptorSet->GetVkDescriptorSet();
        WriteInfo.dstBinding = m_bindingPoint;
        WriteInfo.dstArrayElement = 0;
        WriteInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        WriteInfo.descriptorCount = 1;
        WriteInfo.pBufferInfo = &BufferInfo;

    }
    void GFXVulkanDescriptor::SetTextureSampler2D(GFXTexture2D* texture)
    {
        auto vkTex2d = static_cast<GFXVulkanTexture2D*>(texture);

        ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        ImageInfo.imageView = vkTex2d->GetVkImageView();
        ImageInfo.sampler = vkTex2d->GetVkSampler();

        WriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        WriteInfo.dstSet = m_descriptorSet->GetVkDescriptorSet();
        WriteInfo.dstBinding = m_bindingPoint;
        WriteInfo.dstArrayElement = 0;
        WriteInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        WriteInfo.descriptorCount = 1;
        WriteInfo.pImageInfo = &ImageInfo;
    }


    GFXVulkanDescriptorSet::GFXVulkanDescriptorSet(GFXVulkanApplication* app, GFXDescriptorSetLayout* layout)
        : m_app(app)
    {
        m_setlayout = static_cast<GFXVulkanDescriptorSetLayout*>(layout);

        auto VkSetLayout = m_setlayout->GetVkDescriptorSetLayout();

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = app->GetVkDescriptorPool();
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &VkSetLayout;


        auto result = vkAllocateDescriptorSets(app->GetVkDevice(), &allocInfo, &m_descriptorSet);
        bool a = result == VK_ERROR_OUT_OF_POOL_MEMORY;
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }
    }

    GFXVulkanDescriptorSet::~GFXVulkanDescriptorSet()
    {

        if (m_descriptorSet != VK_NULL_HANDLE)
        {
            vkFreeDescriptorSets(m_app->GetVkDevice(), m_app->GetVkDescriptorPool(), 1, &m_descriptorSet);
            m_descriptorSet = VK_NULL_HANDLE;
        }
    }

    GFXVulkanDescriptor* GFXVulkanDescriptorSet::AddDescriptor(uint32_t bindingPoint)
    {
        auto descriptor = new GFXVulkanDescriptor(this, bindingPoint);
        m_descriptors.push_back(descriptor);
        return descriptor;
    }
    void GFXVulkanDescriptorSet::Submit()
    {
        std::vector<VkWriteDescriptorSet> writeInfos;
        for (auto& descriptor : m_descriptors)
        {
            writeInfos.push_back(descriptor->WriteInfo);
        }
        vkUpdateDescriptorSets(m_app->GetVkDevice(), writeInfos.size(), writeInfos.data(), 0, nullptr);
    }



}