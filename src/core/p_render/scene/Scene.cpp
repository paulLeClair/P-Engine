#include "../../../../include/core/p_render/scene/Scene.hpp"

#include "../../../../include/core/p_render/scene/mesh/Mesh.hpp"
#include "../../../../include/core/p_render/scene/patch_list/PatchList.hpp"

#include "../../../../include/core/p_render/backend/resources/Buffer.hpp"
#include "../../../../include/core/p_render/backend/frame_context/FrameContext.hpp"

#include "../../../../include/core/p_render/render_graph/RenderGraph.hpp"

using namespace scene;

scene::Scene::Scene(const SceneCreateInfo &info) {
    // scenes should be created and then the desired renderables will be added by whatever is managing the scene
    threadPool_ = info.threadPool;
    context_ = info.context;
    allocator_ = info.allocator;
    renderGraph_ = info.renderGraph;

    std::string name = renderGraph_->getName() + "SceneUpdateQueue";
    sceneUpdateQueue_ = std::make_shared<JobQueue>(name, *threadPool_);

    // setup scene geometry context
    geometryContext_ = std::make_shared<backend::GeometryContext>();
    VkCommandPoolCreateInfo geomPoolInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    geomPoolInfo.queueFamilyIndex = context_->getGraphicsQueueFamilyIndex();
    
    if (vkCreateCommandPool(context_->getLogicalDevice(), &geomPoolInfo, nullptr, &geometryContext_->copyCommandPool) != VK_SUCCESS) {
        throw std::runtime_error("Unable to create scene geometry copy commandpool!");
    }

}

void scene::Scene::registerMesh(const std::string &name, std::shared_ptr<Mesh> mesh) {
    meshes_.push_back(mesh);
    meshNames_[name] = meshes_.size() - 1;
    meshIDs_[mesh->getID()] = meshes_.size() - 1; // adding this in; maybe we don't really need to store names
        // but i think we can keep them for when it may be convenient
}

void scene::Scene::deleteMesh(const std::string &name) { // should probably override this to allow for ID deletion since it should be safer
    //todo
}

void scene::Scene::registerPatchList(const std::string &name, std::shared_ptr<PatchList> patchList) {
    //todo
}

void scene::Scene::fillSceneBufferSingleThread() {
    // i think now we just fill the scene buffer, subset by subset...
    char *tmp;
    if (vmaMapMemory(allocator_, sceneBufferAllocation_, (void**)&tmp) != VK_SUCCESS) {
        throw std::runtime_error("Unable to map scene buffer memory!");
    }

    // i think for now i'll just write some single threaded logic, although we could
    // probably use the threadpool here like i planned... hmm
    for (auto renderableBufferSubset : sceneBufferSubsets_) {
        auto startByte = renderableBufferSubset.second.startByte;
        auto numBytes = renderableBufferSubset.second.subsetSizeInBytes;

        // separate lambdas for each renderable (for now... will probably reorganize this later)
        const auto &processMesh = [&](std::shared_ptr<Renderable> &renderable) {
            auto mesh = std::dynamic_pointer_cast<Mesh>(renderable);

            // copy data into each submesh's subset of the scene buffer
            for (auto &submesh : mesh->getSubMeshes()) {
                const auto &submeshSubset = sceneBufferSubsets_[submesh->getID()];
                
                // copy vertex data:
                void *vertexStartAddr = (void *)&(tmp[startByte]);
                void *vertexData = (void *)submesh->getVertexData();
                memcpy(vertexStartAddr, vertexData, submesh->getVertexDataSizeInBytes());

                // copy index data:
                void *indexStartAddr = (void *)&(tmp[startByte + submesh->getVertexDataSizeInBytes()]);
                void *indexData = (void *)submesh->getIndexData();
                memcpy(indexStartAddr, indexData, submesh->getIndexDataSizeInBytes());
            }
        };

        const auto &processPatchList = [&](std::shared_ptr<Renderable> &patchList) {

        };

        // any other lambdas...

        // gonna branch based on an awkward check to determine the kind of renderable
        // we're dealing with
        if (meshIDs_.find(renderableBufferSubset.first) != meshIDs_.end()) {
            processMesh(meshes_[meshIDs_[renderableBufferSubset.first]]);
        }
        else if (patchListIDs_.find(renderableBufferSubset.first) != patchListIDs_.end()) {
            processPatchList(patchLists_[patchListIDs_[renderableBufferSubset.first]]);
        }
        else {
            throw std::runtime_error("Unable to find renderable!");
        }
    }
}

void scene::Scene::bake() {

    const auto &computeSceneBufferSizeAndSubsets = [&]() {
        // for now we just have meshes actually implemented in any meaningful way so i'll do that first
        unsigned int sceneBufferSizeInBytes = 0;
        unsigned int byteIndex = 0;

        for (auto &meshRenderable : meshes_) {
            auto mesh = std::dynamic_pointer_cast<Mesh>(meshRenderable);

            unsigned int meshSizeInBytes = 0;
            for (auto &submesh : mesh->getSubMeshes()) {
                auto submeshSizeInBytes = submesh->getSizeInBytes();
                
                SceneBufferSubset submeshSubset = {};
                submeshSubset.startByte = byteIndex;
                submeshSubset.subsetSizeInBytes = submeshSizeInBytes;
                
                sceneBufferSubsets_[submesh->getID()] = submeshSubset;
                
                sceneBufferSizeInBytes += submeshSizeInBytes;
                meshSizeInBytes += submeshSizeInBytes;
                
                byteIndex = sceneBufferSizeInBytes + 1;
            }
            
        }

        for (auto &patchList : patchLists_) {
            // add size of each chunk of patch data to the bufferSize
                // todo

        }

        return sceneBufferSizeInBytes;
    };
    sceneBufferSizeInBytes_ = computeSceneBufferSizeAndSubsets();

    // setup vulkan struct
    VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.flags = 0;
    bufferInfo.pNext = nullptr;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.size = sceneBufferSizeInBytes_;
    // verify these are all the usages we need for the host-side scene buffer
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT; 

    // setup vma allocation
    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY; // the scene buffer is host-visible only
    
    // create the scene buffer 
    if (vmaCreateBuffer(allocator_, &bufferInfo, &allocCreateInfo, &sceneBuffer_, &sceneBufferAllocation_, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("Unable to allocate host-side scene buffer!");
    }

    //TODO: write a multithreaded version of fillSceneBuffer
    fillSceneBufferSingleThread();
    
    // allocate the VRAM buffer that will receive the vertex data
    VkBufferCreateInfo gpuInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    gpuInfo.flags = 0;
    gpuInfo.pNext = nullptr;

    gpuInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    gpuInfo.size = sceneBufferSizeInBytes_;
    // maybe should allow the GPU buffer to be transfer_src, so that we can potentially allow copying back
    // GPU data per-frame, if we have some sort of processing that we want to do and then have access to the results (maybe not useful, idk)
    gpuInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VmaAllocationCreateInfo gpuAllocCreateInfo = {};
    gpuAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if (vmaCreateBuffer(allocator_, &gpuInfo, &gpuAllocCreateInfo, &gpuBuffer_, &gpuBufferAllocation_, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("Unable to create GPU-side scene buffer!");
    }

    // now we fill the host scene buffer initially with all the data we have at bake() time
    context_->immediateSubmitCommand(
        [&](VkCommandBuffer buffer) {
            copyHostSceneBufferToGPU(buffer);
        }
    );
    
}

void scene::Scene::copyHostSceneBufferToGPU(VkCommandBuffer &copyBuffer) {

    // record copy command
    VkBufferCopy copy = {};
    copy.srcOffset = 0;
    copy.dstOffset = 0;
    copy.size = getSceneBufferSizeInBytes();

    vkCmdCopyBuffer(copyBuffer, getHostSceneBuffer(), getGPUSceneBuffer(), 1, &copy);

    // now enqueue a simple pipeline barrier
    VkBufferMemoryBarrier bufferBarrier = {VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
    bufferBarrier.buffer = getGPUSceneBuffer();

    bufferBarrier.offset = 0;
    bufferBarrier.size = getSceneBufferSizeInBytes();

    bufferBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    bufferBarrier.srcQueueFamilyIndex = context_->getGraphicsQueueFamilyIndex();

    // not entirely sure what we need for this mask
    bufferBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_SHADER_READ_BIT;
    bufferBarrier.dstQueueFamilyIndex = context_->getGraphicsQueueFamilyIndex();

    vkCmdPipelineBarrier(copyBuffer, 
                        VK_PIPELINE_STAGE_TRANSFER_BIT, 
                        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
                        0,
                        0,
                        nullptr,
                        1,
                        &bufferBarrier,
                        0,
                        nullptr);

}

std::shared_ptr<Renderable> scene::Scene::findRenderableByUID(UniqueID::uid_t uid) {
    std::shared_ptr<Renderable> renderable = nullptr;
            
    if (meshIDs_.count(uid)) {
        renderable = meshes_[meshIDs_[uid]];
    }
    if (patchListIDs_.count(uid)) {
        renderable = patchLists_[patchListIDs_[uid]];
    }

    return renderable;
}

void scene::Scene::copyUpdatesToHostSceneBuffer(const std::vector<std::pair<std::shared_ptr<SceneBufferSubset>, UniqueID::uid_t>> &updates) {
    // here we copy only the data from the renderables' whose subsets are included in the argument vector
    for (auto &pair : updates) {
        // get renderable
        auto renderable = findRenderableByUID(pair.second);

        if (renderable->isMesh()) { // i guess mesh updates will also have to signal you wanna update all submeshes..
            auto mesh = std::dynamic_pointer_cast<Mesh>(renderable);

            // copy mesh data into scene buffer subset
            char *tmp;
            if (vmaMapMemory(allocator_, sceneBufferAllocation_, (void**)&tmp) != VK_SUCCESS) {
                // error!
            }

            // copy each submesh over
            unsigned int startByte = pair.first->startByte;
            for (auto &submesh : mesh->getSubMeshes()) {
                const auto &submeshSubset = sceneBufferSubsets_[submesh->getID()];
                
                // copy vertex data:
                void *vertexStartAddr = (void *)&(tmp[startByte]);
                void *vertexData = (void *)submesh->getVertexData();
                memcpy(vertexStartAddr, vertexData, submesh->getVertexDataSizeInBytes());

                // copy index data:
                void *indexStartAddr = (void *)&(tmp[startByte + submesh->getVertexDataSizeInBytes()]);
                void *indexData = (void *)submesh->getIndexData();
                memcpy(indexStartAddr, indexData, submesh->getIndexDataSizeInBytes());
            }

            vmaUnmapMemory(allocator_, sceneBufferAllocation_);
        }
        else if (renderable->isSubMesh()) {
            // in this case we only copy the submesh subset in
            auto submesh = std::dynamic_pointer_cast<SubMesh>(renderable);

            // copy mesh data into scene buffer subset
            char *tmp;
            if (vmaMapMemory(allocator_, sceneBufferAllocation_, (void**)&tmp) != VK_SUCCESS) {
                // error!
            }

            unsigned int startByte = pair.first->startByte;

            // copy vertex data:
            void *vertexStartAddr = (void *)&(tmp[startByte]);
            void *vertexData = (void *)submesh->getVertexData();
            memcpy(vertexStartAddr, vertexData, submesh->getVertexDataSizeInBytes());

            // copy index data:
            void *indexStartAddr = (void *)&(tmp[startByte + submesh->getVertexDataSizeInBytes()]);
            void *indexData = (void *)submesh->getIndexData();
            memcpy(indexStartAddr, indexData, submesh->getIndexDataSizeInBytes());

            vmaUnmapMemory(allocator_, sceneBufferAllocation_);
        }
        else if (renderable->isPatchList()) {
            // todo
        }
        else {
            // error!
        }
    }
}

void scene::Scene::update(backend::FrameContext &frame) {
    // reset the command pool to deallocate any previously-allocated copy buffers
    if (vkResetCommandPool(context_->getLogicalDevice(), geometryContext_->copyCommandPool, 0) != VK_SUCCESS) {
        throw std::runtime_error("Unable to reset geometry copy command pool!");
    }

    // first fill update queue...
    std::vector<std::future<bool>> updateFutures = {};
    std::map<unsigned int, UniqueID::uid_t> futureRenderableIDs = {};
    const auto fillUpdateQueue = [&]() {
        for (auto &meshRenderable : meshes_) {
            auto mesh = std::dynamic_pointer_cast<Mesh>(meshRenderable);

            for (auto &submesh : mesh->getSubMeshes()) {
                // schedule update job; some thread will actually execute the update 
                updateFutures.push_back(sceneUpdateQueue_->scheduleTask(
                    [&]() {
                        return submesh->update();
                    }
                ));
                // store index of renderable, according to its uid
                futureRenderableIDs[updateFutures.size() - 1] = submesh->getID();
            }
        }

        for (auto &patchList : patchLists_) {

        }
    };
    fillUpdateQueue();

    // submit the ThreadPool::JobQueue to the worker threads
    sceneUpdateQueue_->submit();
    assert(sceneUpdateQueue_->isEmpty());

    std::vector<std::shared_ptr<SceneBufferSubset>> bufferSubsetsToUpdate = {};
    std::map<unsigned int, UniqueID::uid_t> bufferSubsetUIDs = {};
    for (int futureIndex = 0; futureIndex < updateFutures.size(); futureIndex++) {
        auto &future = updateFutures[futureIndex];
        if (future.get()) {
            // future returned true, so we need to add its subset to the set of scene buffer subsets we're updating
            bufferSubsetsToUpdate.push_back(
                std::make_shared<SceneBufferSubset>(sceneBufferSubsets_[futureRenderableIDs[futureIndex]])
            );
            bufferSubsetUIDs[futureIndex] = bufferSubsetsToUpdate.size() - 1;
        }
    }

    // at this point, each renderable will have updated its data according to the callback it was given
    
    // build up updates list
    std::vector<std::pair<std::shared_ptr<SceneBufferSubset>, UniqueID::uid_t>> updates = {};
    for (int i = 0; i < bufferSubsetsToUpdate.size(); i++) {
        updates.push_back(std::make_pair(bufferSubsetsToUpdate[i], bufferSubsetUIDs[i]));
    }
    // when this function returns, we should end up with a host buffer that is up-to-date
    copyUpdatesToHostSceneBuffer(updates);

    // now we can go through and enqueue some copy commands + pipeline barrier 
    VkCommandBuffer copyBuffer;

    VkCommandBufferAllocateInfo cmdAllocInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    cmdAllocInfo.commandBufferCount = 1;
    cmdAllocInfo.commandPool = geometryContext_->copyCommandPool;
    cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    if (vkAllocateCommandBuffers(context_->getLogicalDevice(), &cmdAllocInfo, &copyBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Unable to allocate geometry copy command buffer!");
    }

    copyHostSceneBufferToGPU(copyBuffer); // this will fill the command buffer with copies + a pipeline barrier on the vertex buffer

    // enqueue the copy buffer!
    frame.enqueueCommandBuffer(copyBuffer);
}