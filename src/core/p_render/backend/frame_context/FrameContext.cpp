#include "../../../../../include/core/p_render/backend/frame_context/FrameContext.hpp"

#include "../../../../../include/core/p_render/backend/Context.hpp"

using namespace backend;

FrameContext::FrameContext(std::shared_ptr<Context> context, unsigned int index, unsigned int numThreads) 
    : context_(context), index_(index), numThreads_(numThreads), wsi_(context_->WSI()) {

    for (auto i = 0; i < numThreads; i++) {
        ThreadCommandPool pool;
        pool.threadIndex = i;

        VkCommandPoolCreateInfo info = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        info.flags = 0;
        info.pNext = nullptr;
        info.queueFamilyIndex = context_->getGraphicsQueueFamilyIndex(); // for now we'll just be using hopefully 1 compute+gpu queue, although
        // eventually i'll probably want to decouple them into separate queues (need to look more into that part of the vulkan spec though)

        auto result = vkCreateCommandPool(context_->getLogicalDevice(), &info, nullptr, &pool.commandPool);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Unable to create " + std::to_string(i) + "th command pool!");
        }

        commandPools_.push_back(std::make_shared<ThreadCommandPool>(pool)); // this works for now i guess 
    }

    // create frame fence
    VkFenceCreateInfo info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    info.pNext = nullptr;
    info.flags = 0;
    
    if (vkCreateFence(context_->getLogicalDevice(), &info, nullptr, &frameFence_) != VK_SUCCESS) {
        throw std::runtime_error("Unable to create frame fence!");
    }
}

FrameContext::~FrameContext() {

}

bool FrameContext::begin() {
    // TODO - determine if this is even needed lol
    return true;
}

void FrameContext::enqueueCommandBuffers(std::vector<VkCommandBuffer> &buffers) {
    std::unique_lock<std::mutex> ul(commandBuffersLock_);

    frameCommandBuffers_.insert(frameCommandBuffers_.end(), buffers.begin(), buffers.end());
}

void FrameContext::enqueueCommandBuffer(VkCommandBuffer &buffer) {
    std::unique_lock<std::mutex> ul(commandBuffersLock_);

    frameCommandBuffers_.push_back(buffer);
}

void FrameContext::submitCommandBuffers() {
    // TODO - restructure this

    // std::unique_lock<std::mutex> ul(commandBuffersLock_);

    VkSubmitInfo info = {};
    VkShaderStageFlags waitFlags = VK_SHADER_STAGE_ALL_GRAPHICS; // i think this should work, this function wouldn't work for compute commands tho i don't think
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.pNext = 0;
    info.pWaitDstStageMask = &waitFlags;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &wsi_.getSwapchainPresentSemaphore();
    info.signalSemaphoreCount = 1;
    info.pSignalSemaphores = &wsi_.getSwapchainRenderCompleteSemaphore(); // signal renderCompleteSemaphore

    // set up all enqueued command buffers for submission (gotta make sure all of them are in before this is called)
    info.commandBufferCount = static_cast<uint32_t>(frameCommandBuffers_.size());
    info.pCommandBuffers = frameCommandBuffers_.data();

    auto result = vkQueueSubmit(context_->getGraphicsQueue(), 1, &info, frameFence_);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Unable to submit command buffers to graphics queue!");
    }
    while (vkWaitForFences(context_->getLogicalDevice(), 1, &frameFence_, VK_TRUE, 50000000) == VK_TIMEOUT) {
        // TODO: handle VK_FAILUREs etc
    }

    frameCommandBuffers_.clear(); // clear the command buffers, which should have all been executed by this point!
}

bool FrameContext::end() {
    // should submit and present 
    submitCommandBuffers();

    return true;
}

