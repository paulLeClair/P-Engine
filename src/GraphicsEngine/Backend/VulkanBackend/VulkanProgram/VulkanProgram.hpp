//
// Created by paull on 2023-11-22.
//

#pragma once

#include <vector>
#include <memory>
#include <algorithm>
#include <ranges>

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

            const VkDevice &device;

            unsigned numberOfWorkerThreads = 1u;

            // set of shader modules to reflect from
            const VulkanShaderModule *const vertexShaderModule;
            const VulkanShaderModule *const fragmentShaderModule;

            // TODO - add in vulkan push constant attachments and compare them against what gets reflected

            const std::vector<gir::DrawAttachmentIR> &drawAttachments = {};

            const std::vector<gir::vertex::VertexInputBindingIR> &vertexInputBindings = {};
        };

        explicit VulkanProgram(const CreationInput &creationInput)
            : name(creationInput.name),
              uid(creationInput.uid),
              device(creationInput.device),
              vertexInputBindings({}),
              vertexAttributes({}) {
            // first obtain shader module reflection data for each shader module
            vertexShaderData = reflectShaderInfo(*creationInput.vertexShaderModule, creationInput.drawAttachments,
                                                 creationInput.vertexInputBindings);
            fragmentShaderData = reflectShaderInfo(*creationInput.fragmentShaderModule, creationInput.drawAttachments,
                                                   creationInput.vertexInputBindings);


            // idea from here: use the shader module reflection information to build descriptor set allocators and pipeline layout
            auto maxLayoutSize = static_cast<uint32_t>(vertexShaderData.descriptorSetLayoutInfos.size()) >
                                 static_cast<uint32_t>(fragmentShaderData.descriptorSetLayoutInfos.size())
                                     ? static_cast<uint32_t>(vertexShaderData.descriptorSetLayoutInfos.size())
                                     : static_cast<uint32_t>(fragmentShaderData.descriptorSetLayoutInfos.size());
            for (uint32_t layoutIndex = 0;
                 layoutIndex < maxLayoutSize;
                 layoutIndex++) {
                // obtain all bindings across all stages
                std::vector<VkDescriptorSetLayoutBinding> layoutBindings = {};

                if (layoutIndex < vertexShaderData.descriptorSetLayoutInfos.size()) {
                    auto &descriptorSetLayout = vertexShaderData.descriptorSetLayoutInfos[layoutIndex];
                    for (auto &binding: descriptorSetLayout.bindings) {
                        layoutBindings.push_back(binding);
                    }
                }

                if (layoutIndex < fragmentShaderData.descriptorSetLayoutInfos.size()) {
                    auto &descriptorSetLayout = fragmentShaderData.descriptorSetLayoutInfos[layoutIndex];
                    for (auto &binding: descriptorSetLayout.bindings) {
                        layoutBindings.push_back(binding);
                    }
                }

                VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .bindingCount = static_cast<uint32_t>(layoutBindings.size()),
                    .pBindings = layoutBindings.data(),
                };
                descriptorSetAllocators.push_back(
                    std::make_unique<descriptor::VulkanDescriptorSetAllocator>(
                        descriptor::VulkanDescriptorSetAllocator::CreationInput{
                            layoutIndex,
                            creationInput.device,
                            // NOTE -> this object points to a variable that goes out of scope; be careful
                            layoutCreateInfo,
                            creationInput.numberOfWorkerThreads,
                            DEFAULT_INITIAL_DESCRIPTOR_POOL_SIZE,
                            DEFAULT_MAX_DESCRIPTOR_SETS_PER_POOL
                        }));
                descriptorSetLayoutsToAllocatorIndices[descriptorSetAllocators.back()->getDescriptorSetLayout()] =
                        layoutIndex;
            }

            std::vector<VkDescriptorSetLayout> aggregatedLayouts = {};
            for (const auto &descriptorSetAllocator: descriptorSetAllocators) {
                aggregatedLayouts.push_back(descriptorSetAllocator->getDescriptorSetLayout());
            }

            std::vector<VkPushConstantRange> aggregatedPushConstantRanges = {};

            for (const auto &pushConstantRange: vertexShaderData.pushConstantRanges) {
                aggregatedPushConstantRanges.push_back(pushConstantRange);
            }

            for (const auto &pushConstantRange: fragmentShaderData.pushConstantRanges) {
                aggregatedPushConstantRanges.push_back(pushConstantRange);
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
            if (const auto result = vkCreatePipelineLayout(device,
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

        [[nodiscard]] const program::ShaderModuleReflectionData &getVertexShaderModuleReflectionData() {
            return vertexShaderData;
        }

        [[nodiscard]] const program::ShaderModuleReflectionData &getFragmentShaderModuleReflectionData() {
            return fragmentShaderData;
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

        // might need to make this return a pair (with flag that indicates whether the returned set is fresh or reused)
        std::pair<VkDescriptorSet, bool> requestDescriptorSet(const VkDescriptorSetLayout &layout,
                                                              descriptor::ResourceDescriptorBindings &bindings,
                                                              uint32_t threadIndex = 0) const {
            if (!descriptorSetLayoutsToAllocatorIndices.contains(layout)) {
                // TODO - log that this happened!
                return {VK_NULL_HANDLE, false};
            }

            auto &allocator = descriptorSetAllocators[descriptorSetLayoutsToAllocatorIndices.at(layout)];
            return allocator->requestDescriptorSet(threadIndex, bindings);
        }

        std::vector<VkDescriptorSetLayout> getDescriptorSetLayouts() {
            auto keys = std::views::keys(descriptorSetLayoutsToAllocatorIndices);
            return std::vector<VkDescriptorSetLayout>{keys.begin(), keys.end()};
        }

    private:
        std::string name;
        util::UniqueIdentifier uid;

        VkDevice device;

        // each module's reflected shader information

        program::ShaderModuleReflectionData vertexShaderData;
        program::ShaderModuleReflectionData fragmentShaderData;
        // TODO -> other shader module types

        std::vector<VkVertexInputBindingDescription> vertexInputBindings = {};

        std::vector<VkVertexInputAttributeDescription> vertexAttributes;

        // descriptor set allocators
        // we may need to be wrapping our VkDescriptorSetLayout so that we can store more fine-grained info
        // about the components of said layout (
        std::unordered_map<VkDescriptorSetLayout, uint32_t> descriptorSetLayoutsToAllocatorIndices = {};
        std::vector<std::unique_ptr<descriptor::VulkanDescriptorSetAllocator> > descriptorSetAllocators = {};

        // at least one pipeline layout (not sure if we need more than one but it seems like it'd be 1 per pipeline)
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

        program::ShaderModuleReflectionData
        reflectShaderInfo(const VulkanShaderModule &shaderModule,
                          const std::vector<gir::DrawAttachmentIR> &drawAttachments,
                          const std::vector<gir::vertex::VertexInputBindingIR> &vertexInputBindings) {
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
            shaderModuleReflectionData.shaderModuleUsage = shaderUsage;

            reflectDescriptorSetInformation(reflectShaderModule, shaderModuleReflectionData);

            reflectInputOutputVariables(reflectShaderModule, shaderModuleReflectionData, shaderUsage, drawAttachments,
                                        vertexInputBindings);

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
            const std::vector<gir::DrawAttachmentIR> &drawAttachments,
            const std::vector<gir::vertex::VertexInputBindingIR> &vertexInputBindings) {
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
                processVertexAttributes(inputs, vertexInputBindings);

                // TODO - handle other inputs (uniform blocks, etc)

                // TODO - any other vertex-shader specific processing that we should do
            }
        }

        void processVertexAttributes(const std::vector<SpvReflectInterfaceVariable *> &inputs,
                                     const std::vector<gir::vertex::VertexInputBindingIR> &vertexInputBindings) {
            std::vector<SpvReflectInterfaceVariable *> vertexAttributeInterfaceVariableReflections = {};
            for (auto &input: inputs) {
                if (input->storage_class == SpvStorageClassInput) {
                    // I think vertex attributes fall under this category of storage class?
                    // additionally, they should be the only variables that are labeled "in" for a vert shader (I think...?)
                    vertexAttributeInterfaceVariableReflections.push_back(input);
                }
            }
            obtainVertexAttributes(vertexAttributeInterfaceVariableReflections, vertexInputBindings);
        }

        void obtainVertexAttributes(
            const std::vector<SpvReflectInterfaceVariable *> &vertexAttributeInterfaceVariables,
            const std::vector<gir::vertex::VertexInputBindingIR> &vertexInputBindingDescriptions) {
            // first we obtain the reflected info
            vertexAttributes.resize(vertexAttributeInterfaceVariables.size());
            std::map<VkVertexInputAttributeDescription *, std::string> attributeNames = {};
            for (size_t inputVarIndex = 0; inputVarIndex < vertexAttributeInterfaceVariables.size(); inputVarIndex++) {
                const auto &reflectedVar = *vertexAttributeInterfaceVariables[inputVarIndex];

                vertexAttributes[inputVarIndex].location = reflectedVar.location;
                vertexAttributes[inputVarIndex].binding = 0; // this will have to be obtained from user specification
                // TODO - set this for each attribute as we advance thru the bindings (once we support more than one binding per program)
                vertexAttributes[inputVarIndex].format = static_cast<VkFormat>(reflectedVar.format);
                vertexAttributes[inputVarIndex].offset = 0;

                attributeNames[&vertexAttributes[inputVarIndex]] = reflectedVar.name;
            }

            // sort our singular set of input attributes before we distribute them into concrete bindings
            // based on the user-specified info
            std::ranges::sort(vertexAttributes,
                              [&](const VkVertexInputAttributeDescription &first,
                                  const VkVertexInputAttributeDescription &second) {
                                  // we want to sort in ascending order, so we set this condition
                                  return first.location < second.location;
                              });

            for (uint32_t bindingIndex = 0; bindingIndex < vertexInputBindingDescriptions.size(); bindingIndex++) {
                // for each binding, we want to compute its particular stride and set the offsets
                // for the attributes within it

                // first we have to gather up the attributes for this particular binding from what we reflected;
                // this requires some sort of "key" that we can compare to determine whether the user-spec attribute
                // matches with a given shader attribute... unfortunately we don't have vertex attribute names for free
                VkVertexInputBindingDescription bindingDescription = {
                    bindingIndex,
                    0,
                    VK_VERTEX_INPUT_RATE_VERTEX
                };
                auto bindingAttributeDescriptions = vertexInputBindingDescriptions[bindingIndex].attributes;

                std::vector<VkVertexInputAttributeDescription> bindingAttributes = {};
                // here we want to extract only the attributes that match names with the user's attribute descriptions for this binding
                std::ranges::copy_if(vertexAttributes, std::back_inserter(bindingAttributes),
                                     [&](VkVertexInputAttributeDescription &attribute) {
                                         return std::ranges::any_of(bindingAttributeDescriptions,
                                                                    [&](const gir::vertex::VertexAttributeIR &
                                                                attribDescription) {
                                                                        return attributeNames[&attribute] ==
                                                                               attribDescription.attributeName;
                                                                    });
                                     });

                for (auto &attribute: vertexAttributes) {
                    attribute.offset = bindingDescription.stride;
                    bindingDescription.stride += getVulkanFormatSizeInBytes(attribute.format);
                }

                vertexInputBindings.push_back(bindingDescription);
            }
        }

        static void reflectDescriptorSetInformation(SpvReflectShaderModule &reflectShaderModule,
                                                    program::ShaderModuleReflectionData &shaderReflectionData) {
            auto reflectedDescriptorSets = enumerateDescriptorSets(reflectShaderModule);

            shaderReflectionData.descriptorSetLayoutInfos.resize(reflectedDescriptorSets.size());
            uint32_t index = 0;
            for (const auto &reflectedDescriptorSet: reflectedDescriptorSets) {
                // we reflect descriptor sets one by one:

                program::ShaderModuleReflectionData::DescriptorSetLayoutInfo info;
                obtainDescriptorSetLayoutBindingInfos(reflectShaderModule, reflectedDescriptorSet, info);

                info.setNumber = reflectedDescriptorSet->set;
                info.layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                info.layoutCreateInfo.pNext = nullptr;
                info.layoutCreateInfo.flags = 0;
                info.layoutCreateInfo.bindingCount = static_cast<uint32_t>(info.bindings.size());
                info.layoutCreateInfo.pBindings = info.bindings.data();

                shaderReflectionData.descriptorSetLayoutInfos[index] = info;
                index++;
            }
        }

        static void obtainDescriptorSetLayoutBindingInfos(
            const SpvReflectShaderModule &reflectShaderModule,
            SpvReflectDescriptorSet *const &reflectedDescriptorSet,
            program::ShaderModuleReflectionData::DescriptorSetLayoutInfo &info) {
            info.bindings.resize(reflectedDescriptorSet->binding_count);
            for (auto j = 0u; j < info.bindings.size(); j++) {
                const auto &reflectedBinding = *reflectedDescriptorSet->bindings[j];
                auto &infoBinding = info.bindings[j];

                infoBinding.binding = reflectedBinding.binding;
                infoBinding.descriptorType = static_cast<VkDescriptorType>(reflectedBinding.descriptor_type);

                infoBinding.descriptorCount = 1;
                for (uint32_t bindingDim = 0; bindingDim < reflectedBinding.array.dims_count; bindingDim++) {
                    infoBinding.descriptorCount *= reflectedBinding.array.dims[bindingDim];
                }

                infoBinding.stageFlags = static_cast<VkShaderStageFlagBits>(reflectShaderModule.shader_stage);
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
                                                            gir::ShaderModuleIR::ShaderUsage
                                                            shaderUsageToCompareAgainst) {
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
