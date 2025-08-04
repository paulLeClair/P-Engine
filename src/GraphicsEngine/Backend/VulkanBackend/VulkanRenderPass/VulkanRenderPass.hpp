//
// Created by paull on 2022-06-26.
//

#pragma once

#include <vector>

namespace pEngine::girEngine::backend::vulkan {
    struct VulkanRenderPass {
        virtual ~VulkanRenderPass() = default;

        enum class RenderResult {
            SUCCESS,
            FAILURE
        };

        // ACTUALLY wait i don't think returning submits will work (they contain pointers to structs that go out of scope)
        // new alternative: probably just pull the submit stuff completely out of this function, put it into the
        // renderer itself, since it needs to have the submit info anyway
        virtual RenderResult
        recordRenderingCommands(VkCommandBuffer &commandBuffer,
                                uint32_t frameInFlightIndex,
                                std::vector<VkCommandBuffer> &copyCommandBuffers,
                                VkCommandPool &commandPool
        ) {
            // TODO -> determine if we need any arguments here
            return RenderResult::FAILURE;
        }

        /**
         * This should contain the relevant info for one particular attribute in one particular binding
         */
        struct VertexBindingAttribute {
            uint32_t attributeLocation = 0;

            VkFormat attributeFormat = VK_FORMAT_UNDEFINED;

            uint32_t attributeOffset = 0;
        };

        /**
         * This represents one particular vertex input binding, which includes some set of attributes
         */
        struct VertexInputBinding {
            uint32_t bindingIndex = 0; // I don't think we need more than 255 bindings per pass (can change later)
            std::vector<VertexBindingAttribute> attributes = {};
        };

        /**
         * I'll just try to give the current subclasses a public vector of bindings that they'll use for
         * drawing.
         */
        std::vector<VertexInputBinding> vertexInputBindings = {};
        std::vector<VkCommandBuffer> allocatedCommandBuffers;

        explicit VulkanRenderPass(const std::vector<VertexInputBinding> &vertexInputBindings)
            : vertexInputBindings(vertexInputBindings) {
        }

        VulkanRenderPass() = default;

        // TODO -> consider adding in descriptor set stuff here if possible; not sure what form that would have to take really
    };
} // namespace PGraphics
