#include "GFXVulkanRenderer.h"
#include "GFXVulkanApplication.h"
#include "GFXVulkanRenderPass.h"
#include "GFXVulkanCommandBuffer.h"

#include <array>

namespace gfx
{
    GFXVulkanRenderer::GFXVulkanRenderer(GFXVulkanApplication* app)
        : m_app(app)
    {
        m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            if (vkCreateSemaphore(app->GetVkDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(app->GetVkDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(app->GetVkDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }

    }
    void GFXVulkanRenderer::Render()
    {

        vkWaitForFences(m_app->GetVkDevice(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        auto result = m_app->GetVulkanViewport()->AcquireNextImage(m_imageAvailableSemaphores[m_currentFrame], &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            m_app->GetVulkanViewport()->ReInitSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        vkResetFences(m_app->GetVkDevice(), 1, &m_inFlightFences[m_currentFrame]);

        auto cmdBuffer = static_cast<GFXVulkanCommandBuffer*>(m_app->GetVulkanViewport()->GetVkCommandBuffer());

        cmdBuffer->m_imageSemaphore = m_imageAvailableSemaphores[m_currentFrame];
        cmdBuffer->m_renderSemaphore = m_renderFinishedSemaphores[m_currentFrame];
        cmdBuffer->m_inFlightFence = m_inFlightFences[m_currentFrame];

        cmdBuffer->Begin();

        RecordCommandBuffer(cmdBuffer, { m_renderTarget });

        cmdBuffer->End();

        //VkSubmitInfo submitInfo{};
        //submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        //VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
        //VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        //submitInfo.waitSemaphoreCount = 1;
        //submitInfo.pWaitSemaphores = waitSemaphores;
        //submitInfo.pWaitDstStageMask = waitStages;

        //submitInfo.commandBufferCount = 1;
        //submitInfo.pCommandBuffers = &m_app->GetVkCommandBuffer(m_currentFrame);

        //VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };
        //submitInfo.signalSemaphoreCount = 1;
        //submitInfo.pSignalSemaphores = signalSemaphores;

        //if (vkQueueSubmit(m_app->GetVkGraphicsQueue(), 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS)
        //{
        //    throw std::runtime_error("failed to submit draw command buffer!");
        //}

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        VkSwapchainKHR swapChains[] = { m_app->GetVulkanViewport()->GetVkSwapChain()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

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

        m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void GFXVulkanRenderer::RecordCommandBuffer(
        GFXCommandBuffer* cmdBuffer, 
        const std::vector<GFXRenderTarget*>& renderTarget)
    {
        auto cmd = static_cast<GFXVulkanCommandBuffer*>(cmdBuffer);
        auto rt = static_cast<GFXVulkanRenderTarget*>(renderTarget[0]);
        cmd->BeginRenderTarget(rt);
        cmd->CmdSetViewport(0, 0, 1280, 720);
        cmd->CmdClearColor(1, 0, 1, 1);
        cmd->EndRenderTarget();

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