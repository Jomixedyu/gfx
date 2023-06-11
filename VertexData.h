#pragma once

namespace gfx
{
    struct VertexData
    {
        static constexpr int MAX_COORD_NUM = 4;

        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec3 Tangent;
        glm::vec4 VertColor;
        glm::vec2 Coords[MAX_COORD_NUM];
    };

    static auto GetBindingDescription(gfx::GFXVulkanApplication* app)
    {
        auto vertDescription = app->CreateVertexLayoutDescription();
        vertDescription->BindingPoint = 0;
        vertDescription->Stride = sizeof(VertexData);

        vertDescription->Attributes.push_back({ gfx::GFXVertexInputDataFormat::R32G32B32_SFloat, offsetof(VertexData, Position) });
        vertDescription->Attributes.push_back({ gfx::GFXVertexInputDataFormat::R32G32B32_SFloat, offsetof(VertexData, Normal) });
        vertDescription->Attributes.push_back({ gfx::GFXVertexInputDataFormat::R32G32B32_SFloat, offsetof(VertexData, Tangent) });
        vertDescription->Attributes.push_back({ gfx::GFXVertexInputDataFormat::R32G32B32A32_SFloat, offsetof(VertexData, VertColor) });
        for (size_t i = 0; i < VertexData::MAX_COORD_NUM; i++)
        {
            vertDescription->Attributes.push_back({ gfx::GFXVertexInputDataFormat::R32G32_SFloat, offsetof(VertexData, Coords[i]) });
        }
        return vertDescription;
    }

}