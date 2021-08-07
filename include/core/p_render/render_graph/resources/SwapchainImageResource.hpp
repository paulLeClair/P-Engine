#pragma once

#include "./ImageResource.hpp"
#include "../../backend/resources/SwapchainImage.hpp"


class SwapchainImageResource : public ImageResource {
  public:
    SwapchainImageResource(unsigned int index, const std::string &name, const AttachmentInfo &info, std::shared_ptr<Backend::Context> context) : ImageResource(index, name, info) {
        // this needs to work in a special way:
          // i think i can wrap up all the swapchain images into one special case Image type, which should
          // be very commonly used because usually you're gonna wanna render to your screen

        std::vector<VkImage> swapchainImages = {};
        for (auto i = 0u; i < context->getSwapchainImageCount(); i++) {
          // store these here
          swapchainImages.push_back(context->getSwapchainImage(i));
        }

        image_ = std::make_shared<Backend::SwapchainImage>(swapchainImages);

    }

    ~SwapchainImageResource() {
      
    }

    std::shared_ptr<Backend::SwapchainImage> getBackendImage() {
      return image_;
    }

    VkImage getSpecifiedSwapchainImage(unsigned int index) {
      return image_->getSwapchainImage(index);
    }


  private:  
    // maybe this can mostly just wrap up an associated backend-friendly resource
    // that can just be plugged in directly, and handle it specially during bake also
    std::shared_ptr<Backend::SwapchainImage> image_ = nullptr;
    
};