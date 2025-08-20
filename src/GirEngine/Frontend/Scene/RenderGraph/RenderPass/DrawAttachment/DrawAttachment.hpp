//
// Created by paull on 2023-10-02.
//

#pragma once

#include <utility>
#include <vector>
#include "../../../SceneResources/Model/Model.hpp"

namespace pEngine::girEngine::scene::graph::renderPass {

    using namespace scene::model;

    /**
     * This has now been repurposed so that it will be the first (likely of many)
     * attachment type where you can bind some draw data to a particular binding slot
     * of some render pass. These should not exist outside of a render pass.
     *
     *
     */
    struct DrawAttachment {
        /**
         * This controls the index of the geometry binding slot that the attached draw data
         * will be bound to.
         */
        unsigned targetGeometryBindingIndex = 0;

        // for the single-animated-model demo the only possibility for draw attachments will be
        // scene::Models but this will change when I switch over to fleshing out the scene functionality
        std::vector<Model> models = {};


    };

} // scene
