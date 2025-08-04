//
// Created by paull on 2025-06-12.
//

#pragma once

#include <filesystem>
#include <vector>
#include <glm/glm.hpp>
#include <assimp/scene.h>

#include "AssimpData.hpp"
#include "SkeletalAnimation.hpp"

#define MAX_BONES 255

namespace pEngine::util::assimp {
    struct AnimatedModel {
        std::string name;
        UniqueIdentifier uid;

        std::vector<Material> materials; // textures are todo
        std::vector<Mesh> meshes;

        SkeletalAnimation animation;
    };

    class SceneImporter {
    public:
        SceneImporter() = default;

        ~SceneImporter() = default;

        SceneImporter(const SceneImporter &other) = default;

        SceneImporter(SceneImporter &&other) noexcept
            : importSuccessful(other.importSuccessful),
              rootTransform(other.rootTransform),
              animatedModels(std::move(other.animatedModels)) {
        }

        SceneImporter &operator=(const SceneImporter &other) = default;


        void extractMeshData(const aiScene *assimpScene, SkeletalAnimation &modelAnim, std::vector<Mesh> &meshes,
                             aiNode *meshNode) const;

        [[nodiscard]] bool import(const aiScene *aiScene);

        /**
         * NOTE: this current basic implementation assumes a single model per assimp scene...
         * not sure what would happen if you tried to use a multimodel scene but it would not be good
         * @return All models read from the import file - see above
         */
        [[nodiscard]] std::vector<AnimatedModel> &getImportedModels() {
            return animatedModels;
        }

        [[nodiscard]] aiAnimation *getAnimation() const {
            return anim;
        }

    private:
        bool importSuccessful = false;

        glm::mat4 rootTransform = glm::mat4(1.0f);

        aiAnimation *anim = nullptr;

        std::vector<AnimatedModel> animatedModels;

        /**
        * Checks whether a given node or any of its children contain meshes (if it has any)
        * @param node
        * @return
        */
        static bool nodeTreeContainsNoMeshes(const aiNode *node) {
            if (node->mNumMeshes) {
                return false;
            }
            if (!node->mNumChildren) {
                return true;
            }

            for (int i = 0; i < node->mNumChildren; i++) {
                if (!nodeTreeContainsNoMeshes(node->mChildren[i])) {
                    return false;
                }
            }
            return true;
        }
    };
}
