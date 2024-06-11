//
// Created by paull on 2022-06-25.
//

#pragma once

#include <utility>
#include <vector>

#include "../../../../../lib/vk_mem_alloc/vk_mem_alloc.h"
#include "../VulkanRenderable.hpp"

namespace pEngine::girEngine::backend::vulkan {
    class VulkanIndexedMesh : public VulkanRenderable {
    public:
        struct CreationInput {

        };

        explicit VulkanIndexedMesh(const CreationInput &creationInput) {


        }

        ~VulkanIndexedMesh() = default;

        bool isVulkanIndexedMesh() override {
            return true;
        }

    private:

    };

}
