//
// Created by paull on 2023-01-07.
//

#pragma once

#include <vulkan/vulkan_core.h>

namespace PGraphics {

    class VulkanCameraView {
    public:
        struct CreationInput {
            VkViewport viewport;
        };

        explicit VulkanCameraView(const CreationInput &input) : viewport(input.viewport) {

        }

        ~VulkanCameraView() = default;

        [[nodiscard]] const VkViewport &getViewport() const {
            return viewport;
        }

    private:
        /**
         * Before going back to the scene-level abstraction I'll figure out what's needed for the actual Vulkan shite
         *
         * Viewport transformation stuff is the final coordinate transform in the Vulkan pipeline
         * before rasterization.
         *
         * Main idea: it transforms vertices from NORMALIZED DEVICE COORDS to WINDOW COORDS
         *
         * We set these up when creating our graphics pipeline(s), so that's where it'll be in the baking process.
         *
         * As far as I know, we just have to maintain a single struct for this
         *
         * The viewport itself is mainly about the actual graphical window being rendered to by the Vulkan application,
         * I think. When we want to change the camera position we would do that by changing the
         * VIEW matrix in the Model-View-Projection matrices that are used in drawing vertices
         *
         * That should hopefully simplify this bad boi though, we don't really even update it - it's basically
         * just a bake-time thing
         */
        VkViewport viewport;


    };

} // PGraphics
