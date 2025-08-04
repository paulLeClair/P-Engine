//
// Created by paull on 2024-09-14.
//

#pragma once

#include <chrono>
#include <utility>

#include "../../../Scene/Scene.hpp"
#include "../VulkanBufferSuballocator/VulkanBufferSuballocator.hpp"
#include "../VulkanBufferSuballocator/VulkanBufferSuballocation/VulkanBufferSuballocationHandle.hpp"

// TODO -> decouple these
#include <vector>


namespace pEngine::girEngine::backend::vulkan {
    /**
     * This will probably won't survive long past the single-animated-model demo; I'll likely do a big pass to
     * restructure geometry management and that will probably include getting rid of rigid stuff like this.
     */
    struct GeometryGIRAttachment {
        gir::BufferIR vertexBuffer;
        gir::BufferIR indexBuffer;
        unsigned vertexCount;
        unsigned indexCount;
    };

    class VulkanModel {
    public:
        struct CreationInput {
            std::string name;

            UniqueIdentifier uid;

            VmaAllocator allocator;

            std::vector<VulkanBufferSuballocationHandle> suballocatedUniformBuffers = {};

            // TEMPORARY MEASURE: while models manage their own uniform buffers internally, we'll pass in the alignment
            uint32_t uniformBufferMinAlignment;

            std::vector<GeometryGIRAttachment> &geometryBindings;

            std::vector<unsigned> queueFamilyIndices = {};

            float defragmentationPercentage = 0.3f;
        };

        explicit VulkanModel(const CreationInput &input)
            : name(input.name),
              uid(input.uid),
              suballocatedUniformBuffers(input.suballocatedUniformBuffers),
              vertexBufferSuballocator(
                  VulkanBufferSuballocator(VulkanBufferSuballocator::CreationInput{
                      input.allocator,
                      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                      obtainVertexBufferGirs(
                          input.geometryBindings),
                      input.queueFamilyIndices,
                      input.uniformBufferMinAlignment,
                      input.defragmentationPercentage
                  })),
              indexBufferSuballocator(VulkanBufferSuballocator(VulkanBufferSuballocator::CreationInput{
                  input.allocator,
                  VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                  obtainIndexBufferGirs(
                      input.geometryBindings),
                  input.queueFamilyIndices,
                  input.uniformBufferMinAlignment,
                  input.defragmentationPercentage
              })),
              geometryBindings(input.geometryBindings) {
            std::vector<gir::BufferIR> vertexBuffersToSuballocate = {};
            std::vector<gir::BufferIR> indexBuffersToSuballocate = {};
            for (auto &drawAttachment: input.geometryBindings) {
                vertexBuffersToSuballocate.push_back(drawAttachment.vertexBuffer);
                indexBuffersToSuballocate.push_back(drawAttachment.indexBuffer);
            }
            vertexBufferSuballocator.suballocateBuffers(vertexBuffersToSuballocate);
            indexBufferSuballocator.suballocateBuffers(indexBuffersToSuballocate);

            for (auto &drawAttachment: input.geometryBindings) {
                auto &vertexBuffer = drawAttachment.vertexBuffer;
                auto &indexBuffer = drawAttachment.indexBuffer;

                auto handle = vertexBufferSuballocator.findSuballocation(vertexBuffer.uid);
                if (!handle.has_value()) {
                    // TODO -> log!
                    continue;
                }
                vertexBufferSuballocations.emplace_back(handle.get(), drawAttachment.vertexCount);

                handle = indexBufferSuballocator.findSuballocation(indexBuffer.uid);
                if (!handle.has_value()) {
                    // TODO -> log!
                    continue;
                }
                indexBufferSuballocations.emplace_back(handle.get(), drawAttachment.indexCount);

                numberOfVertices += drawAttachment.vertexCount;
            }
        }

        [[nodiscard]] const std::string &getName() const {
            return name;
        }

        [[nodiscard]] const UniqueIdentifier &getUid() const {
            return uid;
        }

        [[nodiscard]] const VulkanBufferSuballocator &getVertexBufferSuballocator() const {
            return vertexBufferSuballocator;
        }

        [[nodiscard]] const VulkanBufferSuballocator &getIndexBufferSuballocator() const {
            return indexBufferSuballocator;
        }

        const std::vector<std::pair<VulkanBufferSuballocationHandle, uint32_t> > &getVertexBufferSuballocations() {
            return vertexBufferSuballocations;
        }


        [[nodiscard]] const std::vector<std::pair<VulkanBufferSuballocationHandle, uint32_t> > &
        getIndexBufferSuballocations() {
            return indexBufferSuballocations;
        }

        [[nodiscard]] uint32_t getNumberOfVertices() const {
            return numberOfVertices;
        }

        [[nodiscard]] const std::vector<VulkanBufferSuballocationHandle> &getSuballocatedUniformBuffers() const {
            return suballocatedUniformBuffers;
        }

        [[nodiscard]] const std::vector<GeometryGIRAttachment> &getGeometryBindings() const {
            return geometryBindings;
        }

    private:
        std::string name;

        UniqueIdentifier uid;

        // for now, we're only supporting plain uniform buffers for models (but other types can be added similarly)
        std::vector<VulkanBufferSuballocationHandle> suballocatedUniformBuffers = {};

        VulkanBufferSuballocator vertexBufferSuballocator;
        VulkanBufferSuballocator indexBufferSuballocator;

        std::vector<std::pair<VulkanBufferSuballocationHandle, uint32_t> > vertexBufferSuballocations;
        std::vector<std::pair<VulkanBufferSuballocationHandle, uint32_t> > indexBufferSuballocations;

        // temporary (static) geometry bindings
        std::vector<GeometryGIRAttachment> geometryBindings; //TODO -> make this singular (not a vector)
        uint32_t numberOfVertices = 0;

        [[nodiscard]] static std::vector<gir::BufferIR>
        obtainVertexBufferGirs(const std::vector<GeometryGIRAttachment> &geometryBindings) {
            std::vector<gir::BufferIR> buffers(geometryBindings.size());
            unsigned attachmentIndex = 0;
            for (auto &geometryBinding: geometryBindings) {
                buffers[attachmentIndex] = geometryBinding.vertexBuffer;
                attachmentIndex++;
            }
            return buffers;
        }

        [[nodiscard]] static std::vector<gir::BufferIR>
        obtainIndexBufferGirs(const std::vector<GeometryGIRAttachment> &geometryBindings) {
            std::vector<gir::BufferIR> buffers(geometryBindings.size());
            unsigned attachmentIndex = 0;
            for (auto &geometryBinding: geometryBindings) {
                buffers[attachmentIndex] = geometryBinding.indexBuffer;
                attachmentIndex++;
            }
            return buffers;
        }
    };
}
