#pragma once
#include <gfx/GFXRenderTarget.h>
#include "VulkanInclude.h"

namespace gfx
{
	class GFXVulkanApplication;

	class GFXVulkanRenderTarget : public GFXRenderTarget
	{
	public:

	public:
		GFXVulkanRenderTarget(GFXVulkanApplication* app, VkFormat colorFormat);
		virtual ~GFXVulkanRenderTarget() override;
	public:
		VkImage GetVkImage() const { return m_image; }
		VkImageLayout GetVkImageLayout() const { return m_imageLayout; }

	protected:
		VkImage m_image = VK_NULL_HANDLE;
		VkImageView m_imageView = VK_NULL_HANDLE;
		VkImageLayout m_imageLayout{};
		VkDeviceMemory m_imagememory = VK_NULL_HANDLE;
		VkRenderPass m_renderPass = VK_NULL_HANDLE;

		GFXVulkanApplication* m_app;
	};
}