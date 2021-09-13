#pragma once

#include "../../PRender.hpp"

#include "../../backend/Context.hpp"
#include "../../render_graph/pass/Subpass.hpp"

#include <memory>
#include <vector>

namespace backend {

class Program {
  public:
    Program(std::shared_ptr<Context> context, std::shared_ptr<Subpass> subpass);
    ~Program();

    // program interface

    VkPipelineLayout &getPipelineLayout() {
        return pipelineLayout_;
    }

    std::unique_ptr<VkPipelineShaderStageCreateInfo> &getVertexStageInfo() {
        return vertexStage_;
    }

    std::unique_ptr<VkPipelineShaderStageCreateInfo> &getTessellationControlStage() {
        return tessellationControlStage_;
    }

    std::unique_ptr<VkPipelineShaderStageCreateInfo> &getTessellationEvalStage() {
        return tessellationEvalStage_;
    }

    std::unique_ptr<VkPipelineShaderStageCreateInfo> &getGeometryStage() {
        return geometryStage_;
    }

    std::unique_ptr<VkPipelineShaderStageCreateInfo> &getFragmentStage() {
        return fragmentStage_;
    }

  private:
    std::shared_ptr<Context> context_ = nullptr;

    std::shared_ptr<Subpass> subpass_ = nullptr;

    // each program should basically handle all the pipeline layout stuff
    // as well as the shader information (some of which is pre-computed during graph specification)
    VkPipelineLayout pipelineLayout_ = VK_NULL_HANDLE;
    
    std::unique_ptr<VkPipelineShaderStageCreateInfo> vertexStage_ = nullptr;
    std::unique_ptr<VkPipelineShaderStageCreateInfo> tessellationEvalStage_ = nullptr;
    std::unique_ptr<VkPipelineShaderStageCreateInfo> tessellationControlStage_ = nullptr;
    std::unique_ptr<VkPipelineShaderStageCreateInfo> geometryStage_ = nullptr;
    std::unique_ptr<VkPipelineShaderStageCreateInfo> fragmentStage_ = nullptr;

};

}