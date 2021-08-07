#include "../../../../../include\core\p_render\backend\shaders\Program.hpp"

#include "../../../../../include/core/p_render/render_graph/Shader.hpp"
#include "../../../../../include/core/p_render/render_graph/pass/Subpass.hpp"

using namespace Backend;

Program::Program(std::shared_ptr<Context> context, std::shared_ptr<Subpass> subpass) {
    // hopefully i can use just the subpass pointer to make this happen!
    context_ = context;
    subpass_ = subpass;

    VkPipelineLayoutCreateInfo plInfo = {};
    plInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    plInfo.pNext = nullptr;
    
    // need to reflect push constant information as well... 

    // go through the different shaders and set up the pipeline shader stage info
    // and get descriptor set layouts 

    std::vector<VkDescriptorSetLayoutCreateInfo> dslInfos = {};
    std::vector<VkDescriptorSetLayout> dsLayouts = {};
    std::vector<VkPushConstantRange> pushConstantRanges = {};

    const auto &processShaderModule = [&](std::shared_ptr<ShaderModule> &sm) {
        VkPipelineShaderStageCreateInfo stageInfo = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        stageInfo.pNext = nullptr;
        stageInfo.flags = 0; // not using any of the flag-enabled functionality
        stageInfo.module = sm->getVkShaderModule();
        // stageInfo.pName = sm->getShaderName().c_str(); // for shader modules that contain >1 shader, this can be specified here (implement it later)
        stageInfo.pName = "main"; // hardcoding this for now; 1 shader per module only (might be inefficient?)

        VkSpecializationInfo specInfo = {};
        // not gonna do anything with spec yet....
        // don't really know what it's for or how it works
        stageInfo.pSpecializationInfo = &specInfo;

        switch (sm->getShaderStage()) {
            case (ShaderModule::ShaderStage::Vertex): {
                // create vertex shader stage
                stageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
                vertexStage_ = std::make_unique<VkPipelineShaderStageCreateInfo>(stageInfo);
            }
            break;
            case (ShaderModule::ShaderStage::TessellationEval): {
                // create tess eval stage
                stageInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
                tessellationEvalStage_ = std::make_unique<VkPipelineShaderStageCreateInfo>(stageInfo);
            }
            break;
            case (ShaderModule::ShaderStage::TessellationControl): {
                stageInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
                tessellationControlStage_ = std::make_unique<VkPipelineShaderStageCreateInfo>(stageInfo);
            }
            break;
            case (ShaderModule::ShaderStage::Geometry): {
                stageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
                geometryStage_ = std::make_unique<VkPipelineShaderStageCreateInfo>(stageInfo);
            }
            break;
            case (ShaderModule::ShaderStage::Fragment): {
                stageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                fragmentStage_ = std::make_unique<VkPipelineShaderStageCreateInfo>(stageInfo);
            }
            break;
            default:
                throw std::runtime_error("Undefined shader stage!"); // this shouldn't happen ideally
        }

        // get descriptor set layout from this shader 
            // need to make sure the bindings are all lined up here
        for (auto &info : sm->getDescriptorSetLayoutInfos()) {
            VkDescriptorSetLayoutCreateInfo ci = {};
            ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            ci.pNext = nullptr;
            ci.flags = 0;
            ci.bindingCount = static_cast<uint32_t>(info.bindings.size());
            ci.pBindings = info.bindings.data();
            dslInfos.push_back(ci);
        }

        // actually create the descriptor set layouts for the pipeline layout create info
        dsLayouts.resize(dslInfos.size());
        for (auto i = 0u; i < dsLayouts.size(); i++) {
            auto result = vkCreateDescriptorSetLayout(context_->getLogicalDevice(), &dslInfos[i], nullptr, &dsLayouts[i]);
            if (result != VK_SUCCESS) {
                throw std::runtime_error("Unable to create descriptor set layout!");
            }
        }
        
        for (auto &pc : sm->getPushConstantRanges()) {
            pushConstantRanges.push_back(pc);
        }

        // whatever else is required to process an individual shader module!

    };

    /* BUILD SHADER STAGES */
    if (subpass->getVertexShader() != nullptr) {
        // get the shader module, which should contain a lot of pre-computed reflection info
        auto shaderModule = subpass_->getVertexShader()->getModule();
        processShaderModule(shaderModule);
    }
    else {
        // this is an error, every graphics pipeline needs at least a vertex shader
        throw std::runtime_error("No vertex shader set for this graphics subpass!");
    }

    if (subpass->getTessellationEvalShader() != nullptr) {
        auto shaderModule = subpass_->getTessellationEvalShader()->getModule();
        processShaderModule(shaderModule);
    }

    if (subpass->getTessellationControlShader() != nullptr) {
        auto shaderModule = subpass_->getTessellationControlShader()->getModule();
        processShaderModule(shaderModule);
    }

    if (subpass->getGeometryShader() != nullptr) {
        auto shaderModule = subpass_->getGeometryShader()->getModule();
        processShaderModule(shaderModule);
    }

    if (subpass->getFragmentShader() != nullptr) {
        auto shaderModule = subpass_->getFragmentShader()->getModule();
        processShaderModule(shaderModule);
   }

    // assemble the pipeline layout create info struct?
    plInfo.setLayoutCount = static_cast<uint32_t>(dsLayouts.size());
    plInfo.pSetLayouts = (dsLayouts.size()) ? dsLayouts.data() : nullptr;
    plInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
    plInfo.pPushConstantRanges = (plInfo.pushConstantRangeCount) ? pushConstantRanges.data() : nullptr;
    
    auto result = vkCreatePipelineLayout(context_->getLogicalDevice(), &plInfo, nullptr, &pipelineLayout_);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Unable to create pipeline layout!");
    }
}

Program::~Program() {
    vkDestroyPipelineLayout(context_->getLogicalDevice(), pipelineLayout_, nullptr);
}