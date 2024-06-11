//
// Created by paull on 2023-11-22.
//

#pragma once

#include <vector>
#include <memory>
#include <algorithm>

#include "../../../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"
#include "../../../GraphicsIR/ShaderModuleIR/ShaderModuleIR.hpp"

#include "ShaderModuleReflectionData/ShaderModuleReflectionData.hpp"
#include "../VulkanDescriptorSetAllocator/VulkanDescriptorSetAllocator.hpp"
#include "../VulkanShaderModule/VulkanShaderModule.hpp"
#include "VulkanFormatSize.hpp"

#include <vulkan/vulkan.h>

// TODO - probably "rephrase" these constants so theyre not defines
#define DEFAULT_INITIAL_DESCRIPTOR_POOL_SIZE 100 // arbitrary - maybe tune these
#define DEFAULT_MAX_DESCRIPTOR_SETS_PER_POOL 200 // arbitrary

namespace pEngine::girEngine::backend::vulkan {

    /**
     * The idea with a Program is that it should basically group up all the shaders
     * in a graphics pipeline (so make sure that we refactor that to use this class)
     * and reflect a bunch of information from them.\n\n
     *
     * The two main pieces of functionality (until I come across other things that need support) will be
     * the reflection of a Vulkan pipeline layout and the reflection of a set of descriptor set allocators
     * for each descriptor set layout that is reflected from the shaders. \n\n
     *
     * So the program will basically contain a set of reflected shader module information,
     * a set of descriptor set allocators for each descriptor set layout that is reflected from the given shaders,
     * and then presumably 1 pipeline layout that was built from reflected information...
     *
     * That seems to make sense and it could just plug right into the render pass classes and provide them with
     * that info and the ability to allocate descriptors for that program
     */
    class VulkanProgram {
    public:
        struct CreationInput {
            std::string name;
            util::UniqueIdentifier uid;

            unsigned numberOfWorkerThreads = 1u;

            // set of shader modules to reflect from
            std::vector<std::shared_ptr<VulkanShaderModule>> shaderModulesToReflect = {};

            // TODO - add in vulkan push constant attachments and compare them against what gets reflected

            const std::vector<std::shared_ptr<gir::DrawAttachmentIR>> &drawAttachments = {};

            const VkDevice &device;
        };

        explicit VulkanProgram(const CreationInput &creationInput)
                : device(creationInput.device),
                  name(creationInput.name),
                  uid(creationInput.uid),
                  shaderModuleReflections({}),
                  pipelineLayout(nullptr),
                  vertexInputBindings({}),
                  vertexAttributes({}) {

            // first obtain shader module reflection data for each shader module
            for (auto &inputShaderModule: creationInput.shaderModulesToReflect) {
                // note: we also obtain the input/output info from vertex shaders if present; can probably move that out
                // and also add sanity checking to make sure a vertex shader is present or else tank the whole process
                shaderModuleReflections.push_back(reflectShaderInfo(*inputShaderModule, creationInput.drawAttachments));
            }

            // idea from here: use the shader module reflection information to build descriptor set allocators and pipeline layout
            for (auto &shaderModuleReflection: shaderModuleReflections) {
                for (auto &descriptorSetLayout: shaderModuleReflection.descriptorSetLayoutInfos) {
                    descriptorSetAllocators.push_back(
                            std::make_unique<descriptor::VulkanDescriptorSetAllocator>(
                                    descriptor::VulkanDescriptorSetAllocator::CreationInput{
                                            descriptorSetLayout.setNumber,
                                            creationInput.device,
                                            descriptorSetLayout.layoutCreateInfo,
                                            creationInput.numberOfWorkerThreads,
                                            DEFAULT_INITIAL_DESCRIPTOR_POOL_SIZE,
                                            DEFAULT_MAX_DESCRIPTOR_SETS_PER_POOL
                                    }));
                }
            }

            // okay, so it doesn't look like we're building a pipeline layout yet; they seem to mainly
            // just amalgamate the descriptor set layouts and push constant ranges;

            // I'm fairly certain we have to basically amalgamate all of them into one group by binding index...?
            // actually the descriptor set layouts themselves will already hold the information about individual
            // binding indices for each descriptor, so we should be able to just shunt them all into one big list
            std::vector<VkDescriptorSetLayout> aggregatedLayouts = {};
            aggregatedLayouts.reserve(descriptorSetAllocators.size());
            for (const auto &descriptorSetAllocator: descriptorSetAllocators) {
                aggregatedLayouts.push_back(descriptorSetAllocator->getDescriptorSetLayout());
            }

            std::vector<VkPushConstantRange> aggregatedPushConstantRanges = {};
            for (const auto &reflectionData: shaderModuleReflections) {
                for (const auto &pushConstantRange: reflectionData.pushConstantRanges) {
                    aggregatedPushConstantRanges.push_back(pushConstantRange);
                }
            }

            VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
                    VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                    nullptr,
                    0,
                    static_cast<uint32_t>(aggregatedLayouts.size()),
                    aggregatedLayouts.data(),
                    static_cast<uint32_t>(aggregatedPushConstantRanges.size()),
                    aggregatedPushConstantRanges.data()
            };
            if (auto result = vkCreatePipelineLayout(device,
                                                     &pipelineLayoutCreateInfo,
                                                     nullptr,
                                                     &pipelineLayout);
                    result != VK_SUCCESS) {
                // TODO - log, probably don't throw
                throw std::runtime_error("Error in VulkanProgram() -> unable to create pipeline layout!");
            }
        }

        ~VulkanProgram() {
            vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        }

        [[nodiscard]] const std::vector<program::ShaderModuleReflectionData> &getReflectionDataList() const {
            return shaderModuleReflections;
        }

        // TODO - getter for DS allocators

        [[nodiscard]] const VkPipelineLayout &getPipelineLayout() const {
            return pipelineLayout;
        }

        [[nodiscard]] const std::vector<VkVertexInputBindingDescription> &getVertexInputBindingDescriptions() const {
            return vertexInputBindings;
        }

        [[nodiscard]] const std::vector<VkVertexInputAttributeDescription> &
        getVertexInputAttributeDescriptions() const {
            return vertexAttributes;
        }

    private:
        std::string name;
        util::UniqueIdentifier uid;

        VkDevice device;

        // list of each module's reflected shader information
        std::vector<program::ShaderModuleReflectionData> shaderModuleReflections = {};

        // New thought: moving these vertex-shader-specific fields out so that we basically only look for the vertex
        // shader and take the information there; we need to be able to hand it off up-front

        std::vector<VkVertexInputBindingDescription> vertexInputBindings;

        std::vector<VkVertexInputAttributeDescription> vertexAttributes;

        // descriptor set allocators
        std::vector<std::unique_ptr<descriptor::VulkanDescriptorSetAllocator>> descriptorSetAllocators = {};

        // at least one pipeline layout (not sure if we need more than one but it seems like it'd be 1 per pipeline)
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

        // TODO - refactor all the janky reflection code I yanked over here from the VulkanShaderModule class I wrote way back when
        // on the other hand, it's a lil odd but I think having C-style procedural logic implemented with static is kewl for now

        program::ShaderModuleReflectionData
        reflectShaderInfo(const VulkanShaderModule &shaderModule,
                          const std::vector<std::shared_ptr<gir::DrawAttachmentIR>> &drawAttachments) {
            const std::vector<uint32_t> &spirvCode = shaderModule.getSpirVByteCode();
            SpvReflectShaderModule reflectShaderModule = {};
            SpvReflectResult createShaderModuleResult =
                    spvReflectCreateShaderModule(spirvCode.size() * sizeof(uint32_t),
                                                 spirvCode.data(),
                                                 &reflectShaderModule);

            if (createShaderModuleResult != SPV_REFLECT_RESULT_SUCCESS) {
                // TODO - something other than throw
                throw std::runtime_error("Unable to create SPIRV-Reflect shader module!");
            }

            program::ShaderModuleReflectionData shaderModuleReflectionData;
            gir::ShaderModuleIR::ShaderUsage shaderUsage = shaderModule.getUsage();
            if (!reflectedShaderUsageMatchesShaderModule(reflectShaderModule.shader_stage, shaderUsage)) {
                throw std::runtime_error(
                        "Reflected shader module usage does not match user-specified shader module usage!");
            }

            reflectDescriptorSetInformation(reflectShaderModule, shaderModuleReflectionData);

            reflectInputOutputVariables(reflectShaderModule, shaderModuleReflectionData, shaderUsage, drawAttachments);

            reflectPushConstantInformation(reflectShaderModule, shaderModuleReflectionData, shaderUsage);

            // TODO - reflect tessellation info as well



            return shaderModuleReflectionData;
        }

        static void reflectPushConstantInformation(SpvReflectShaderModule &reflectShaderModule,
                                                   program::ShaderModuleReflectionData &shaderReflectionData,
                                                   gir::ShaderModuleIR::ShaderUsage shaderUsage) {
            uint32_t pushConstantBlockCount = 0;
            SpvReflectResult enumeratePushConstantBlocksResult =
                    spvReflectEnumeratePushConstantBlocks(&reflectShaderModule, &pushConstantBlockCount, nullptr);
            if (enumeratePushConstantBlocksResult != SPV_REFLECT_RESULT_SUCCESS) {
                throw std::runtime_error("Unable to enumerate push constant blocks!");
            }

            std::vector<SpvReflectBlockVariable *> pushConstantBlocks(pushConstantBlockCount);
            enumeratePushConstantBlocksResult =
                    spvReflectEnumeratePushConstantBlocks(&reflectShaderModule, &pushConstantBlockCount,
                                                          pushConstantBlocks.data());
            if (enumeratePushConstantBlocksResult != SPV_REFLECT_RESULT_SUCCESS) {
                throw std::runtime_error("Unable to enumerate push constant blocks!");
            }

            shaderReflectionData.pushConstantRanges.clear();
            for (auto pushConstant: pushConstantBlocks) {
                VkPushConstantRange range = {};
                range.size = pushConstant->size;
                range.offset = pushConstant->offset;

                switch (shaderUsage) {
                    case (gir::ShaderModuleIR::ShaderUsage::VERTEX_SHADER): {
                        range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                        break;
                    }
                    case (gir::ShaderModuleIR::ShaderUsage::FRAGMENT_SHADER): {
                        range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                        break;
                    }
                    case (gir::ShaderModuleIR::ShaderUsage::TESSELLATION_CONTROL_SHADER): {
                        range.stageFlags = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
                        break;
                    }
                    case (gir::ShaderModuleIR::ShaderUsage::TESSELLATION_EVALUATION_SHADER): {
                        range.stageFlags = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
                        break;
                    }
                    case (gir::ShaderModuleIR::ShaderUsage::GEOMETRY_SHADER): {
                        range.stageFlags = VK_SHADER_STAGE_GEOMETRY_BIT;
                        break;
                    }
                    default: {
                        // TODO - log!
                    }
                }

                shaderReflectionData.pushConstantRanges.push_back(range);
            }
        }

        /**
         * Note: this is part of some C-style procedural stuff that kinda breaks with a lot of the other code style. \n\n
         *
         * It's following the code examples for SPIR-V reflect mainly; I will probably come through and rewrite this
         * to be consistent with the rest of the engine if necessary but it seems fine. Procedural programming -> GOAT
         * programming paradigm anyway. ;) \n\n
         *
         *
         * @param reflectShaderModule
         * @param shaderReflectionData
         * @param shaderUsage
         */
        void reflectInputOutputVariables(
                SpvReflectShaderModule &reflectShaderModule,
                program::ShaderModuleReflectionData &shaderReflectionData,
                const gir::ShaderModuleIR::ShaderUsage &shaderUsage,
                const std::vector<std::shared_ptr<gir::DrawAttachmentIR>> &drawAttachments) {
            // enumerate the input variables
            uint32_t inputCount = 0;
            SpvReflectResult enumerateInputVariablesResult =
                    spvReflectEnumerateInputVariables(&reflectShaderModule, &inputCount, nullptr);
            if (enumerateInputVariablesResult != SPV_REFLECT_RESULT_SUCCESS) {
                throw std::runtime_error("Unable to enumerate input variables for shader ");
            }
            std::vector<SpvReflectInterfaceVariable *> inputs(inputCount);
            enumerateInputVariablesResult = spvReflectEnumerateInputVariables(&reflectShaderModule, &inputCount,
                                                                              inputs.data());
            if (enumerateInputVariablesResult != SPV_REFLECT_RESULT_SUCCESS) {
                throw std::runtime_error("Unable to enumerate input variables for shader ");
            }

            // enumerate output variables
            uint32_t outputCount = 0;
            SpvReflectResult enumerateOutputVariablesResult =
                    spvReflectEnumerateOutputVariables(&reflectShaderModule, &outputCount, nullptr);
            if (enumerateOutputVariablesResult != SPV_REFLECT_RESULT_SUCCESS) {
                throw std::runtime_error("Unable to enumerate output variables for shader ");
            }
            std::vector<SpvReflectInterfaceVariable *> outputs(outputCount);
            enumerateOutputVariablesResult =
                    spvReflectEnumerateOutputVariables(&reflectShaderModule, &outputCount, outputs.data());
            if (enumerateOutputVariablesResult != SPV_REFLECT_RESULT_SUCCESS) {
                throw std::runtime_error("Unable to enumerate output variables for shader ");
            }
            // TODO - probably actually store these output variable things! Or use them! They're only being acquired rn

            if (shaderUsage == gir::ShaderModuleIR::ShaderUsage::VERTEX_SHADER) {
                // one very very important thing: **INPUTS CAN INCLUDE THINGS OTHER THAN VERTEX ATTRIBUTES (obviously)**
                // this means we'll have to prefilter the list and acquire new lists of each type of reflected variable
                processVertexAttributes(drawAttachments, inputs);

                // TODO - any other vertex-shader specific processing that we should do

            }
        }

        void processVertexAttributes(const std::vector<std::shared_ptr<gir::DrawAttachmentIR>> &drawAttachments,
                                     const std::vector<SpvReflectInterfaceVariable *> &inputs) {
            std::vector<SpvReflectInterfaceVariable *> vertexAttributeInterfaceVariableReflections = {};
            // TODO - handle other inputs (uniform blocks, etc)
            for (auto &input: inputs) {
                if (input->storage_class == SpvStorageClassInput) {
                    // I think vertex attributes fall under this category of storage class?
                    // additionally, they should be the only variables that are labeled "in" for a vert shader (I think...?)
                    vertexAttributeInterfaceVariableReflections.push_back(input);
                }
            }

            // build up global list of vertex input attribs...?
            // I think right now the only purpose this loop is serving is to basically read in the reflected locations;
            // it's possible a good portion of the existing logic only makes sense for the simplifying assumptions in
            // the example I'm going off for this shite.
            // maybe it'll be worth it to just do all of this in-place kinda (take each attribute, match it up against a binding,
            // and then kinda advance through all the bindings and match attributes up as you go until you reach the end)
            // that seems nice and pretty but it'll require a bit of engineering and testing that I think is probably worth postponing for
            // a later update...
            vertexAttributes.resize(vertexAttributeInterfaceVariableReflections.size());
            for (size_t inputVar = 0; inputVar < vertexAttributeInterfaceVariableReflections.size(); inputVar++) {
                const auto &reflectedVar = *vertexAttributeInterfaceVariableReflections[inputVar];
                auto &desc = vertexAttributes[inputVar];

                desc.location = reflectedVar.location;
                desc.binding = 0; // note we'll have to set this for each attribute as we advance thru the bindings
                desc.format = static_cast<VkFormat>( reflectedVar.format );
                desc.offset = 0; // these will also have to be set, but they'll be computed dynamically (which was already being done)
            }

            // match them up to the bindings
            unsigned bindingIndex = 0;
            unsigned globalAttributeListIndex = 0;
            for (const auto &drawAttachment: drawAttachments) {
                vertexInputBindings.push_back({});
                VkVertexInputBindingDescription vertexInputBindingDescription = vertexInputBindings.back();

                vertexInputBindingDescription.binding = bindingIndex;
                vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // TODO - make this configurable
                vertexInputBindingDescription.stride = 0;

                // obtain *this attachment's* input attributes first;
                // since we're expecting the user to set things up to match 1-1 b/w their shaders and their data specification,
                // the logic can be to just grab the next `n` of them where `n` is the drawAttachmentVertexAttributeCount of the draw attachment's vertex attribute list
                const auto drawAttachmentVertexAttributeCount = drawAttachment->getVertexAttributes().size();
                std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions(
                        drawAttachmentVertexAttributeCount);
                for (unsigned currentVertexAttributeIndex = globalAttributeListIndex;
                     currentVertexAttributeIndex < globalAttributeListIndex + drawAttachmentVertexAttributeCount;
                     currentVertexAttributeIndex++) {
                    vertexInputAttributeDescriptions[currentVertexAttributeIndex]
                            = vertexAttributes[currentVertexAttributeIndex];
                }
                globalAttributeListIndex += drawAttachmentVertexAttributeCount;

                // TODO - deduce layouts and what not
                // for now, I'm gonna leave layouts unhandled because they won't actually need to work until we are drawing triangles

                // sort *this attachment's* input attributes (we'll need to make a smaller list of only the attributes for this list)
                std::sort(
                        std::begin(vertexInputAttributeDescriptions),
                        std::end(vertexInputAttributeDescriptions),
                        [&](const VkVertexInputAttributeDescription &first,
                            const VkVertexInputAttributeDescription &second) {
                            // we want to sort in ascending order, so we set this condition
                            return first.location < second.location;
                        });

                for (auto &attribute: vertexInputAttributeDescriptions) {
                    attribute.offset = vertexInputBindingDescription.stride;
                    vertexInputBindingDescription.stride +=
                            getVulkanFormatSizeInBytes(attribute.format);
                }

                bindingIndex++;
            }
        }

        static void reflectDescriptorSetInformation(SpvReflectShaderModule &reflectShaderModule,
                                                    program::ShaderModuleReflectionData &shaderReflectionData) {
            auto reflectedDescriptorSets = enumerateDescriptorSets(reflectShaderModule);

            for (const auto &reflectedDescriptorSet: reflectedDescriptorSets) {
                // we reflect descriptor sets one by one:
                const SpvReflectDescriptorSet &reflectionSet = *reflectedDescriptorSet;
                program::ShaderModuleReflectionData::DescriptorSetLayoutInfo info;

                obtainDescriptorSetLayoutBindingInfos(reflectShaderModule, reflectedDescriptorSet, info);

                info.setNumber = reflectionSet.set;
                info.layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                info.layoutCreateInfo.pNext = nullptr;
                info.layoutCreateInfo.flags = 0;
                info.layoutCreateInfo.bindingCount = static_cast<uint32_t>( info.bindings.size());
                info.layoutCreateInfo.pBindings = info.bindings.data();

                shaderReflectionData.descriptorSetLayoutInfos.push_back(info);
            }
        }

        static void obtainDescriptorSetLayoutBindingInfos(
                const SpvReflectShaderModule &reflectShaderModule,
                SpvReflectDescriptorSet *const &reflectedDescriptorSet,
                program::ShaderModuleReflectionData::DescriptorSetLayoutInfo &info) {
            info.bindings.reserve(reflectedDescriptorSet->binding_count);
            for (auto j = 0u; j < info.bindings.size(); j++) {
                const auto &reflectedBinding = *reflectedDescriptorSet->bindings[j];
                auto &infoBinding = info.bindings[j];

                infoBinding.binding = reflectedBinding.binding;
                infoBinding.descriptorType = static_cast<VkDescriptorType>( reflectedBinding.descriptor_type );

                infoBinding.descriptorCount = 1;
                for (uint32_t bindingDim = 0; bindingDim < reflectedBinding.array.dims_count; bindingDim++) {
                    infoBinding.descriptorCount *= reflectedBinding.array.dims[bindingDim];
                }

                infoBinding.stageFlags = static_cast<VkShaderStageFlagBits>( reflectShaderModule.shader_stage );
            }
        }

        static std::vector<SpvReflectDescriptorSet *>
        enumerateDescriptorSets(SpvReflectShaderModule &reflectShaderModule) {
            uint32_t count = 0;
            SpvReflectResult enumerateDescriptorSetsResult = spvReflectEnumerateDescriptorSets(&reflectShaderModule,
                                                                                               &count, nullptr);
            if (enumerateDescriptorSetsResult != SPV_REFLECT_RESULT_SUCCESS) {
                // TODO - something!
            }

            std::vector<SpvReflectDescriptorSet *> sets(count);
            enumerateDescriptorSetsResult = spvReflectEnumerateDescriptorSets(&reflectShaderModule, &count,
                                                                              sets.data());
            if (enumerateDescriptorSetsResult != SPV_REFLECT_RESULT_SUCCESS) {
                // TODO - something!
            }

            return sets;
        }

        static bool reflectedShaderUsageMatchesShaderModule(SpvReflectShaderStageFlagBits bits,
                                                            gir::ShaderModuleIR::ShaderUsage shaderUsageToCompareAgainst) {
            gir::ShaderModuleIR::ShaderUsage reflectedShaderUsage;
            switch (bits) {
                case SPV_REFLECT_SHADER_STAGE_VERTEX_BIT:
                    reflectedShaderUsage = gir::ShaderModuleIR::ShaderUsage::VERTEX_SHADER;
                    break;
                case SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT:
                    reflectedShaderUsage = gir::ShaderModuleIR::ShaderUsage::FRAGMENT_SHADER;
                    break;
                case SPV_REFLECT_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
                    reflectedShaderUsage = gir::ShaderModuleIR::ShaderUsage::TESSELLATION_CONTROL_SHADER;
                    break;
                case SPV_REFLECT_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
                    reflectedShaderUsage = gir::ShaderModuleIR::ShaderUsage::TESSELLATION_EVALUATION_SHADER;
                    break;
                case SPV_REFLECT_SHADER_STAGE_GEOMETRY_BIT:
                    reflectedShaderUsage = gir::ShaderModuleIR::ShaderUsage::GEOMETRY_SHADER;
                    break;
                default: {
                    // TODO - log!
                }
            }

            return reflectedShaderUsage == shaderUsageToCompareAgainst;
        }

    };

} // vulkan
