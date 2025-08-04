//
// Created by paull on 2024-06-29.
//

#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <glm/ext/matrix_common.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_double4x4.hpp>
#include <glm/ext/vector_common.hpp>

#include "../Animation/Skeleton/Bone/Bone.hpp"

namespace pEngine::girEngine::scene::model {

    /**
     * New idea: attach external scene resources to individual segments,
     * but generally these will just be structured as a very naive tree
     * (which will probably be completely gutted and replaced with something smarter).
     * Doing research for assimp integration has made me think that these should logically correspond to a "mesh",
     * meaning it's some data that is unified mainly by the fact that it uses the same material (ie image texture info)
     *
     * As time goes on this system will have to be able to accommodate fancier materials and what not but for now
     * I'll be trying to support strictly whatever is required to get this basic goofy capoeira FBX model & animation
     * running.
     *
     * For the assimp integration, we'll migrate some of this stuff into the bone and skeleton classes,
     * and have model segments be specifically tied to
     */
    class ModelSegment {
    public:
        struct CreationInput {
            std::string name;
            util::UniqueIdentifier uniqueIdentifier;

            std::shared_ptr<Buffer> vertexBuffer;
            std::shared_ptr<Buffer> indexBuffer;

        };

        explicit ModelSegment(const CreationInput &input) : name(input.name), uniqueIdentifier(input.uniqueIdentifier),
                                                            vertexBuffer(input.vertexBuffer),
                                                            indexBuffer(input.indexBuffer) {

        }

        ModelSegment(const ModelSegment &other) = default;

        ModelSegment &operator=(const ModelSegment &other) = default;

    private:
        std::string name;
        util::UniqueIdentifier uniqueIdentifier;

        std::shared_ptr<Buffer> vertexBuffer;
        std::shared_ptr<Buffer> indexBuffer;


    };

}
