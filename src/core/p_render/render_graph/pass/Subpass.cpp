#include "../../../../../include/core/p_render/render_graph/pass/Subpass.hpp"

#include "../../../../../include/core/p_render/render_graph/pass/Pass.hpp"

#include "../../../../../include/core/thread_pool/ThreadPool.hpp"

#include "../../../../../include/core/p_render/render_graph/RenderGraph.hpp"

Subpass::Subpass(const std::string &name, std::shared_ptr<RenderGraph> graph, std::shared_ptr<ThreadPool> pool, std::shared_ptr<Pass> pass) {
    name_ = name;
    graph_ = graph;
    pool_ = pool;
    pass_ = pass;
}