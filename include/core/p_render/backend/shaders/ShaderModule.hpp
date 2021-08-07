#pragma once

// this will be the object that actually pulls the module from the file system and sets it up 
// to be used in a graphics pipeline / for reflection i think

// maybe we can even include some of that functionality in this class

#include "../../PRender.hpp"

#include <fstream>
#include <filesystem>

namespace Backend {

// here we'll declare some other that will probably be useful

// this should be everything needed to get descriptor set layouts for this shader module
struct DescriptorSetLayoutInfo {
    unsigned int setNumber = 0;
    VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
    std::vector<VkDescriptorSetLayoutBinding> bindings = {};
};

// this is the same function as given in the SPIRVReflect repo, 
// i'll use it to help compute vertex info 
static uint32_t getVertexFormatSize(VkFormat format) {
  uint32_t result = 0;
  switch (format) {
  case VK_FORMAT_UNDEFINED: result = 0; break;
  case VK_FORMAT_R4G4_UNORM_PACK8: result = 1; break;
  case VK_FORMAT_R4G4B4A4_UNORM_PACK16: result = 2; break;
  case VK_FORMAT_B4G4R4A4_UNORM_PACK16: result = 2; break;
  case VK_FORMAT_R5G6B5_UNORM_PACK16: result = 2; break;
  case VK_FORMAT_B5G6R5_UNORM_PACK16: result = 2; break;
  case VK_FORMAT_R5G5B5A1_UNORM_PACK16: result = 2; break;
  case VK_FORMAT_B5G5R5A1_UNORM_PACK16: result = 2; break;
  case VK_FORMAT_A1R5G5B5_UNORM_PACK16: result = 2; break;
  case VK_FORMAT_R8_UNORM: result = 1; break;
  case VK_FORMAT_R8_SNORM: result = 1; break;
  case VK_FORMAT_R8_USCALED: result = 1; break;
  case VK_FORMAT_R8_SSCALED: result = 1; break;
  case VK_FORMAT_R8_UINT: result = 1; break;
  case VK_FORMAT_R8_SINT: result = 1; break;
  case VK_FORMAT_R8_SRGB: result = 1; break;
  case VK_FORMAT_R8G8_UNORM: result = 2; break;
  case VK_FORMAT_R8G8_SNORM: result = 2; break;
  case VK_FORMAT_R8G8_USCALED: result = 2; break;
  case VK_FORMAT_R8G8_SSCALED: result = 2; break;
  case VK_FORMAT_R8G8_UINT: result = 2; break;
  case VK_FORMAT_R8G8_SINT: result = 2; break;
  case VK_FORMAT_R8G8_SRGB: result = 2; break;
  case VK_FORMAT_R8G8B8_UNORM: result = 3; break;
  case VK_FORMAT_R8G8B8_SNORM: result = 3; break;
  case VK_FORMAT_R8G8B8_USCALED: result = 3; break;
  case VK_FORMAT_R8G8B8_SSCALED: result = 3; break;
  case VK_FORMAT_R8G8B8_UINT: result = 3; break;
  case VK_FORMAT_R8G8B8_SINT: result = 3; break;
  case VK_FORMAT_R8G8B8_SRGB: result = 3; break;
  case VK_FORMAT_B8G8R8_UNORM: result = 3; break;
  case VK_FORMAT_B8G8R8_SNORM: result = 3; break;
  case VK_FORMAT_B8G8R8_USCALED: result = 3; break;
  case VK_FORMAT_B8G8R8_SSCALED: result = 3; break;
  case VK_FORMAT_B8G8R8_UINT: result = 3; break;
  case VK_FORMAT_B8G8R8_SINT: result = 3; break;
  case VK_FORMAT_B8G8R8_SRGB: result = 3; break;
  case VK_FORMAT_R8G8B8A8_UNORM: result = 4; break;
  case VK_FORMAT_R8G8B8A8_SNORM: result = 4; break;
  case VK_FORMAT_R8G8B8A8_USCALED: result = 4; break;
  case VK_FORMAT_R8G8B8A8_SSCALED: result = 4; break;
  case VK_FORMAT_R8G8B8A8_UINT: result = 4; break;
  case VK_FORMAT_R8G8B8A8_SINT: result = 4; break;
  case VK_FORMAT_R8G8B8A8_SRGB: result = 4; break;
  case VK_FORMAT_B8G8R8A8_UNORM: result = 4; break;
  case VK_FORMAT_B8G8R8A8_SNORM: result = 4; break;
  case VK_FORMAT_B8G8R8A8_USCALED: result = 4; break;
  case VK_FORMAT_B8G8R8A8_SSCALED: result = 4; break;
  case VK_FORMAT_B8G8R8A8_UINT: result = 4; break;
  case VK_FORMAT_B8G8R8A8_SINT: result = 4; break;
  case VK_FORMAT_B8G8R8A8_SRGB: result = 4; break;
  case VK_FORMAT_A8B8G8R8_UNORM_PACK32: result = 4; break;
  case VK_FORMAT_A8B8G8R8_SNORM_PACK32: result = 4; break;
  case VK_FORMAT_A8B8G8R8_USCALED_PACK32: result = 4; break;
  case VK_FORMAT_A8B8G8R8_SSCALED_PACK32: result = 4; break;
  case VK_FORMAT_A8B8G8R8_UINT_PACK32: result = 4; break;
  case VK_FORMAT_A8B8G8R8_SINT_PACK32: result = 4; break;
  case VK_FORMAT_A8B8G8R8_SRGB_PACK32: result = 4; break;
  case VK_FORMAT_A2R10G10B10_UNORM_PACK32: result = 4; break;
  case VK_FORMAT_A2R10G10B10_SNORM_PACK32: result = 4; break;
  case VK_FORMAT_A2R10G10B10_USCALED_PACK32: result = 4; break;
  case VK_FORMAT_A2R10G10B10_SSCALED_PACK32: result = 4; break;
  case VK_FORMAT_A2R10G10B10_UINT_PACK32: result = 4; break;
  case VK_FORMAT_A2R10G10B10_SINT_PACK32: result = 4; break;
  case VK_FORMAT_A2B10G10R10_UNORM_PACK32: result = 4; break;
  case VK_FORMAT_A2B10G10R10_SNORM_PACK32: result = 4; break;
  case VK_FORMAT_A2B10G10R10_USCALED_PACK32: result = 4; break;
  case VK_FORMAT_A2B10G10R10_SSCALED_PACK32: result = 4; break;
  case VK_FORMAT_A2B10G10R10_UINT_PACK32: result = 4; break;
  case VK_FORMAT_A2B10G10R10_SINT_PACK32: result = 4; break;
  case VK_FORMAT_R16_UNORM: result = 2; break;
  case VK_FORMAT_R16_SNORM: result = 2; break;
  case VK_FORMAT_R16_USCALED: result = 2; break;
  case VK_FORMAT_R16_SSCALED: result = 2; break;
  case VK_FORMAT_R16_UINT: result = 2; break;
  case VK_FORMAT_R16_SINT: result = 2; break;
  case VK_FORMAT_R16_SFLOAT: result = 2; break;
  case VK_FORMAT_R16G16_UNORM: result = 4; break;
  case VK_FORMAT_R16G16_SNORM: result = 4; break;
  case VK_FORMAT_R16G16_USCALED: result = 4; break;
  case VK_FORMAT_R16G16_SSCALED: result = 4; break;
  case VK_FORMAT_R16G16_UINT: result = 4; break;
  case VK_FORMAT_R16G16_SINT: result = 4; break;
  case VK_FORMAT_R16G16_SFLOAT: result = 4; break;
  case VK_FORMAT_R16G16B16_UNORM: result = 6; break;
  case VK_FORMAT_R16G16B16_SNORM: result = 6; break;
  case VK_FORMAT_R16G16B16_USCALED: result = 6; break;
  case VK_FORMAT_R16G16B16_SSCALED: result = 6; break;
  case VK_FORMAT_R16G16B16_UINT: result = 6; break;
  case VK_FORMAT_R16G16B16_SINT: result = 6; break;
  case VK_FORMAT_R16G16B16_SFLOAT: result = 6; break;
  case VK_FORMAT_R16G16B16A16_UNORM: result = 8; break;
  case VK_FORMAT_R16G16B16A16_SNORM: result = 8; break;
  case VK_FORMAT_R16G16B16A16_USCALED: result = 8; break;
  case VK_FORMAT_R16G16B16A16_SSCALED: result = 8; break;
  case VK_FORMAT_R16G16B16A16_UINT: result = 8; break;
  case VK_FORMAT_R16G16B16A16_SINT: result = 8; break;
  case VK_FORMAT_R16G16B16A16_SFLOAT: result = 8; break;
  case VK_FORMAT_R32_UINT: result = 4; break;
  case VK_FORMAT_R32_SINT: result = 4; break;
  case VK_FORMAT_R32_SFLOAT: result = 4; break;
  case VK_FORMAT_R32G32_UINT: result = 8; break;
  case VK_FORMAT_R32G32_SINT: result = 8; break;
  case VK_FORMAT_R32G32_SFLOAT: result = 8; break;
  case VK_FORMAT_R32G32B32_UINT: result = 12; break;
  case VK_FORMAT_R32G32B32_SINT: result = 12; break;
  case VK_FORMAT_R32G32B32_SFLOAT: result = 12; break;
  case VK_FORMAT_R32G32B32A32_UINT: result = 16; break;
  case VK_FORMAT_R32G32B32A32_SINT: result = 16; break;
  case VK_FORMAT_R32G32B32A32_SFLOAT: result = 16; break;
  case VK_FORMAT_R64_UINT: result = 8; break;
  case VK_FORMAT_R64_SINT: result = 8; break;
  case VK_FORMAT_R64_SFLOAT: result = 8; break;
  case VK_FORMAT_R64G64_UINT: result = 16; break;
  case VK_FORMAT_R64G64_SINT: result = 16; break;
  case VK_FORMAT_R64G64_SFLOAT: result = 16; break;
  case VK_FORMAT_R64G64B64_UINT: result = 24; break;
  case VK_FORMAT_R64G64B64_SINT: result = 24; break;
  case VK_FORMAT_R64G64B64_SFLOAT: result = 24; break;
  case VK_FORMAT_R64G64B64A64_UINT: result = 32; break;
  case VK_FORMAT_R64G64B64A64_SINT: result = 32; break;
  case VK_FORMAT_R64G64B64A64_SFLOAT: result = 32; break;
  case VK_FORMAT_B10G11R11_UFLOAT_PACK32: result = 4; break;
  case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32: result = 4; break;

  default:
    break;
  }
  return result;
}

class ShaderModule { 
  public:
    ShaderModule(const std::string &shaderFileName, std::shared_ptr<Context> context, std::shared_ptr<RenderGraph> graph);
    ~ShaderModule();

    enum ShaderStage {
        Undefined,
        Vertex,
        TessellationControl,
        TessellationEval,
        Geometry,
        Fragment
    };

    // interface 
    ShaderStage getShaderStage() {
        return stage_;
    }

    // get shader module
    VkShaderModule &getVkShaderModule() {
      assert(shaderModule_);
      return shaderModule_;
    }

    std::vector<DescriptorSetLayoutInfo> &getDescriptorSetLayoutInfos() {
        return descriptorSetLayoutInfos_;
    }

    const std::string &getShaderName() {
      return shaderFilename_;
    }

    std::vector<VkPushConstantRange> &getPushConstantRanges() {
      return pushConstantRanges_;
    }

  private:
    // associated render context
    std::shared_ptr<Context> context_ = nullptr;

    // associated graph environment
    std::shared_ptr<RenderGraph> graph_ = nullptr;

    // store the stage that the shader is intended to be used for
    ShaderStage stage_ = ShaderStage::Undefined;

    // shader filename (not including an extension)
    std::string shaderFilename_;

    std::vector<uint32_t> spvCode_ = {};

    // the actual shader module
    VkShaderModule shaderModule_ = VK_NULL_HANDLE;

    // reflection info
    
    // everything needed to make our DSLs!
    std::vector<DescriptorSetLayoutInfo> descriptorSetLayoutInfos_ = {};

    // push constant ranges (i'll just reflect these directly i guess)
    std::vector<VkPushConstantRange> pushConstantRanges_ = {};

    // vertex info, which will be reflected from vertex shaders
        // does not support instancing, which will eventually have to 
    VkVertexInputBindingDescription bindingDesc_ = {};
    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo_ = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions_ = {};
    
    // tessellation info can go here when i add tessellation

    
};


}