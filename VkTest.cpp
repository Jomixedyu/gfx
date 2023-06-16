
#define GLFW_INCLUDE_VULKAN
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
//#include <vulkan/vulkan_win32.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <gfx-vk/GFXThirdParty/glfw/include/GLFW/glfw3.h>
#include <gfx-vk/GFXThirdParty/glfw/include/GLFW/glfw3native.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#undef max
#undef min

#include <gfx/GFXDefined.h>
#include <gfx/GFXCommandBufferScope.h>
#include <gfx/GFXShaderModule.h>
#include <gfx/GFXGraphicsPipeline.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>
#include <array>
#define GLM_CLIP_CONTROL_LH_BIT 1
#define GLM_CONFIG_CLIP_CONTROL 1
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <gfx-vk/GFXVulkanCommandBuffer.h>
#include <gfx/GFXApplication.h>
#include <gfx-vk/GFXVulkanApplication.h>
#include <gfx-vk/GFXVulkanBuffer.h>
#include <gfx-vk/GFXVulkanVertexLayoutDescription.h>
#include <gfx-vk/GFXVulkanTexture2D.h>
#include <gfx-vk/GFXVulkanDescriptorSet.h>
#include <gfx-vk/GFXVulkanGraphicsPipeline.h>
#include <chrono>
#include <gfx-vk/BufferHelper.h>
#include <gfx-vk/GFXVulkanShaderModule.h>
#include "VertexData.h"
#include "IOHelper.hpp"
#include <gfx/GFXDescriptorSet.h>

#define VULKAN_REVERT_VIEWPORT 1

const int MAX_FRAMES_IN_FLIGHT = 2;

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif


class HelloTriangleApplication {
public:
    const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0,
    4,5,6,6,7,4
    };
    const std::vector<gfx::VertexData> vertices =
    {
        {{-0.5f, -0.f, -0.5f}, {0.f,1.f,0.f}, {0.f,0.f,0.f}, {1.0f, 0.0f, 0.0f, 1.f}, {{0.f, 1.f}}},
        {{0.5f, 0.f, -0.5f}, {0.f,1.f,0.f}, {0.f,0.f,0.f}, {0.0f, 1.0f, 0.0f, 1.f}, { {1.f, 1.f} }},
        {{0.5f, 0.f, 0.5f}, {0.f,1.f,0.f}, {0.f,0.f,0.f}, {0.0f, 0.0f, 1.0f, 1.f}, { {1.f, 0.f} }},
        {{-0.5f, 0.f, 0.5f}, {0.f,1.f,0.f}, {0.f,0.f,0.f}, {1.0f, 1.0f, 1.0f, 1.f}, { {0.f, 0.f} }},

        {{-0.5f, -0.5f, -0.5f}, {0.f,1.f,0.f}, {0.f,0.f,0.f}, {1.0f, 0.0f, 0.0f, 1.f}, {{0.f, 1.f}}},
        {{0.5f, -0.5f, -0.5f}, {0.f,1.f,0.f}, {0.f,0.f,0.f}, {0.0f, 1.0f, 0.0f, 1.f}, { {1.f, 1.f} }},
        {{0.5f, -0.5f, 0.5f}, {0.f,1.f,0.f}, {0.f,0.f,0.f}, {0.0f, 0.0f, 1.0f, 1.f}, { {1.f, 0.f} }},
        {{-0.5f, -0.5f, 0.5f}, {0.f,1.f,0.f}, {0.f,0.f,0.f}, {1.0f, 1.0f, 1.0f, 1.f}, { {0.f, 0.f} }},
    };

    struct UniformBufferObject
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

    void run()
    {
        gfx::GFXGlobalConfig config;
        config.WindowHeight = 720;
        config.WindowWidth = 1280;
        strcpy(config.Title, "Puslar Editor v0.1.0 - Vulkan 1.1");
        strcpy(config.ProgramName, "Puslar");
        
        gfxapp = new gfx::GFXVulkanApplication(config);
        gfxapp->Initialize();

        descriptorSetLayout = std::shared_ptr<gfx::GFXVulkanDescriptorSetLayout>(new gfx::GFXVulkanDescriptorSetLayout(
            gfxapp,
            {
                {uint32_t(0), gfx::GFXDescriptorType::ConstantBuffer, gfx::GFXShaderStage::Vertex},
                {uint32_t(1), gfx::GFXDescriptorType::CombinedImageSampler, gfx::GFXShaderStage::Fragment}
            }));

        {
            auto vertShaderCode = IOHelper::ReadFile("shader/lit.vert.spv");
            auto fragShaderCode = IOHelper::ReadFile("shader/lit.pixel.spv");

            auto shaderModule = gfxapp->CreateShaderModule(vertShaderCode, fragShaderCode);
            gfx::GFXGraphicsPipelineConfig cfg;
            {
                cfg.CullMode = gfx::GFXCullMode::Back;
                cfg.DepthTestEnable = true;
                cfg.DepthWriteEnable = true;
                cfg.DepthCompareOp = gfx::GFXCompareMode::Less;
            }

            pipeline = std::static_pointer_cast<gfx::GFXVulkanGraphicsPipeline> 
                (gfxapp->CreateGraphicsPipeline(cfg, gfx::GetBindingDescription(gfxapp), shaderModule, descriptorSetLayout));
            

        }


        {
            auto texbuf = IOHelper::ReadFile("textures/texture.png");
            textureImage = std::static_pointer_cast<gfx::GFXVulkanTexture2D>(gfxapp->CreateTexture2DFromMemory((uint8_t*)texbuf.data(), texbuf.size()));
        }


        vertexBuffer = gfxapp->CreateBuffer(gfx::GFXBufferUsage::Vertex, sizeof(vertices[0]) * vertices.size());
        vertexBuffer->Fill(vertices.data());

        indexBuffer = (gfx::GFXVulkanBuffer*)gfxapp->CreateBuffer(gfx::GFXBufferUsage::Index, sizeof(indices[0]) * indices.size());
        indexBuffer->Fill(indices.data());

        uniformBuffers = (gfx::GFXVulkanBuffer*)gfxapp->CreateBuffer(gfx::GFXBufferUsage::ConstantBuffer, sizeof(UniformBufferObject));
        //uniformBuffers.push_back((gfx::GFXVulkanBuffer*)gfxapp->CreateBuffer(gfx::GFXBufferUsage::ConstantBuffer, sizeof(UniformBufferObject)));

        //create descriptor set
        {
            auto set0 = gfxapp->GetDescriptorManager()->GetDescriptorSet(descriptorSetLayout.get());
            set0->AddDescriptor(0)->SetConstantBuffer(sizeof(UniformBufferObject), uniformBuffers);
            set0->AddDescriptor(1)->SetTextureSampler2D(textureImage.get());

            auto set1 = gfxapp->GetDescriptorManager()->GetDescriptorSet(descriptorSetLayout.get());
            set1->AddDescriptor(0)->SetConstantBuffer(sizeof(UniformBufferObject), uniformBuffers);
            set1->AddDescriptor(1)->SetTextureSampler2D(textureImage.get());

            set0->Submit();
            set1->Submit();
            descriptorSets.push_back(std::static_pointer_cast<gfx::GFXVulkanDescriptorSet>(set0));
            //descriptorSets.push_back(std::static_pointer_cast<gfx::GFXVulkanDescriptorSet>(set1));
        }



        createSyncObjects();

        gfxapp->OnLoop = [this](float dt)
        {
            drawFrame(dt);
        };
        gfxapp->ExecLoop();

        cleanup();
        gfxapp->Terminate();
    }

    std::shared_ptr<gfx::GFXVulkanTexture2D> textureImage;

    VkCommandBuffer GetVkCommandBuffer(const gfx::GFXCommandBufferScope& scope)
    {
        return static_cast<gfx::GFXVulkanCommandBuffer*>(scope.operator->())->GetVkCommandBuffer();
    }



private:

    gfx::GFXVulkanApplication* gfxapp;
    gfx::GFXBuffer* vertexBuffer;
    gfx::GFXVulkanBuffer* indexBuffer;

    std::shared_ptr<gfx::GFXVulkanDescriptorSetLayout> descriptorSetLayout;
    std::vector<std::shared_ptr<gfx::GFXVulkanDescriptorSet>> descriptorSets;

    std::shared_ptr<gfx::GFXVulkanGraphicsPipeline> pipeline;

    gfx::GFXVulkanBuffer* uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;


    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    bool framebufferResized = false;
    uint32_t currentFrame = 0;



    void cleanupSwapChain() {

        for (size_t i = 0; i < gfxapp->GetVkSwapchainImageViews().size(); i++) {
            vkDestroyImageView(gfxapp->GetVkDevice(), gfxapp->GetVkSwapchainImageViews()[i], nullptr);
        }

        vkDestroySwapchainKHR(gfxapp->GetVkDevice(), gfxapp->GetVkSwapchain(), nullptr);

    }

    void recreateSwapChain() {

        int width = 0, height = 0;
        glfwGetFramebufferSize(reinterpret_cast<GLFWwindow*>(gfxapp->GetWindowHandle()), &width, &height);
        while (width == 0 || height == 0)
        {
            glfwGetFramebufferSize(reinterpret_cast<GLFWwindow*>(gfxapp->GetWindowHandle()), &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(gfxapp->GetVkDevice());

        cleanupSwapChain();

        gfxapp->InitSwapChain();
        gfxapp->InitDepthTestBuffer();
        gfxapp->InitFrameBuffers();

    }

    void cleanup() {

        vertexBuffer->Release();
        indexBuffer->Release();
        /*for (auto& uniformBuffer : uniformBuffers)
        {
            uniformBuffer->Release();
        }*/
        delete uniformBuffers;

        textureImage.reset();

        descriptorSetLayout.reset();
        descriptorSets.clear();

        pipeline.reset();

        vkDestroyRenderPass(gfxapp->GetVkDevice(), gfxapp->GetVkRenderPass(), nullptr);

        /*vkDestroyCommandPool(gfxapp->GetVkDevice(), gfxapp->GetVkCommandPool(), nullptr);*/
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(gfxapp->GetVkDevice(), renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(gfxapp->GetVkDevice(), imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(gfxapp->GetVkDevice(), inFlightFences[i], nullptr);
        }

    }


    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = gfxapp->GetVkRenderPass();
        renderPassInfo.framebuffer = gfxapp->GetVkFrameBuffers()[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = gfxapp->GetVkSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        {
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetVkPipeline());

            VkViewport viewport{};
#if VULKAN_REVERT_VIEWPORT
            viewport.x = 0.0f;
            viewport.y = 0.0f + gfxapp->GetVkSwapChainExtent().height;
            viewport.width = (float)gfxapp->GetVkSwapChainExtent().width;
            viewport.height = -(float)gfxapp->GetVkSwapChainExtent().height;
#else
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = (float)gfxapp->GetVkSwapChainExtent().width;
            viewport.height = (float)gfxapp->GetVkSwapChainExtent().height;
#endif
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = { 0, 0 };
            scissor.extent = gfxapp->GetVkSwapChainExtent();
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            //vkCmdDraw(commandBuffer, 3, 1, 0, 0);

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetVkPipeline());

            VkBuffer vertexBuffers[] = { static_cast<gfx::GFXVulkanBuffer*>(vertexBuffer)->GetVkBuffer() };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer->GetVkBuffer(), 0, VK_INDEX_TYPE_UINT16);
            //vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
            auto descriptorSet = descriptorSets[0]->GetVkDescriptorSet();
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetVkPipelineLayout(), 0, 1, &descriptorSet, 0, nullptr);
            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
        }
        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void createSyncObjects() {

        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            if (vkCreateSemaphore(gfxapp->GetVkDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(gfxapp->GetVkDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(gfxapp->GetVkDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }



    }

    void updateUniformBuffer(uint32_t currentImage)
    {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.f));
        //ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.f, 0.f));
        ubo.view = glm::lookAtLH(glm::vec3(0.0f, 2.0f, -2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ubo.proj = glm::perspectiveLH_ZO(glm::radians(45.0f), gfxapp->GetVkSwapChainExtent().width / (float)gfxapp->GetVkSwapChainExtent().height, 0.1f, 10.0f);

        uniformBuffers->Fill(&ubo);
    }

    void drawFrame(float)
    {

        updateUniformBuffer(currentFrame);

        //std::cout << "tick" << std::endl;
        vkWaitForFences(gfxapp->GetVkDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(gfxapp->GetVkDevice(), gfxapp->GetVkSwapchain(), UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        vkResetFences(gfxapp->GetVkDevice(), 1, &inFlightFences[currentFrame]);

        vkResetCommandBuffer(gfxapp->GetVkCommandBuffer(currentFrame), /*VkCommandBufferResetFlagBits*/ 0);
        recordCommandBuffer(gfxapp->GetVkCommandBuffer(currentFrame), imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &gfxapp->GetVkCommandBuffer(currentFrame);

        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(gfxapp->GetVkGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { gfxapp->GetVkSwapchain() };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(gfxapp->GetVkPresentQueue(), &presentInfo);
        vkQueueWaitIdle(gfxapp->GetVkPresentQueue());

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }


};

int main() 
{
    HelloTriangleApplication app;

    try 
    {
        app.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}