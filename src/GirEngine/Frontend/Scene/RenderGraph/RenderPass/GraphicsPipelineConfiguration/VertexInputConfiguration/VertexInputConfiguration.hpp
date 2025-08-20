//
// Created by paull on 2023-10-26.
//

#pragma once

namespace pEngine::girEngine::scene::graph::renderPass::pipeline {

    /**
     * For now (big emphasis on "For now") I'm just going to structure this around
     * the Vulkan input bindings nomenclature in a very direct way;
     *
     * Coming back this, it'll have to just capture all the information
     * in the new GeometryBinding::VertexDataBinding struct
     */

    struct VertexInputConfiguration {

        struct VertexBindingSlotDescription {
            // this should amalgamate the information that goes into the VkPipelineVertexInputStateCreateInfo
            // (namely the VkVertexInputBindingDescription and VkVertexInputAttributeDescription structs)
            unsigned bindingIndex = 0;

            unsigned vertexStride = 0;

            std::vector<geometry::AttributeDescription> attributes = {};
        };
        std::vector<VertexBindingSlotDescription> bindingSlotDescriptions = {};

    };

} // pipeline
