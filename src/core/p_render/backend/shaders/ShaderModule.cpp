#include "../../../../../include/core/p_render/backend/shaders/ShaderModule.hpp"

#include "../../../../../include/spirv_reflect/spirv_reflect.h"

using namespace Backend;

ShaderModule::ShaderModule(const std::string &shaderFilename, std::shared_ptr<Context> context, std::shared_ptr<RenderGraph> graph) {
    // this should expect an extension-less name that will be used as a SPV file name
        // for now the project uses CMake to pre-compile any shader source files it finds in 
        // a particular engine directory and the resulting .spv files are what the engine uses
    shaderFilename_ = shaderFilename;
    context_ = std::move(context);
    graph_ = std::move(graph);

    const auto &getShaderFilePathString = [&](const std::string &shaderModuleName) {
        // i think we have to build up the proper path to access the given shader file...
        // the thing is, my Cmake command isn't producing any .spv binaries, which isn't what i want
        
        // TODO: come up with a more consistent method of getting the file path string... this barely works

        // have to excise part of this for now... really ugly, need to come up with a better way of doing this, but for now it should work
        std::string buildDirString = std::filesystem::current_path().string();
        std::string strToExcise = "build\\src";
        std::string fixedSrcDirString = buildDirString.substr(0, buildDirString.length() - strToExcise.length());
        
        // could probably store this somewhere / make it configurable in the future
        std::string shaderBinaryRelativePath = "src\\core\\p_render\\backend\\shaders\\bin\\";

        // build and return path string
        std::string pathString = fixedSrcDirString + shaderBinaryRelativePath + shaderModuleName + ".spv";
        return pathString;
    };

    auto pathString = getShaderFilePathString(shaderFilename);

    // read code from file using std::ifstream
        // following the vkguide's description, we use these flags:
            // ate -> start at the end so we know the size and copy everything in a chunk
            // binary -> we open the stream in BINARY MODE to prevent any possible translations of the SPIR-V code inside
    std::ifstream file(pathString, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open shader module with filename: " + shaderFilename);
    }

    auto filesize = static_cast<uint32_t>(file.tellg()); // use this to get the number of bytes the cursor has passed
        // over to get to the end
    
    // make sure our buffer is the right size for reading the SPIR-V file 
    unsigned int packedFilesize = filesize / sizeof(uint32_t);
    spvCode_.resize(packedFilesize); // we pack SPIR-V into unsigned ints, so make sure that works out

    // reset file cursor
    file.seekg(0);

    // load the entire file into the buffer
    file.read((char*)spvCode_.data(), filesize);

    // close the filestream
    file.close();

    // create the Vulkan shader module
    VkShaderModuleCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.codeSize = filesize;
    info.pCode = spvCode_.data();
    auto smResult = vkCreateShaderModule(context_->getLogicalDevice(), &info, nullptr, &shaderModule_);
    if (smResult != VK_SUCCESS) {
        throw std::runtime_error("Unable to create shader module " + shaderFilename);
    }

    // it probably makes sense to do some reflection at this point too...

    // gonna have to try and figure out how to use this SPIRVReflect library as i go

    // to start, you need to create a SpvReflect shader module:
    SpvReflectShaderModule module = {};
    SpvReflectResult result = spvReflectCreateShaderModule(spvCode_.size() * sizeof(uint32_t), spvCode_.data(), &module);
    if (result != SPV_REFLECT_RESULT_SUCCESS) {
        throw std::runtime_error("Unable to create SPIRV-Reflect shader module!");
    }

    // set shader stage according to reflection
    switch (module.shader_stage) {
        case SPV_REFLECT_SHADER_STAGE_VERTEX_BIT:
            stage_ = ShaderStage::Vertex;
            break;
        case SPV_REFLECT_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
            stage_ = ShaderStage::TessellationControl;
            break;
        case SPV_REFLECT_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
            stage_ = ShaderStage::TessellationEval;
            break;
        case SPV_REFLECT_SHADER_STAGE_GEOMETRY_BIT:
            stage_ = ShaderStage::Geometry;
            break;
        case SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT:
            stage_ = ShaderStage::Fragment;
            break;
        default:
            throw std::runtime_error("Unable to determine shader stage!");
            break; // i think the break is redundant if we throw but w/e
    }

    // now we can enumerate the descriptor sets for this shader
    uint32_t count = 0;
    result = spvReflectEnumerateDescriptorSets(&module, &count, NULL);
    if (result != SPV_REFLECT_RESULT_SUCCESS) {
        throw std::runtime_error("Unable to enumerate shader " + shaderFilename + "'s descriptor sets!");
    }

    // the reflection information is stored in a special struct, so we obtain that info now
    std::vector<SpvReflectDescriptorSet*> sets(count); 
    result = spvReflectEnumerateDescriptorSets(&module, &count, sets.data());
    if (result != SPV_REFLECT_RESULT_SUCCESS) {
        throw std::runtime_error("Unable to enumerate shader " + shaderFilename + "'s descriptor sets!");
    }

    // now we can use this to GET ALL THE DESCRIPTOR SET LAYOUTS FOR THIS SHADER 
    for (auto i = 0u; i < sets.size(); i++) {
        // we reflect descriptor sets one by one:
        const SpvReflectDescriptorSet &reflectionSet = *(sets[i]);
        DescriptorSetLayoutInfo &info = descriptorSetLayoutInfos_[i];

        // set up bindings
        info.bindings.reserve(reflectionSet.binding_count); 
        for (auto j = 0u; j < info.bindings.size(); j++) {
            const auto &reflectedBinding = *reflectionSet.bindings[j];
            auto &infoBinding = info.bindings[j];

            infoBinding.binding = reflectedBinding.binding;
            infoBinding.descriptorType = static_cast<VkDescriptorType>(reflectedBinding.descriptor_type);

            // set descriptor count, respecting arrays of descriptors
            infoBinding.descriptorCount = 1;
            for (uint32_t bindingDim = 0; bindingDim < reflectedBinding.array.dims_count; bindingDim++) {
                infoBinding.descriptorCount *= reflectedBinding.array.dims[bindingDim];
            }

            infoBinding.stageFlags = static_cast<VkShaderStageFlagBits>(module.shader_stage);
        }

        info.setNumber = reflectionSet.set;
        info.layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        info.layoutCreateInfo.pNext = nullptr;
        info.layoutCreateInfo.flags = 0; // none of the functionality enabled by the flags works in this so it's 0
        
        info.layoutCreateInfo.bindingCount = static_cast<uint32_t>(info.bindings.size());
        info.layoutCreateInfo.pBindings = info.bindings.data();
    }
    // at the conclusion of this loop, all the descriptorsetlayoutinfo structs will be filled for this object

    // enumerate the input variables
    uint32_t inputCount = 0;
    result = spvReflectEnumerateInputVariables(&module, &inputCount, NULL);
    if (result != SPV_REFLECT_RESULT_SUCCESS) {
        throw std::runtime_error("Unable to enumerate input variables for shader " + shaderFilename);
    }
    std::vector<SpvReflectInterfaceVariable*> inputs(inputCount);
    result = spvReflectEnumerateInputVariables(&module, &inputCount, inputs.data());
    if (result != SPV_REFLECT_RESULT_SUCCESS) {
        throw std::runtime_error("Unable to enumerate input variables for shader " + shaderFilename);
    }

    // enumerate output variables
    uint32_t outputCount = 0;
    result = spvReflectEnumerateOutputVariables(&module, &outputCount, NULL);
    if (result != SPV_REFLECT_RESULT_SUCCESS) {
        throw std::runtime_error("Unable to enumerate output variables for shader " + shaderFilename);
    }
    std::vector<SpvReflectInterfaceVariable*> outputs(outputCount);
    result = spvReflectEnumerateOutputVariables(&module, &outputCount, outputs.data());
    if (result != SPV_REFLECT_RESULT_SUCCESS) {
        throw std::runtime_error("Unable to enumerate output variables for shader " + shaderFilename);
    }

    // not really sure what i'll have to do for inputs/outputs...
        // maybe i can just change this code to store them and they can be 
        // processed later as needed, eg to make sure that shader i/o variables line up 
    // eventually i'd like this engine to be pretty error-resistant, and have it be fairly easy to 
    // change things on the fly and if they're wrong the engine will allow you to change them or whatever without failing

    // we'll now do everything to populate the VkPipelineVertexInputStateCreateInfo given the 
    // module's expected input variables, as is done in the SPIRVReflect examples
    if (stage_ == ShaderStage::Vertex) {
        // if we're building a vertex shader, we reflect
        
        // might need to flesh this out to make sure our vertex data is good to go
        
        bindingDesc_.binding = 0; // gonna assume a single vertex buffer for the input source, bound at 0
        bindingDesc_.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // TODO: support instancing somehow lol
        
        // the stride will be computed in the loop below
        bindingDesc_.stride = 0;

        // now we'll build and sort the vertex attribute descriptions array
        vertexAttributeDescriptions_.resize(inputs.size());
        for (size_t inputVar = 0; inputVar < inputs.size(); inputVar++) {
            const auto &reflectedVar = *inputs[inputVar];
            auto &desc = vertexAttributeDescriptions_[inputVar];

            desc.location = reflectedVar.location;
            desc.binding = bindingDesc_.binding; // could use this logic to have the binding be changeable
            desc.format = static_cast<VkFormat>(reflectedVar.format);
            desc.offset = 0;
        }
        std::sort(std::begin(vertexAttributeDescriptions_), std::end(vertexAttributeDescriptions_), 
            [&](const VkVertexInputAttributeDescription &first, const VkVertexInputAttributeDescription &second) {
                // we want to sort in ascending order, so we set this condition
                return first.location < second.location;
            }
        );

        // compute offsets for each attribute and the total vertex stride
        for (auto &attribute : vertexAttributeDescriptions_) {
            auto formatSize = getVertexFormatSize(attribute.format);
            attribute.offset = bindingDesc_.stride;
            bindingDesc_.stride += formatSize;
        }
    }

    /* REFLECT PUSH CONSTANT BLOCKS */
    uint32_t pushConstantBlockCount = 0;
    result = spvReflectEnumeratePushConstantBlocks(&module, &pushConstantBlockCount, nullptr);
    if (result != SPV_REFLECT_RESULT_SUCCESS) {
        throw std::runtime_error("Unable to enumerate push constant blocks!");
    }

    std::vector<SpvReflectBlockVariable*> pushConstantBlocks(pushConstantBlockCount);
    result = spvReflectEnumeratePushConstantBlocks(&module, &pushConstantBlockCount, pushConstantBlocks.data());
    if (result != SPV_REFLECT_RESULT_SUCCESS) {
        throw std::runtime_error("Unable to enumerate push constant blocks!");
    }

    pushConstantRanges_.clear();// ensure this is empty
    for (auto i = 0u; i < pushConstantBlocks.size(); i++) {
        auto *pushConstant = pushConstantBlocks[i];
        
        VkPushConstantRange range = {};
        range.size = pushConstant->size;
        range.offset = pushConstant->offset;

        switch (stage_) {
            case (ShaderModule::ShaderStage::Vertex): {
                range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            }
            break;
            case (ShaderModule::ShaderStage::TessellationControl): {
                range.stageFlags = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
            }
            break;
            case (ShaderModule::ShaderStage::TessellationEval): {
                range.stageFlags = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            }
            break;
            case (ShaderModule::ShaderStage::Geometry): {
                range.stageFlags = VK_SHADER_STAGE_GEOMETRY_BIT;
            }
            break;
            case (ShaderModule::ShaderStage::Fragment): {
                range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            }
            break;
        }

        pushConstantRanges_.push_back(range);
    }

    // TODO: i think tessellation info can be reflected?
        // i honestly have no clue though, i'm planning to add tessellation in one big pass way later on 
        // after all the basic stuff is working; there will have to be a bunch of research done of course

    // whatever other reflection is required!

}

ShaderModule::~ShaderModule() {
    // make sure the context gets destroyed last of all the vulkan objects to keep this clean
    vkDestroyShaderModule(context_->getLogicalDevice(), shaderModule_, nullptr);
}