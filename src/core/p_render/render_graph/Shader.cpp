#include "../../../../include/core/p_render/render_graph/Shader.hpp"

#include "../../../../include/core/p_render/render_graph/RenderGraph.hpp"

Shader::Shader(const std::string &subpassName, const std::string &shaderName, std::shared_ptr<RenderGraph> graph) {
    graph_ = std::move(graph);
    shaderName_ = shaderName;
    subpassName_ = subpassName;

    shaderModule_ = graph_->getShaderModule(shaderName);

    // whatever else!
}