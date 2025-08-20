//
// Created by paull on 2023-09-18.
//

#pragma once

#include <memory>

namespace pEngine::girEngine::scene {


    /**
     *
     * The idea is that you specify a particular render target image (with resolution and
     * other such relevant information), and then rather than create an Image for it manually
     * you just specify this thing and then upon scene bake it will generate everything for you
     *
     * Coming back to this - to clarify the above I think we'll basically be obtaining
     * this information on the backend side; this maybe can just contain some simple
     * defaults and just basically be a special image resource that can be referenced
     *
     *
     */
    class SwapchainRenderTarget {
        // we don't need all the image info here since it'll be obtained dynamically backend-side
        // (I'll leave swapchain configuration for a later update; for now you get what you get)
    public:
        struct CreationInput {
            // i'm not fully sure of what information the user should be specifying up front about this one

            boost::optional<std::string> name = boost::none;

            // I'll make it optional to provide an extent, else the system assumes you'll want it to match swapchain dims
            boost::optional<Image::ImageExtent2D> extent = boost::none;

            boost::optional<glm::vec3> clearColor = boost::none;
        };

        explicit SwapchainRenderTarget(const CreationInput &input)
                : swapchainImage(
                Image(Image::CreationInput{
                        input.name.get_value_or(""),
                        util::UniqueIdentifier(),
                        ResourceFormat::SWAPCHAIN_DEPENDENT,
                        Image::ImageUsage::SwapchainRenderTarget
                        // all other config is default; this image is just a handle
                })),
                  clearColor(input.clearColor.get_value_or({})) {

        }

        SwapchainRenderTarget() : SwapchainRenderTarget(CreationInput{}) {

        }

        [[nodiscard]] const Image &getSwapchainImage() const {
            return swapchainImage;
        }

        [[nodiscard]] const glm::vec3 &getClearColor() const {
            return clearColor;
        }

    private:
        /**
         * This is the image resource that is logically bound to whatever swapchain image
         * has been granted to the system (which comes from the presentation engine and depends on
         * your chosen presentation mechanism (for now we're just doing FIFO/plain vsync))
         */
        Image swapchainImage = {}; // TODO -> refactor all these dumbass shared pointers out lol

        glm::vec3 clearColor = {};
    };

} // scene
