#include <gfx-vk/GFXVulkanCommandBuffer.h>
#include <gfx-vk/GFXVulkanApplication.h>

namespace gfx
{
    void GFXVulkanCommandBuffer::Begin()
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_app->GetVkCommandPool();
        allocInfo.commandBufferCount = 1;

        vkAllocateCommandBuffers(m_app->GetVkDevice(), &allocInfo, &m_cmdBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(m_cmdBuffer, &beginInfo);


        //VkRenderPassBeginInfo renderPassInfo{};
        //renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        //renderPassInfo.renderPass = m_app->GetVkRenderPass();
        //renderPassInfo.framebuffer = m_app->GetVkFrameBuffers()[imageIndex];
        //renderPassInfo.renderArea.offset = { 0, 0 };
        //renderPassInfo.renderArea.extent = gfxapp->GetVkSwapChainExtent();

        //std::array<VkClearValue, 2> clearValues{};
        //clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        //clearValues[1].depthStencil = { 1.0f, 0 };

        //renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        //renderPassInfo.pClearValues = clearValues.data();

        //vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }
    void GFXVulkanCommandBuffer::End()
    {
        vkEndCommandBuffer(m_cmdBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_cmdBuffer;

        vkQueueSubmit(m_app->GetVkGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_app->GetVkGraphicsQueue());

        vkFreeCommandBuffers(m_app->GetVkDevice(), m_app->GetVkCommandPool(), 1, &m_cmdBuffer);
    }

    void GFXVulkanCommandBuffer::CmdClear(float r, float g, float b, float a, bool depth, bool stencil)
    {
        VkClearValue colorValue;
        colorValue.color = { {r, g, b, a} };
        VkClearValue depthValue;

    }
    void GFXVulkanCommandBuffer::CmdBindPipeline(GFXGraphicsPipeline* pipeline) {}
    void GFXVulkanCommandBuffer::CmdBindVertexBuffers() {}
    void GFXVulkanCommandBuffer::CmdBindIndexBuffer() {}
    void GFXVulkanCommandBuffer::CmdBindDescriptorSets() {}
    void GFXVulkanCommandBuffer::CmdDrawIndexed() {}

}