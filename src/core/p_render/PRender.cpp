#include "../../../include/core/p_render/PRender.hpp"

// #include "../../../include/components/p_render/gui/GUIHandler.hpp"

// #include "../../../include/core/p_render/backend/vulkan/VulkanWrapper.hpp"
// #include "../../../include/core/p_render/backend/vulkan/vulkan_manager/VulkanManager.hpp"

#include "../../../include/core/p_render/backend/Context.hpp"
#include "../../../include/core/p_render/backend/gui/VulkanGUIHandler.hpp"

#include "../../../include/core/PEngineCore.hpp"

#include "../../../include/core/p_render/backend/frame_context/FrameContext.hpp"




// scene
// #include "../../../include/components/p_render/scene/Scene.hpp"

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

    // set up imgui
    setupImGui();

    // TODO: start implementing the Scene abstraction
    // scene_ = std::make_unique<scene::Scene>();

    // hmm.. maybe i should just have the render graph create to empty, and then have the gui management happen outside of the render graph entirely
    // renderGraphs_["default"] = std::make_shared<RenderGraph>(context_, allocator_); // MAR3 - shared_ptr instead of unique
    //     // gonna try doing this, then we can set up the initial render graph at the end of this constructor

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
    guiCreateInfo.engineCore = core_; // this is weird but i'll try it...

    // create GUI handler
    gui_ = std::make_shared<VulkanGUIHandler>(guiCreateInfo);

    // initiate any other state/components
    // BACKEND REWRITE: initiate frame contexts; for now just make it use the number of swapchain images
    activeFrameContext_ = 0; // this will probably get reset by "vkAcquireNextSwapchainImage" but i'll initialize it i suppose
    for (unsigned int i = 0; i < context_->getSwapchainImageCount(); i++) {
        // i think the frame context will probably need some ctor info;
            // should probably specify an index, a swapchain image(?), etc
        frameContexts_.push_back(std::make_shared<Backend::FrameContext>(context_, i, core_->getNumThreads()));
    }
}


PRender::~PRender() {

}

void PRender::setupImGui() {
    // IMGUI_CHECKVERSION();

    // imguiData_ = std::make_shared<ImGuiData>();

    // imguiData_.ctx_ = ImGui::CreateContext();
    // ImGui::SetCurrentContext(imguiData_.ctx_);

    // // get io, enable keyboard+mouse?
    // imguiData_.io_ = &ImGui::GetIO();
    // imguiData_.io_->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // // io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos; // does this do it? dunno if you need it 

    // // first set up the single color attachment description (there's only one we need i think)
    // VkAttachmentDescription attachmentDescription = {};
    // attachmentDescription.flags = 0;
    // attachmentDescription.format = context_->getSwapchainImageFormat();
    // attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    // attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    // attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    // attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    // attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    // attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // // create a color attachment reference for the color attachment
    // VkAttachmentReference attachmentReference = {};
    // attachmentReference.attachment = 0;
    // attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // // create a graphics subpass for the color attachment (should only need one subpass for DearIMGUI)
    // VkSubpassDescription subpass = {};
    // subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    // subpass.colorAttachmentCount = 1;
    // subpass.pColorAttachments = &attachmentReference;

    // // set up a subpass dependency for synchronization
    // VkSubpassDependency subpassDependency;
    // subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    // subpassDependency.dstSubpass = 0; // we only use one subpass 
    // subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    // subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    // subpassDependency.srcAccessMask = 0; // apparently could also set this to VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT 
    // subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // // finally create the gui render pass
    // VkRenderPassCreateInfo renderPassCreateInfo = {};
    // renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    // renderPassCreateInfo.attachmentCount = 1;
    // renderPassCreateInfo.pAttachments = &attachmentDescription;
    // renderPassCreateInfo.subpassCount = 1;
    // renderPassCreateInfo.pSubpasses = &subpass;
    // renderPassCreateInfo.dependencyCount = 1;
    // renderPassCreateInfo.pDependencies = &subpassDependency;

    // if (vkCreateRenderPass(context_->getLogicalDevice(), &renderPassCreateInfo, nullptr, &imguiData_.guiPass) != VK_SUCCESS) {
    //     throw std::runtime_error("Unable to create IMGUI Render Pass!");
    // }

    // // set up a descriptor pool (just using some baloney sizes for now, hopefully it won't cause any problems)
    //     // hopefully these work, some are probably not needed but whatever
    // std::vector<VkDescriptorPoolSize> descriptorPoolSizes = {
    //     { VK_DESCRIPTOR_TYPE_SAMPLER, 100 },
    //     { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 },
    //     { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100 },
    //     { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 },
    //     { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100 },
    //     { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100 },
    //     { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
    //     { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 },
    //     { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 },
    //     { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100 },
    //     { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100 }
    // };

    // VkDescriptorPoolCreateInfo descriptorPoolCreateInfo;
    // descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    // descriptorPoolCreateInfo.pNext = nullptr;
    // descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    // descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
    // descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
    // descriptorPoolCreateInfo.maxSets = static_cast<uint32_t>(context_->getMinImageCount()); // use the number of swapchain images???

    // auto result = vkCreateDescriptorPool(context_->getLogicalDevice(), &descriptorPoolCreateInfo, nullptr, &imguiData_.guiDescriptorPool);
    // if (result != VK_SUCCESS) {
    //     throw std::runtime_error("Unable to create GUI descriptor pool!");
    // }

    // ImGui_ImplVulkan_InitInfo imguiInitInfo;
    // imguiInitInfo.Instance = context_->getInstance(); 
    // imguiInitInfo.Device = context_->getLogicalDevice();
    // imguiInitInfo.PhysicalDevice = context_->getSelectedPhysicalDevice();
    // imguiInitInfo.ImageCount = context_->getMinImageCount();
    // imguiInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    // imguiInitInfo.Queue = context_->getGraphicsQueue(); // graphics queue is always present queue
    // imguiInitInfo.QueueFamily = context_->getGraphicsQueueFamilyIndex();
    // imguiInitInfo.DescriptorPool = imguiData_.guiDescriptorPool;
    // // imguiInitInfo.Subpass = VK_NULL_HANDLE; // TODO: have this pass an actual VkSubpass (maybe GUIPass::getIMGUISubpass()?)
    // // imguiInitInfo.Subpass = guiPass->getIMGUISubpass();
    // imguiInitInfo.Subpass = 0; // i guess just set it to the first subpass since the render pass only has one???

    // // null
    // imguiInitInfo.PipelineCache = VK_NULL_HANDLE;
    // imguiInitInfo.MinImageCount = 2; // yep
    // imguiInitInfo.Allocator = nullptr;
    // imguiInitInfo.CheckVkResultFn = nullptr;

    // // init imgui for vulkan!
    // ImGui_ImplVulkan_Init(&imguiInitInfo, imguiData_.guiPass);

    // // for now just supports 1 window 
    // // createMainWindow();
    // ImGui_ImplWin32_Init(core_->getMainWindowHWND());

    // // i think it's necessary to do some more setup before you can have imgui render things for you
    //     // i'll try separating it out in a separate function for now
    // setupIMGUIResources();
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

    // create framebuffers for imgui rendering

    
}

// void PRender::start() {
//     // assuming a created vulkan instance, get ready to render to the window i guess?
//         // this will be blank in the "create-vulkan-instance" branch i'm on rn
// }

// void PRender::registerRenderGraph(const std::string &renderGraphName, std::shared_ptr<RenderGraph> renderGraph) {
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
    // assert(backendType_ == GraphicsBackendType::vulkan);
    // return dynamic_cast<Backend::Context&>(*context_);
    return std::move(context_);
}

void PRender::registerGUIComponent(std::function<void()> call) {
    std::dynamic_pointer_cast<VulkanGUIHandler>(gui_)->registerGUIComponent(call);
}

void PRender::clearGUIComponents() {
    std::dynamic_pointer_cast<VulkanGUIHandler>(gui_)->clearGUIComponents();
}

void PRender::submitCommandBuffers(Backend::FrameContext &frameContext) {
    // submit command buffers, for now it should wait using a fence...

    // for now i'm gonna just gonna include this function which doesn't make use of wait semaphores but
    // i can probably write another version that does

    // std::unique_lock<std::mutex> ul(commandBuffersLock_);
    // maybe acquire the lock to the frame context
    // std::unique_lock<std::mutex> ul(frameContext.getSubmissionMutex());

    // VkSubmitInfo info = {};
    // VkShaderStageFlags waitFlags = VK_SHADER_STAGE_ALL_GRAPHICS; // i think this should work, this function wouldn't work for compute commands tho i don't think
    // info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    // info.pNext = 0;
    // info.pWaitDstStageMask = &waitFlags;
    // info.waitSemaphoreCount = 1; // APR8 - wait on the imageacquiredsemaphore?
    // info.pWaitSemaphores = &context_->getSwapchainPresentSemaphore();
    // info.signalSemaphoreCount = 1;
    // info.pSignalSemaphores = &context_->getSwapchainRenderCompleteSemaphore(); // signal renderCompleteSemaphore

    // // set up all enqueued command buffers for submission (gotta make sure all of them are in before this is called)
    // info.commandBufferCount = static_cast<uint32_t>(frameContext.getRecordedCommandBuffers().size());
    // info.pCommandBuffers = frameContext.getRecordedCommandBuffers().data();

    // auto result = vkQueueSubmit(context_->getGraphicsQueue(), 1, &info, frameFence_); // for now graphics queue is always the present queue
    // if (result != VK_SUCCESS) {
    //     throw std::runtime_error("Unable to submit command buffers to graphics queue!");
    // }

    // // i guess just have this thread wait on the fence...
    // // result = vkWaitForFences(instance_.device, 1, &frameFence_, VK_TRUE, 50000000); // just gonna try this...

    // while (vkWaitForFences(instance_.device, 1, &frameFence_, VK_TRUE, 50000000) == VK_TIMEOUT) {
    //     // ugly - just kinda some ez bs code to wait, although it doesn't handle any VK_FAILs
    // }

    // frameCommandBuffers_.clear(); // clear the command buffers, which should have all been executed by this point!

    frameContext.submitCommandBuffers(); // maybe just do this...
}