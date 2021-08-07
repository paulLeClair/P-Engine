#include "../../../../../include/core/p_render/backend/gui/VulkanGUIData.hpp"

// hmm maybe i can just include the guihandler header to get the create info struct
#include "../../../../../include/core/p_render/backend/gui/VulkanGUIHandler.hpp"

// render graph
// #include "../../../../../include/core/p_render/render_graph/RenderGraph.hpp"

#include "../../../../../include/core/p_render/backend/Context.hpp"

#include "../../../../../include/imgui/imgui.h"
#include "../../../../../include/imgui/imgui_internal.h"
#include "../../../../../include/imgui/imgui_impl_vulkan.h"

//MAY14 - TODO: combine VulkanGUIData and VulkanGUIHandler (needless separation)

VulkanGUIData::VulkanGUIData(const VulkanIMGUICreateInfo &createInfo) {
    // store pointers
    // renderGraph_ = createInfo.renderGraph;
    context_ = createInfo.context;

        // MAR29 - think i have to move this to the windows-specific code to get this to work with the window proc
    // // set up the IMGUI context
    // IMGUI_CHECKVERSION();
    ctx_ = ImGui::GetCurrentContext();
    ImGui::SetCurrentContext(ctx_);
    

    // // get io, enable keyboard+mouse
    // io_ = &ImGui::GetIO();
    // io_->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // // io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos; // does this do it? dunno if you need it 

    /* set up the GUI pass */
        // mar27 - might not need a guipass actually...
    // renderGraph_ = createInfo.renderGraph;
    // renderGraph_->createDefaultGUIPass();
    // guiPass_ = renderGraph_->createDefaultGUIPass("default"); // might be able to just make a general "createGUIPass()" function but 
    // auto guiPass = std::make_shared<GUIPass>("default");
        // mar23 - for now i think i'll make GUIPasses fairly limited compared to your average render pass
            // i'll probably end up doing some 3D gui stuff anyway, which will go in its own pass during 3D rendering
    
    // instead of having this GUIData class correspond to a particular vulkan pass, it'll 
    // maintain the single imgui VkRenderPass?
    
    // first set up the single color attachment description (there's only one we need i think)
    VkAttachmentDescription attachmentDescription = {};
    attachmentDescription.flags = 0;
    attachmentDescription.format = createInfo.swapchainImageFormat;
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // create a color attachment reference for the color attachment
    VkAttachmentReference attachmentReference = {};
    attachmentReference.attachment = 0;
    attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // create a graphics subpass for the color attachment (should only need one subpass for DearIMGUI)
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attachmentReference;

    // set up a subpass dependency for synchronization
    VkSubpassDependency subpassDependency;
    subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependency.dstSubpass = 0; // we only use one subpass 
    subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.srcAccessMask = 0; // apparently could also set this to VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT 
    subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // finally create the gui render pass
    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &attachmentDescription;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &subpassDependency;

    if (vkCreateRenderPass(createInfo.device, &renderPassCreateInfo, nullptr, &guiPass_) != VK_SUCCESS) {
        throw std::runtime_error("Unable to create IMGUI Render Pass!");
    }

    // set up a descriptor pool (just using some baloney sizes for now, hopefully it won't cause any problems)
        // hopefully these work, some are probably not needed but whatever
    std::vector<VkDescriptorPoolSize> descriptorPoolSizes = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 100 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100 }
    };

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo;
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.pNext = nullptr;
    descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
    descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
    descriptorPoolCreateInfo.maxSets = static_cast<uint32_t>(createInfo.imageCount); // use the number of swapchain images???

    auto result = vkCreateDescriptorPool(createInfo.device, &descriptorPoolCreateInfo, nullptr, &guiDescriptorPool_);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Unable to create GUI descriptor pool!");
    }

    ImGui_ImplVulkan_InitInfo imguiInitInfo;
    imguiInitInfo.Instance = createInfo.vulkanInstance; 
    imguiInitInfo.Device = createInfo.device;
    imguiInitInfo.PhysicalDevice = createInfo.selectedPhysicalDevice;
    imguiInitInfo.ImageCount = createInfo.imageCount;
    imguiInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    imguiInitInfo.Queue = createInfo.queue;
    imguiInitInfo.QueueFamily = createInfo.queueFamilyIndex;
    imguiInitInfo.DescriptorPool = guiDescriptorPool_;
    // imguiInitInfo.Subpass = VK_NULL_HANDLE; // TODO: have this pass an actual VkSubpass (maybe GUIPass::getIMGUISubpass()?)
    // imguiInitInfo.Subpass = guiPass->getIMGUISubpass();
    imguiInitInfo.Subpass = 0; // i guess just set it to the first subpass since the render pass only has one???


    // null
    imguiInitInfo.PipelineCache = VK_NULL_HANDLE;
    imguiInitInfo.MinImageCount = 3; // yep
    imguiInitInfo.Allocator = nullptr;
    imguiInitInfo.CheckVkResultFn = nullptr;

    // init imgui for vulkan!
    ImGui_ImplVulkan_Init(&imguiInitInfo, guiPass_);

    

    // i think it's necessary to do some more setup before you can have imgui render things for you
        // i'll try separating it out in a separate function for now
    setupIMGUIResources();
}

VulkanGUIData::~VulkanGUIData() {
    // have to destroy imgui stuff here
    vkDestroyDescriptorPool(context_->getLogicalDevice(), guiDescriptorPool_, nullptr);
    ImGui_ImplVulkan_Shutdown();

    for (auto framebuffer : frameBuffers_) {
        vkDestroyFramebuffer(context_->getLogicalDevice(), framebuffer, nullptr);
    }
}

void VulkanGUIData::setupIMGUIResources() {
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
    std::vector<VkImageView> attachment(1);
    VkFramebufferCreateInfo fbCreateInfo = {};
    fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbCreateInfo.pNext = nullptr;
    fbCreateInfo.renderPass = guiPass_;
    fbCreateInfo.attachmentCount = 1;
    fbCreateInfo.pAttachments = attachment.data(); // set it to the temp var
    fbCreateInfo.width = context_->getSwapchainImageSize().width;
    fbCreateInfo.height = context_->getSwapchainImageSize().height;
    fbCreateInfo.layers = 1;

    // create one framebuffer for each swapchain image view
    auto numImages = context_->getSwapchainImageCount();
    frameBuffers_.resize(numImages);
    for (uint32_t i = 0; i < frameBuffers_.size(); i++) {
        attachment[0] = context_->getSwapchainImageView(i);
        auto result = vkCreateFramebuffer(context_->getLogicalDevice(), &fbCreateInfo, nullptr, &frameBuffers_[i]);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Unable to create imgui framebuffer!");
        }
    }

    // create gui command pool
    VkCommandPoolCreateInfo guiPoolCreateInfo = {};
    guiPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    guiPoolCreateInfo.pNext = nullptr;
    guiPoolCreateInfo.queueFamilyIndex = context_->getGraphicsQueueFamilyIndex();
    guiPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    auto result = vkCreateCommandPool(context_->getLogicalDevice(), &guiPoolCreateInfo, nullptr, &guiCommandPool_);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Unable to create GUI handler command pool!");
    }

    // allocate gui command buffers
    // resize 
    guiCommandBuffers_.resize(context_->getSwapchainImageCount()); // get numswapchain images from the backend


    VkCommandBufferAllocateInfo guiAllocInfo = {};
    guiAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    guiAllocInfo.pNext = nullptr;
    guiAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    guiAllocInfo.commandPool = guiCommandPool_;
    guiAllocInfo.commandBufferCount = static_cast<uint32_t>(guiCommandBuffers_.size());
    
    result = vkAllocateCommandBuffers(context_->getLogicalDevice(), &guiAllocInfo, guiCommandBuffers_.data());
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Unable to create GUI handler command buffers!");
    }
}