#pragma once
#include <gfx/GFXRenderTarget.h>
#include "VulkanInclude.h"
#include "GFXVulkanTexture2D.h"
#include <cassert>

namespace gfx
{
	class GFXVulkanApplication;

	class GFXVulkanRenderTarget : public GFXRenderTarget
	{
		using base = GFXRenderTarget;
	public:
		/**
		* render target view
		*/
		GFXVulkanRenderTarget(GFXVulkanApplication* app, GFXVulkanTexture2D* tex, VkFormat colorFormat, GFXVulkanTexture2D* depth, VkFormat depthFormat);
		virtual ~GFXVulkanRenderTarget() override;
	public:
		VkImage GetVkColorImage() const { return m_tex2d ? m_tex2d->GetVkImage() : VK_NULL_HANDLE; }
		VkImageView GetVkColorImageView() const { return m_tex2d ? m_tex2d->GetVkImageView() : VK_NULL_HANDLE; }
		VkImageLayout GetVkColorImageLayout() const { assert(m_tex2d); return m_tex2d->GetVkImageLayout(); }

		VkImage GetVkDepthImage() const { return m_depthTex ? m_depthTex->GetVkImage() : VK_NULL_HANDLE; }
		VkImageView GetVkDepthImageView() const { return m_depthTex ? m_depthTex->GetVkImageView() : VK_NULL_HANDLE; }

		VkRenderPass GetVkRenderPass() const { return m_renderPass; }
		VkFramebuffer GetVkFrameBuffer() const { return m_frameBuffer; }

		VkExtent2D GetVkExtent() const { return { (uint32_t)m_width, (uint32_t)m_height }; }

		virtual int32_t GetWidth() const override { return m_width; }
		virtual int32_t GetHeight() const override { return m_height; }
		
	protected:
		void InitRenderPass();
		void TermRenderPass();
	protected:
		VkRenderPass m_renderPass = VK_NULL_HANDLE;
		VkFramebuffer m_frameBuffer = VK_NULL_HANDLE;

		GFXVulkanTexture2D* m_tex2d;
		GFXVulkanTexture2D* m_depthTex;

		GFXVulkanApplication* m_app;

		int32_t m_width;
		int32_t m_height;
	};
}