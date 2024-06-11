//
// Created by paull on 2022-06-26.
//

#pragma once

#include <vector>

namespace pEngine::girEngine::backend::vulkan {

    // TODO - reconsider the vulkan render pass design... I think I'll definitely have to flesh this out
    class VulkanRenderPass {
    public:
        ~VulkanRenderPass() = default;

        [[maybe_unused]] [[nodiscard]] virtual bool isDynamicRenderPass() const {
            return false;
        }

        [[maybe_unused]] [[nodiscard]] virtual bool isDearImguiRenderPass() const {
            return false;
        }

        // ONE THING -> now that I'm trying to get the very first VulkanRenderPass thing
        // to actually record & submit commands (the DearImgui one, whose logic was ripped from the examples pretty much
        // (it might be worth going through and seeing if they've updated it; this was brought over for an older vulkan version)

        /**
         * I was originally thinking I was going to have a virtual function here for drawing frames;
         * I think it might be easier (at least for now; refactoring in the future is a possibility)
         * that we just leave this as-is, and have the vulkan render pass subclasses themselves provide any relevant
         * interface methods.
         *
         * I just need to make sure that we have all info available in the
         */

    protected:


    };

}// namespace PGraphics
