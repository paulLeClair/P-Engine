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

// void Subpass::execute() {
//     // if (!queueBuilt_) 
    
//     // i'm fairly sure that this should only execute as part of RenderPass execution,
//     // and so we can assume that commands are able to be submitted
//     buildJobQueue();

//     // need to flush the queue here, maybe the queue can have a flush() member function that
//     // calls a threadpool function that maybe creates a copy of the specified JobQueue and then
//     // activates all free threads to empty the queue copy
//         // only thing: it's obviously way faster to just get rid of the list as opposed to copying every time..
    
//     // honestly, i'm gonna just have it build the queue every time because it's probably a bad idea to repeatedly
//     // copy a jobqueue every time you execute a subpass 
//     // queue_.flush();
// }

// void Subpass::buildJobQueue() {
//     if (!buildJobQueue_) {
//         throw std::runtime_error("No callback registered!");
//     }

//     buildJobQueue_();
//     // queueBuilt_ = true;
// }