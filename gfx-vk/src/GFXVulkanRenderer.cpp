#include "GFXVulkanRenderer.h"
#include "GFXVulkanApplication.h"
#include "GFXVulkanRenderPass.h"
#include "GFXVulkanCommandBuffer.h"
#include "GFXVulkanQueue.h"

#include <array>

namespace gfx
{

    GFXVulkanRenderer::GFXVulkanRenderer(GFXVulkanApplication* app)
        : m_app(app)
    {
    }
    void GFXVulkanRenderer::Render(float deltaTime)
    {
        auto viewport = m_app->GetVulkanViewport();

        vkWaitForFences(m_app->GetVkDevice(), 1, &viewport->GetQueue()->GetVkFence(), VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        auto result = m_app->GetVulkanViewport()->AcquireNextImage(&imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            m_app->GetVulkanViewport()->ReInitSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        vkResetFences(m_app->GetVkDevice(), 1, &viewport->GetQueue()->GetVkFence());

        GFXVulkanRenderContext renderContext(m_app);

        renderContext.SetQueue(viewport->GetQueue());
        renderContext.DeltaTime = deltaTime;

        auto renderTargets = std::vector{ viewport->GetRenderTarget() };
        m_app->GetRenderPipeline()->OnRender(&renderContext, renderTargets);

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        VkSwapchainKHR swapChains[] = { m_app->GetVulkanViewport()->GetVkSwapChain() };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pWaitSemaphores = &viewport->GetQueue()->GetVkSignalSemaphore();
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(m_app->GetVkPresentQueue(), &presentInfo);
        vkQueueWaitIdle(m_app->GetVkPresentQueue());

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized)
        {
            m_framebufferResized = false;
            m_app->GetVulkanViewport()->ReInitSwapChain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swap chain image!");
        }

    }

    void GFXVulkanRenderer::RecordCommandBuffer(
        GFXCommandBuffer* cmdBuffer,
        const std::vector<GFXRenderTarget*>& renderTarget)
    {
        auto cmd = static_cast<GFXVulkanCommandBuffer*>(cmdBuffer);
        auto rt = static_cast<GFXVulkanRenderTarget*>(renderTarget[0]);

        cmd->SetRenderTarget(rt);

        cmd->CmdClearColor(1, 0, 1, 1);
        cmd->CmdBeginRenderTarget();
        cmd->CmdSetViewport(0, 0, 1280, 720);
        cmd->CmdEndRenderTarget();

        cmd->SetRenderTarget(nullptr);

        //        GFXVulkanCommandBuffer* commandBuffer = static_cast<GFXVulkanCommandBuffer*>(cmdBuffer);
        //        VkRenderPassBeginInfo renderPassInfo{};
        //        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        //        renderPassInfo.renderPass = m_app->GetSwapChainRenderPass()->GetVkRenderPass();
        //        renderPassInfo.framebuffer = m_app->GetVkFrameBuffers()[imageIndex];
        //        renderPassInfo.renderArea.offset = { 0, 0 };
        //        renderPassInfo.renderArea.extent = m_app->GetVkSwapChainExtent();
        //
        //        std::array<VkClearValue, 2> clearValues{};
        //        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        //        clearValues[1].depthStencil = { 1.0f, 0 };
        //
        //        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        //        renderPassInfo.pClearValues = clearValues.data();
        //
        //        vkCmdBeginRenderPass(commandBuffer->GetVkCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        //        {
        //            vkCmdBindPipeline(commandBuffer->GetVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->GetVkPipeline());
        //
        //            VkViewport viewport{};
        //#if VULKAN_REVERT_VIEWPORT
        //            viewport.x = 0.0f;
        //            viewport.y = 0.0f + gfxapp->GetVkSwapChainExtent().height;
        //            viewport.width = (float)gfxapp->GetVkSwapChainExtent().width;
        //            viewport.height = -(float)gfxapp->GetVkSwapChainExtent().height;
        //#else
        //            viewport.x = 0.0f;
        //            viewport.y = 0.0f;
        //            viewport.width = (float)m_app->GetVkSwapChainExtent().width;
        //            viewport.height = (float)m_app->GetVkSwapChainExtent().height;
        //#endif
        //            viewport.minDepth = 0.0f;
        //            viewport.maxDepth = 1.0f;
        //
        //            vkCmdSetViewport(commandBuffer->GetVkCommandBuffer(), 0, 1, &viewport);
        //
        //            VkRect2D scissor{};
        //            scissor.offset = { 0, 0 };
        //            scissor.extent = m_app->GetVkSwapChainExtent();
        //            vkCmdSetScissor(commandBuffer->GetVkCommandBuffer(), 0, 1, &scissor);
        //
        //            //vkCmdDraw(commandBuffer, 3, 1, 0, 0);
        //
        //            vkCmdBindPipeline(commandBuffer->GetVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetVkPipeline());
        //
        //            VkBuffer vertexBuffers[] = { static_cast<gfx::GFXVulkanBuffer*>(vertexBuffer)->GetVkBuffer() };
        //            VkDeviceSize offsets[] = { 0 };
        //            vkCmdBindVertexBuffers(commandBuffer->GetVkCommandBuffer(), 0, 1, vertexBuffers, offsets);
        //            vkCmdBindIndexBuffer(commandBuffer->GetVkCommandBuffer(), indexBuffer->GetVkBuffer(), 0, VK_INDEX_TYPE_UINT16);
        //            //vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
        //            auto descriptorSet = descriptorSets[0]->GetVkDescriptorSet();
        //            vkCmdBindDescriptorSets(commandBuffer->GetVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetVkPipelineLayout(), 0, 1, &descriptorSet, 0, nullptr);
        //            vkCmdDrawIndexed(commandBuffer->GetVkCommandBuffer(), static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
        //        }
        //        vkCmdEndRenderPass(commandBuffer->GetVkCommandBuffer());

    }

    GFXVulkanRenderer::~GFXVulkanRenderer()
    {

    }
}