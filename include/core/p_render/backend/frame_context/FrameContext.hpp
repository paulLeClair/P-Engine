#pragma once

#include "../../PRender.hpp"
#include "../../../thread_pool/Thread.hpp"

#include <memory>

class WindowSystem;

namespace Backend { 

struct ThreadCommandPool {
    int threadIndex;
    VkCommandPool commandPool;

    // for now i'll just have this basically verify that the proper thread is accessing the pool?
    VkCommandPool &pool(Thread &accessingThread) {
        if (accessingThread.getIndex() != threadIndex) {
            throw std::runtime_error("Pool accessed by incorrect thread!");
        }
        return commandPool;
    } 
};

class Context;

class FrameContext {
  public:
    FrameContext(std::shared_ptr<Context> context, unsigned int index, unsigned int numThreads);
    ~FrameContext();

    // begin and end should be all that's needed?
    bool begin();

    void enqueueCommandBuffer(VkCommandBuffer recordedBuffer);
    void enqueueCommandBuffers(std::vector<VkCommandBuffer> &recordedBuffers);

    void submitCommandBuffers();

    bool end();

    std::mutex &getSubmissionMutex() {
        return commandBuffersLock_;
    }

    std::vector<VkCommandBuffer> &getRecordedCommandBuffers() {
        // this shouldn't be called without externally acquiring the commandBuffersLock
        return frameCommandBuffers_;
    }

    VkFence &getFence() {
        return frameFence_;
    }

    unsigned int &getIndex() {
        return index_;
    }

    // VkImage getSwapchainImage() {
    //     return swapchainImage_;
    // }

  private:
    // std::shared_ptr<PEngine> core_;
    // actually we might just need the context
    std::shared_ptr<Context> context_;

    // index of swapchain image
    unsigned int index_;

    // number of threads (for managing per-thread data)
    unsigned int numThreads_;

    // this should contain the relevant state for a single frame context
    WindowSystem &wsi_;

    // swapchain image that this framecontext corresponds to
    // VkImage swapchainImage_;

    // command pools per thread
    std::vector<std::shared_ptr<ThreadCommandPool>> commandPools_;

    // instead of trying to have multiple descriptor pools, i'll try just synchronizing access to one pool
    std::mutex descriptorPoolLock_;
    VkDescriptorPool descriptorPool_;

    // frame command buffers + lock
    std::mutex commandBuffersLock_;
    std::vector<VkCommandBuffer> frameCommandBuffers_;
    VkFence frameFence_;
};

}