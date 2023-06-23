#pragma once
#include <gfx/GFXRenderTarget.h>
#include "VulkanInclude.h"
#include "GFXVulkanTexture2D.h"

namespace gfx
{
	class GFXVulkanApplication;

	class GFXVulkanRenderTarget : public GFXRenderTarget
	{
	public:

	public:
		GFXVulkanRenderTarget(GFXVulkanApplication* app, GFXVulkanTexture2D* tex, VkFormat colorFormat);
		virtual ~GFXVulkanRenderTarget() override;
	public:
		VkImage GetVkImage() const { return m_tex2d->GetVkImage(); }


		GFXVulkanTexture2D* GetTexture() { return m_tex2d; }
		void InitRenderPass();

		VkImageLayout GetVkImageLayout() const { return m_tex2d->GetVkImageLayout(); }
	protected:
		//VkImage m_image = VK_NULL_HANDLE;
		//VkImageView m_imageView = VK_NULL_HANDLE;
		//VkImageLayout m_imageLayout{};
		//VkDeviceMemory m_imagememory = VK_NULL_HANDLE;
		VkRenderPass m_renderPass = VK_NULL_HANDLE;

		GFXVulkanTexture2D* m_tex2d;

		GFXVulkanApplication* m_app;
	};
}