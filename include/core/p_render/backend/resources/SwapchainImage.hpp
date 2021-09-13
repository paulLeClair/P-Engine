#pragma once

// i'll try making a small header-only specialization of the backend::Image class 

#include "./Image.hpp"

#include <vector>

namespace backend {

// maybe i can keep this one simple since it just basically has to hold on to 
// the swapchain images (which come from the presentation engine and not the user)
class SwapchainImage : public Image {
  public:
    SwapchainImage(std::vector<VkImage> swapchainImages) : swapchainImages_(swapchainImages), Image() {
        // call the protected backend::Image ctor w/ no arguments to ignore the existing image implementation stuff basically
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