#include "../../../include/core/p_render/PRender.hpp"

#include "../../../include/core/p_render/backend/Context.hpp"
#include "../../../include/core/p_render/backend/gui/VulkanGUIHandler.hpp"

#include "../../../include/core/PEngineCore.hpp"

#include "../../../include/core/p_render/backend/frame_context/FrameContext.hpp"

PRender::PRender(PEngine *engineCore) : core_(engineCore) {

    context_ = std::make_shared<Backend::Context>(core_);

    // NEW: create vma allocator
    VmaAllocatorCreateInfo allocInfo = {};
    allocInfo.device = context_->getLogicalDevice();
    allocInfo.instance = context_->getInstance();
    allocInfo.physicalDevice = context_->getSelectedPhysicalDevice();
    allocInfo.vulkanApiVersion = VK_API_VERSION_1_1;

    if (vmaCreateAllocator(&allocInfo, &allocator_) != VK_SUCCESS) {
        throw std::runtime_error("Unable to create allocator!");
    }

    // TODO: create scene object 
    // scene_ = std::make_unique<scene::Scene>();

    // set up imgui
    setupImGui();

    VulkanIMGUICreateInfo guiCreateInfo = {};
    guiCreateInfo.vulkanInstance = renderContext()->getInstance();
    guiCreateInfo.device = renderContext()->getLogicalDevice();
    guiCreateInfo.selectedPhysicalDevice = renderContext()->getSelectedPhysicalDevice();
    guiCreateInfo.queueFamilyIndex = renderContext()->getGraphicsQueueFamilyIndex();
    guiCreateInfo.queue = renderContext()->getGraphicsQueue();
    // REWRITE: gonna try 
    guiCreateInfo.swapchainImageFormat = renderContext()->getSwapchainImageFormat();
    guiCreateInfo.minImageCount = renderContext()->getMinImageCount();
    guiCreateInfo.imageCount = renderContext()->getSwapchainImageCount();

    guiCreateInfo.context = context_;
    guiCreateInfo.engineCore = core_; 

    // create GUI handler
    gui_ = std::make_shared<VulkanGUIHandler>(guiCreateInfo);

    // initiate any other state/components
    // initialize frame contexts; for now just make it use the number of swapchain images
    activeFrameContext_ = 0; // this will probably get reset by "vkAcquireNextSwapchainImage" but i'll initialize it i suppose
    for (unsigned int i = 0; i < context_->getSwapchainImageCount(); i++) {
        frameContexts_.push_back(std::make_shared<Backend::FrameContext>(context_, i, core_->getNumThreads()));
    }
}


PRender::~PRender() {

}

void PRender::setupImGui() {
    // this is deprecated i think
}

void PRender::setupIMGUIResources() {
    // this will now set up textures/etc for IMGUI, after this function runs it should be possible to use imgui 
        // for now this will be the exclusive GUI mechanism for the engine (unless you want to set up your own Passes in the render graph
        // which i imagine i'll do for some things later)

    // set up fonts texture on gpu, submit the imgui function (which takes a command buffer arg) 
    context_->immediateSubmitCommand([&](VkCommandBuffer cmd) {
        if (!ImGui_ImplVulkan_CreateFontsTexture(cmd)) {
            throw std::runtime_error("Unable to create fonts texture!");
        }
    });
    // call another imgui function to cleanup some of the leftover font texture CPU data
    ImGui_ImplVulkan_DestroyFontUploadObjects();

    
}

std::shared_ptr<RenderGraph> PRender::registerRenderGraph(const std::string &renderGraphName) {
    // i think this is where we should actually make the render graph...
    auto renderGraph = std::make_shared<RenderGraph>(renderGraphName, core_->getThreadPool(), context_, allocator_);
    
    // store graph and map name to index
    renderGraphs_.push_back(renderGraph);
    renderGraphNames_[renderGraphName] = renderGraphs_.size() - 1;

    return renderGraph;
}

void PRender::renderFrame(const std::string &renderGraphName) {
    // new process for rendering a frame:
        // 1. begin next frame context (maybe just use a simple index to track?)
        // 2. execute render graph (or graphs eventually)
        // 3. end frame context to submit buffers and present the corresponding image 

    // get next swapchain image index through wsi?
    unsigned int fcIndex = context_->WSI().acquireNextSwapchainImage();
    auto &frameContext = *frameContexts_[fcIndex];

    if (!frameContext.begin()) {
        throw std::runtime_error("Unable to begin() frame context " + std::to_string(fcIndex));
    }

    renderGraphs_[renderGraphNames_[renderGraphName]]->execute(frameContext);

    // maybe ill try and just have 2 submits... might be ugly/inefficient but idk 
    // i still have to wrap my brain around a lot of the details of vulkan's synchronization
    gui_->renderFrame(frameContext);

    // i think we can submit the render graph buffers via the frame context
    if (!frameContext.end()) {
        throw std::runtime_error("Unable to end() frame context " + std::to_string(fcIndex));
    }

    // end frame by presenting swapchain image
    context_->WSI().presentImage();
}


std::shared_ptr<Backend::Context> &PRender::renderContext()  {
    return context_;
}

void PRender::registerGUIComponent(std::function<void()> call) {
    std::dynamic_pointer_cast<VulkanGUIHandler>(gui_)->registerGUIComponent(call);
}

void PRender::clearGUIComponents() {
    std::dynamic_pointer_cast<VulkanGUIHandler>(gui_)->clearGUIComponents();
}

void PRender::submitCommandBuffers(Backend::FrameContext &frameContext) {
    frameContext.submitCommandBuffers(); 
}