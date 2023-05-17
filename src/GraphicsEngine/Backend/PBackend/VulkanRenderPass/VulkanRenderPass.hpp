//
// Created by paull on 2022-06-26.
//

#pragma once

#include <vector>

namespace PGraphics {

    class PFrame;

    class VulkanRenderPass {
    public:
        ~VulkanRenderPass() = default;

        virtual void recordRenderPassDrawCommandsIntoFrame(const PFrame &frame) = 0;

        [[maybe_unused]] [[nodiscard]] virtual bool isDynamicRenderPass() const {
            return false;
        }

        [[maybe_unused]] [[nodiscard]] virtual bool isDearImguiRenderPass() const {
            return false;
        }

    };

}// namespace PGraphics
