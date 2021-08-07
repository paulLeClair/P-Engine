#include "../../../../include/core/p_render/render_graph/Shader.hpp"

#include "../../../../include/core/p_render/render_graph/RenderGraph.hpp"

Shader::Shader(const std::string &subpassName, const std::string &shaderName, std::shared_ptr<RenderGraph> graph) {
    graph_ = std::move(graph);
    shaderName_ = shaderName;
    subpassName_ = subpassName;

    // the shader has to be set up using the interface, i think we should at this point 
    // query/create the Backend::ShaderModule, although idk it's kinda useless except for the first time
    shaderModule_ = graph_->getShaderModule(shaderName);

    // whatever else!
}