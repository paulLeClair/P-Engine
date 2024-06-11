//
// Created by paull on 2022-07-09.
//

#pragma once

#include "../../../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"

namespace pEngine::girEngine::backend::vulkan::pipeline {

    /**
     * Stub/placeholder for now - might expand this into a VulkanPipelineManager (or make this a component of a class like
     * that)
     *
     */
    class VulkanPipeline {
    public:
        struct CreationInput {
            std::string name;
            util::UniqueIdentifier uniqueIdentifier;
            // TODO - anything else that is required for
        };

        explicit VulkanPipeline(const CreationInput &creationInput)
                : name(creationInput.name),
                  uniqueIdentifier(creationInput.uniqueIdentifier) {

        }

        ~VulkanPipeline() = default;

        [[nodiscard]] const std::string &getName() const {
            return name;
        }

        [[nodiscard]] const util::UniqueIdentifier &getUniqueIdentifier() const {
            return uniqueIdentifier;
        }

    private:
        std::string name;
        util::UniqueIdentifier uniqueIdentifier;
    };

}// namespace PGraphics
