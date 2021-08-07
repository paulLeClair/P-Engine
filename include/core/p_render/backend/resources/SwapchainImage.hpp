#pragma once

// i'll try making a small header-only specialization of the Backend::Image class 

#include "./Image.hpp"

#include <vector>

namespace Backend {

// maybe i can keep this one simple since it just basically has to hold on to 
// the already-created swapchain images
class SwapchainImage : public Image {
  public:
    SwapchainImage(std::vector<VkImage> swapchainImages) : swapchainImages_(swapchainImages), Image() {
        // call the protected image ctor w/ no arguments to ignore the existing image implementation stuff basically
            // means you shouldn't use the Image interface on these ones (you won't need to i hope)
        activeSwapchainIndex_ = 0u;
    }
    
    void setActiveSwapchainIndex(unsigned int newIndex) {
        activeSwapchainIndex_ = newIndex;
    }

    VkImage getSwapchainImage() {
        return swapchainImages_[activeSwapchainIndex_];
    }

    VkImage getSwapchainImage(unsigned int index) {
        return swapchainImages_[index];
    }    

    bool isSwapchainImage() const override {
        return true;
    }

  private:
    std::vector<VkImage> swapchainImages_ = {};
    unsigned int activeSwapchainIndex_ = 0;

};

}