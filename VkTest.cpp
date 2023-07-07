
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
#include <gfx/GFXShaderModule.h>
#include <gfx/GFXShaderPass.h>
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
#include <gfx-vk/GFXVulkanShaderPass.h>
#include <gfx-vk/GFXVulkanViewport.h>
#include <gfx-vk/GFXVulkanDescriptorManager.h>
#include <gfx/GFXRenderPipeline.h>
#include <chrono>
#include <gfx-vk/BufferHelper.h>
#include <gfx-vk/GFXVulkanShaderModule.h>
#include "VertexData.h"
#include "IOHelper.hpp"
#include <gfx/GFXDescriptorSet.h>
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"

using namespace gfx;

class HDRenderPipeline : public GFXRenderPipeline
{
public:
    GFXShaderPass* Shaderpass;
    std::vector<GFXBuffer*> VertBuffers;
    GFXBuffer* IndexBuffer;
    GFXVulkanDescriptorSet* DescriptorSet;

    virtual void OnRender(GFXRenderContext* context, const std::vector<GFXFrameBufferObject*>& renderTargets) override
    {
        auto rt = static_cast<GFXVulkanFrameBufferObject*>(renderTargets[0]);

        auto& buffer = static_cast<GFXVulkanCommandBuffer&>(context->AddCommandBuffer());
        buffer.Begin();
        buffer.SetFrameBuffer(rt);
        buffer.CmdClearColor(0.1, 0.16, 0.16, 1);
        buffer.CmdBeginFrameBuffer();
        buffer.CmdSetViewport(0, 0, rt->GetWidth(), rt->GetHeight());
        //being render
        buffer.CmdBindShaderPass(Shaderpass);
        buffer.CmdBindVertexBuffers(VertBuffers);
        buffer.CmdBindIndexBuffer(IndexBuffer);
        buffer.CmdBindDescriptorSets(DescriptorSet, Shaderpass);
        buffer.CmdDrawIndexed(IndexBuffer->GetSize() / sizeof(uint16_t));

        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
        if (!is_minimized)
        {

            ImGui_ImplVulkan_RenderDrawData(draw_data, buffer.GetVkCommandBuffer());
        }

        //end render
        buffer.CmdEndFrameBuffer();
        buffer.SetFrameBuffer(nullptr);
        buffer.End();

        context->Submit();
    }
};

class HelloTriangleApplication {
public:
    const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4
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
            auto vertShaderCode = IOHelper::ReadFile("shader/Lit.vs.spv");
            auto fragShaderCode = IOHelper::ReadFile("shader/Lit.ps.spv");

            auto shaderModule = gfxapp->CreateShaderModule(vertShaderCode, fragShaderCode);
            gfx::GFXShaderPassConfig cfg;
            {
                cfg.CullMode = gfx::GFXCullMode::Back;
                cfg.DepthTestEnable = true;
                cfg.DepthWriteEnable = true;
                cfg.DepthCompareOp = gfx::GFXCompareMode::Less;
            }

            pipeline = std::static_pointer_cast<gfx::GFXVulkanShaderPass>
                (gfxapp->CreateGraphicsPipeline(cfg, gfx::GetBindingDescription(gfxapp), shaderModule, descriptorSetLayout, gfxapp->GetVulkanViewport()->GetRenderPass()));

        }


        {
            auto texbuf = IOHelper::ReadFile("textures/texture.png");
            auto tex = gfxapp->CreateTexture2DFromMemory((uint8_t*)texbuf.data(), texbuf.size(), GFXSamplerConfig{});
            textureImage = std::static_pointer_cast<gfx::GFXVulkanTexture2D>(tex);
        }


        vertexBuffer = gfxapp->CreateBuffer(gfx::GFXBufferUsage::Vertex, sizeof(vertices[0]) * vertices.size());
        vertexBuffer->Fill(vertices.data());

        indexBuffer = (gfx::GFXVulkanBuffer*)gfxapp->CreateBuffer(gfx::GFXBufferUsage::Index, sizeof(indices[0]) * indices.size());
        indexBuffer->Fill(indices.data());

        uniformBuffers = (gfx::GFXVulkanBuffer*)gfxapp->CreateBuffer(gfx::GFXBufferUsage::ConstantBuffer, sizeof(UniformBufferObject));

        //create descriptor set
        {
            auto set0 = gfxapp->GetDescriptorManager()->GetDescriptorSet(descriptorSetLayout.get());
            set0->AddDescriptor(0)->SetConstantBuffer(sizeof(UniformBufferObject), uniformBuffers);
            set0->AddDescriptor(1)->SetTextureSampler2D(textureImage.get());

            //auto set1 = gfxapp->GetDescriptorManager()->GetDescriptorSet(descriptorSetLayout.get());
            //set1->AddDescriptor(0)->SetConstantBuffer(sizeof(UniformBufferObject), uniformBuffers);
            //set1->AddDescriptor(1)->SetTextureSampler2D(textureImage.get());

            set0->Submit();
            //set1->Submit();
            descriptorSets.push_back(std::static_pointer_cast<gfx::GFXVulkanDescriptorSet>(set0));
            //descriptorSets.push_back(std::static_pointer_cast<gfx::GFXVulkanDescriptorSet>(set1));
        }

        auto srp = new HDRenderPipeline;
        gfxapp->SetRenderPipeline(srp);

        srp->VertBuffers = { vertexBuffer };
        srp->IndexBuffer = indexBuffer;
        srp->Shaderpass = pipeline.get();
        srp->DescriptorSet = descriptorSets[0].get();

        
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(gfxapp->GetWindow(), true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = gfxapp->GetVkInstance();
        init_info.PhysicalDevice = gfxapp->GetVkPhysicalDevice();
        init_info.Device = gfxapp->GetVkDevice();
        init_info.QueueFamily = 0;
        init_info.Queue = gfxapp->GetVkGraphicsQueue();
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.MinImageCount = 2;
        init_info.ImageCount = 2;
        init_info.DescriptorPool = gfxapp->GetVulkanDescriptorManager()->GetCommonDescriptorSetPool()->GetVkDescriptorPool();
        init_info.Allocator = VK_NULL_HANDLE;

        ImGui_ImplVulkan_Init(&init_info, gfxapp->GetVulkanViewport()->GetRenderPass()->GetVkRenderPass());
        
        {
            GFXVulkanCommandBuffer font(gfxapp);
            VkCommandBufferBeginInfo beginInfo{};
            {
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            }
            font.Begin();
            ImGui_ImplVulkan_CreateFontsTexture(font.GetVkCommandBuffer());
            font.End();
            VkSubmitInfo submitInfo{};
            {
                submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &font.GetVkCommandBuffer();
            }

            auto err = vkQueueSubmit(gfxapp->GetVkGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
            assert(err == VkResult::VK_SUCCESS);
            err = vkQueueWaitIdle(gfxapp->GetVkGraphicsQueue());
            assert(err == VkResult::VK_SUCCESS);
            ImGui_ImplVulkan_DestroyFontUploadObjects();
        }
        
        gfxapp->OnPreRender = [this](float dt)
        {
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            //ImGui::Text("Hello, world %d", 123);
            static bool show_demo_window = true;
            if (show_demo_window)
                ImGui::ShowDemoWindow(&show_demo_window);

            updateUniformBuffer(dt);

        };
        gfxapp->OnPostRender = [this](float dt)
        {

        };

        gfxapp->ExecLoop();

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        delete gfxapp->GetRenderPipeline();
        gfxapp->SetRenderPipeline(nullptr);

        cleanup();
        gfxapp->Terminate();
    }

    std::shared_ptr<gfx::GFXVulkanTexture2D> textureImage;

    VkCommandBuffer GetVkCommandBuffer(const GFXVulkanCommandBufferScope& scope)
    {
        return static_cast<gfx::GFXVulkanCommandBuffer*>(scope.operator->())->GetVkCommandBuffer();
    }



private:

    gfx::GFXVulkanApplication* gfxapp;
    gfx::GFXBuffer* vertexBuffer;
    gfx::GFXVulkanBuffer* indexBuffer;

    std::shared_ptr<gfx::GFXVulkanDescriptorSetLayout> descriptorSetLayout;
    std::vector<std::shared_ptr<gfx::GFXVulkanDescriptorSet>> descriptorSets;

    std::shared_ptr<gfx::GFXVulkanShaderPass> pipeline;

    gfx::GFXVulkanBuffer* uniformBuffers;


    void cleanup() {

        vertexBuffer->Release();
        indexBuffer->Release();
        delete uniformBuffers;

        textureImage.reset();
        pipeline.reset();

        descriptorSetLayout.reset();
        descriptorSets.clear();
    }

    void updateUniformBuffer(float dt)
    {
        static float timecount = 0;
        timecount += dt;

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), timecount * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.f));
        //ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.f, 0.f));
        ubo.view = glm::lookAtLH(glm::vec3(0.0f, 2.0f, -2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ubo.proj = glm::perspectiveLH_ZO(glm::radians(45.0f), gfxapp->GetVulkanViewport()->GetVkSwapChainExtent().width / (float)gfxapp->GetVulkanViewport()->GetVkSwapChainExtent().height, 0.1f, 10.0f);

        uniformBuffers->Fill(&ubo);
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