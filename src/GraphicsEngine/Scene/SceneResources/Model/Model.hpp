#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include <filesystem>

#include "../../../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"

#include "../SceneResource.hpp"
#include "../Material/Material.hpp"
#include "../Buffer/Buffer.hpp"
#include "ModelSegment/ModelSegment.hpp"
#include "Animation/Animation.hpp"
#include "../../../../utilities/assimp/SceneImporter.hpp"

#include "../../../GraphicsIR/model/ModelIR/ModelIR.hpp"
#include "../../SceneSpace/Position/Position.hpp"
#include "../../SceneSpace/Velocity/Velocity.hpp"
#include "../../SceneSpace/Orientation/Orientation.hpp"
#include "../GeometryBinding/GeometryBinding.hpp"

// ugly macros for now
#define MAX_BONES_PER_MODEL 255
#define MAX_BONES_PER_VERTEX 4

// BIG TODO -> optimize a lot of this assimp stuff; cache and reuse things wherever possible etc

using namespace pEngine::girEngine::scene;

namespace pEngine::girEngine::scene::model {
    /**
     * This class will likely need redesigning soon - I'm not entirely sure about certain things here.
     *
     * I think the main point is that Models should store Renderables (which have to maintain geometry data)
     * or it can be handed vertex & index buffers directly.
     *
     * In the future when we're not just dealing with basic meshes it may make sense to make a few other similar classes
     *
     * COMING BACK TO THIS AGAIN: an interesting idea that assimp gives me is to
     * break up the high-level vertex data into channels (like the aiMesh does)
     * so that we can just pitch all the attributes into their own bins,
     * and this way the high-level scene doesn't have to concern itself at all about
     * how the data will be combined into a vertex buffer. (However I'll still want to support that)
     *
     * Not sure entirely how much benefit that gives me off the bat; but it could
     * allow me to hardcode a particular way for vertex buffers directly from
     * high-level data (because generally we're only supporting a commonly-used collection of attribs)
     * However, that would require me to enforce that the user's vertex shader inputs are all following that
     * harcoded convention (not ideal)
     */
    class Model {
    public:
        struct ModelMatrixBufferData {
            glm::mat4 modelMatrix;
            glm::mat4 normalsMatrix;
        };

        struct AnimationBonesUniformBufferData {
            glm::mat4 animationBones[MAX_BONES_PER_MODEL];
        };

        Model() = default;

        /**
         * This non-assimp route for creating models (along with its constructor) is TODO
         */
        struct CreationInput {
            std::string name;
            UniqueIdentifier uid;

            std::vector<Material> materials;
            std::vector<Texture> textures;
            std::vector<Buffer> buffers;
            std::vector<Image> images;
        };

        /**
          * OKAY - now that we're trying to actually read in models and animations using assimp
          * I might (temporarily) disable the ability to pre-specify your model segment;
          * Instead it will take a simple filename to use Assimp importer on, and then
          * I guess I'll try and just build the model segments dynamically from what assimp gives us.
          *
          * This stuff will be useful for providing the data through the other engine abstractions
          */
        struct AssimpCreationInput : Resource::CreationInput {
            // TODO -> evaluate whether we even need these for the assimp case;
            // it's probably better to guarantee that all the data comes exclusively from assimp here
            std::vector<Material> materials;
            std::vector<Texture> textures;
            std::vector<Buffer> buffers;
            std::vector<Image> images;

            /**
             * Note: this will dictate what attributes are obtained from the assimp model!
             * Position must be included.
             */
            std::vector<geometry::GeometryBinding> geometryBindings = {};

            /**
             * NEW: assimp importer object has to have its lifetime managed externally so we actually import the
             * structure and then pass it in for processing.
             */
            const aiScene *modelScene;

            // TODO - support other file types; they all should get processed into the same assimp data structures tho

            // TODO -> rethink/overhaul the spatial/physics engine integration stuff (this seems ok for now)
            space::position::Position position;
            space::velocity::Velocity velocity;
            space::orient::Orientation orientation;
            // TODO -> add scaling vector

            // TODO -> these scale factors might not be strictly necessary for dealing w/ vulkan coordinate system;
            // ensure that we're not "double compensating" for it anywhere
            glm::vec4 coordinateScalingFactors
                    = {1, -1, -1, 1}; // DEFAULT for Vulkan

            bool tempOldCodeEnableFlag = false;
        };

        explicit Model(const CreationInput &creationInput) : name(creationInput.name),
                                                             uniqueIdentifier(creationInput.uid),
                                                             materials(creationInput.materials),
                                                             textures(creationInput.textures),
                                                             buffers(creationInput.buffers),
                                                             images(creationInput.images),
                                                             animations({}) {
        }


        /**
         * ASSIMP CONSTRUCTOR OVERLOAD -> uses a different set of creation inputs to produce a model from an assimp
         * -supported file that is assumed to contain a single model (which may consist of multiple meshes, materials,
         * animations, etc)
         * @param assimpCreationInput
         */
        explicit Model(const AssimpCreationInput &assimpCreationInput)
            : name(assimpCreationInput.name),
              uniqueIdentifier(assimpCreationInput.uid),
              position(assimpCreationInput.position),
              velocity(assimpCreationInput.velocity),
              orientation(assimpCreationInput.orientation),
              materials(assimpCreationInput.materials),
              textures(assimpCreationInput.textures),
              buffers(assimpCreationInput.buffers),
              images(assimpCreationInput.images) {
            // FINAL STRETCH -> it's now time to come back to fix the various issues that have cropped up here.
            // 1. make it work without concatenating models
            // 2. generally clean up and maybe port out the assimp model stuff to a particular AssimpModel class that
            // can be in our utils and can be made backend-agnostic

            // NEW PROCESS
            // 1. obtain "agnostified" assimp data from our new util class
            if (!importer.import(assimpCreationInput.modelScene)) {
                // TODO -> log!
                return;
            }

            for (auto &model: importer.getImportedModels()) {
                /**
                 * Hmmm... we'll need to probably adjust how the models are storing things (and then how they get baked to gir)
                 * because we want to fix the broken multi-mesh functionality that was here before.
                 *
                 * Pretty simple though, you just group up a bunch of meshes and materials with an animation for the
                 * most basic implementation.
                 */

                // BASIC SANITY CHECK TO ENFORCE TEMP SINGLE-GEOMETRY-BINDING CONSTRAINT
                assimpCreationInput.geometryBindings.size() != 1
                    ? throw std::runtime_error("Invalid geometry bindings!")
                    : 0;

                for (auto &geometryBinding: assimpCreationInput.geometryBindings) {
                    uint32_t vertexStride = geometryBinding.vertexBinding.vertexStride;

                    // we'll want to obtain a byte array for copying our data into FOR EACH MESH!
                    // the paradigm in use is that all models will use meshes with the same vertex attribute configs;

                    // NOTE: for now, we're hardcoding all vertex and index buffers for a model to be combined;
                    // this means we need to ensure that for each mesh we're offsetting our indices since the verts
                    // will be all in one big buffer.
                    for (auto &mesh: model.meshes) {
                        std::vector<uint8_t> vertexRawData = {};
                        std::vector<uint8_t> indexRawData = {}; //assuming 32-bit indices

                        vertexRawData.assign(vertexStride * mesh.vertexCount, {});
                        indexRawData.assign(mesh.indexCount * sizeof(uint32_t), {}); //assuming 32-bit indices

                        for (uint32_t v = 0; v < mesh.vertexCount; v++) {
                            uint32_t currentOffset = 0;
                            for (auto &vertexAttribute: geometryBinding.vertexBinding.attributes) {
                                // NOTE: vertex data is always going to be packaged up in a particular hardcoded order (shaders must match for now)
                                switch (vertexAttribute.usageLabel.get_value_or(geometry::AttributeUsage::UNDEFINED)) {
                                    case geometry::AttributeUsage::POSITION: {
                                        auto result = glm::vec4(mesh.positions[v].x,
                                                                mesh.positions[v].y,
                                                                mesh.positions[v].z,
                                                                1.0);
                                        std::memcpy(&vertexRawData[v * vertexStride + currentOffset],
                                                    &result, sizeof(glm::vec4));
                                    }
                                    break;
                                    case geometry::AttributeUsage::NORMAL: {
                                        auto result = glm::vec4(mesh.normals[v].x,
                                                                mesh.normals[v].y,
                                                                mesh.normals[v].z,
                                                                0.0);
                                        std::memcpy(&vertexRawData[v * vertexStride + currentOffset], &result,
                                                    sizeof(glm::vec4));
                                    }
                                    break;
                                    case geometry::AttributeUsage::ANIMATION_BONE_INDICES: {
                                        // for these we have to first pack them and then copy
                                        if (mesh.animationBoneIndices[v].size() > MAX_BONES_PER_VERTEX) {
                                            // TODO -> log warning!
                                        }
                                        glm::uvec4 result = {0, 0, 0, 0};
                                        for (int i = 0; i < std::min(
                                                            static_cast<int32_t>(
                                                                mesh.animationBoneIndices[v].size()),
                                                            MAX_BONES_PER_VERTEX); i++) {
                                            result[i] = mesh.animationBoneIndices[v][i];
                                        }
                                        std::memcpy(&vertexRawData[v * vertexStride + currentOffset], &result,
                                                    sizeof(glm::uvec4));
                                    }
                                    break;
                                    case geometry::AttributeUsage::ANIMATION_BONE_WEIGHTS: {
                                        // for these we have to first pack them and then copy
                                        if (mesh.animationWeights[v].size() > MAX_BONES_PER_VERTEX) {
                                            // TODO -> log warning!
                                        }
                                        glm::vec4 result = {0, 0, 0, 0};
                                        // for these we have to first pack them and then copy
                                        for (int i = 0; i < std::min(
                                                            static_cast<int32_t>(
                                                                mesh.animationWeights[v].size()),
                                                            MAX_BONES_PER_VERTEX); i++) {
                                            result[i] = mesh.animationWeights[v][i];
                                        }
                                        std::memcpy(&vertexRawData[v * vertexStride + currentOffset], &result,
                                                    sizeof(glm::vec4));
                                    }
                                    break;
                                    case geometry::AttributeUsage::UV:
                                    case geometry::AttributeUsage::TANGENT:
                                    case geometry::AttributeUsage::BITANGENT:
                                    case geometry::AttributeUsage::COLOR:
                                    default: {
                                        // TODO -> log!
                                        std::cout << "Unsupported attribute!" << std::endl;
                                        continue;
                                    }
                                }
                                currentOffset += vertexAttribute.size;
                            }
                        }

                        for (uint32_t index = 0; index < mesh.indexCount; index++) {
                            uint32_t result = mesh.indices[index];
                            // + indicesOffset;
                            std::memcpy(&indexRawData[index * sizeof(uint32_t)], &result,
                                        sizeof(uint32_t));
                        }

                        auto vertexRawDataContainer = ByteArray(ByteArray::CreationInput{
                            .name = mesh.name + " vertex raw data container",
                            // TODO -> evaluate whether we want to give this its own uid...
                            .uniqueIdentifier = mesh.uid,
                            .rawData = vertexRawData.data(),
                            .rawDataSizeInBytes = vertexRawData.size(),
                        });

                        auto indexRawDataContainer = ByteArray(ByteArray::CreationInput{
                            .name = mesh.name + " index raw data container",
                            .uniqueIdentifier = mesh.uid,
                            .rawData = indexRawData.data(),
                            .rawDataSizeInBytes = indexRawData.size(),
                        });

                        boundGeometries.emplace_back(
                            geometryBinding.bindingIndex,
                            vertexRawDataContainer,
                            indexRawDataContainer,
                            mesh.vertexCount,
                            mesh.indexCount
                        );
                    }
                }
            }
        }

        ~Model() = default;

        [[nodiscard]] const std::vector<Material> &getMaterials() const {
            return materials;
        }

        [[nodiscard]] const std::vector<Texture> &getTextures() const {
            return textures;
        }

        // TODO - evaluate not splitting up buffers by usage, like here (and rely on buffers storing their own usage)
        [[nodiscard]] const std::vector<Buffer> &getBuffers() const {
            return buffers;
        }

        [[nodiscard]] const std::vector<Image> &getImages() const {
            return images;
        }

        [[nodiscard]] assimp::SkeletalAnimation &getAnimation() {
            if (importer.getImportedModels().empty()) {
                // TODO -> log instead!
                throw std::runtime_error("No animation found!"); //temp
            }
            if (importer.getImportedModels().size() > 1) {
                // TODO -> log a warning that there are multiple models which is unsupported atow
            }
            return importer.getImportedModels().back().animation;
        }

        [[nodiscard]] glm::mat4 getModelMatrix() const {
            // DEBUG -> now that we're wiring in the position of the model, it seems like there is potentially
            // a mismatch in terms of whether each sub-matrix that we're combining is row-major or column-major;
            // stuff like this might eventually warrant a wrapper/layer around GLM and the math stuff it uses to
            // ensure that we have the ability to inject a bunch of sanity checking and all that; generally I like the
            // idea of using layers to add to the engine architecture, with well-defined boundaries and responsibilities
            // as well as the ability to swap out the underlying functionality without any other layer needing to worry much
            const glm::mat4 translation = translate(glm::mat4(1.0f), position.position);
            // ASSUMPTION: this is column-major

            const glm::mat4 rotation = orientation.getOrientationMatrix(); // ASSUMPTION: this is column-major
            const glm::quat rotationQuat = quat_cast(rotation);
            const auto pitch = glm::pitch(rotationQuat);
            const auto yaw = glm::yaw(rotationQuat);

            auto result = rotate(translation, pitch, glm::vec3(0.0f, 1.0f, 0.0f));
            result = rotate(result, yaw, glm::vec3(0.0f, 0.0f, 1.0f));

            return result;
        }

        /**
         * Obviously this works completely differently now; we'll have to adjust
         * the bake process because each Model will probably need to be individually tracked a bit more closely.
         *
         */
        [[nodiscard]] gir::model::ModelIR bakeModel() {
            // NEW: the native animation mechanism is currently disabled for simplicity's sake; this can be
            // redone in a future chunk and I can go through and make an editor GUI and all that stuff to allow for
            // in-engine animation tweaking and all that jazz


            // COMING BACK TO THIS POST-MODEL STUFF:
            // here we need to ensure a couple things, now that animations are being fully-decoupled from model internals:
            // 1. for every mesh (which in assimp is just geometry tied to a  material & some bones), we need to
            // make sure we're passing a mesh attachment for each assimp mesh (and respect that in backend bake too)
            // 2.
            unsigned boundGeometryIndex = 0;
            std::vector<gir::DrawAttachmentIR> drawAttachmentGirs = {};
            // idea here: we want to build the vertex and index buffers from the BoundGeometry
            for (auto &boundGeometry: boundGeometries) {
                std::vector<gir::MeshAttachment> meshAttachments = {};
                meshAttachments.emplace_back();
                auto &meshAttachment = meshAttachments.back();
                meshAttachment.vertexBuffers.emplace_back(
                    gir::BufferIR(
                        name + " Bound Geometry Vertex Buffer " + std::to_string(boundGeometryIndex),
                        util::UniqueIdentifier(),
                        gir::GIRSubtype::BUFFER,
                        gir::BufferIR::BufferUsage::VERTEX_BUFFER,
                        boundGeometry.targetGeometryBindingIndex,
                        (const unsigned char *) boundGeometry.vertexData.getRawDataPointer(),
                        boundGeometry.vertexData.getRawDataSizeInBytes()
                    ),
                    boundGeometry.vertexCount
                );
                meshAttachment.indexBuffers.emplace_back(
                    gir::BufferIR(
                        name + " Bound Geometry Index Buffer " + std::to_string(boundGeometryIndex),
                        util::UniqueIdentifier(),
                        gir::GIRSubtype::BUFFER,
                        gir::BufferIR::BufferUsage::INDEX_BUFFER,
                        boundGeometry.targetGeometryBindingIndex,
                        (const unsigned char *) boundGeometry.indexData.getRawDataPointer(),
                        boundGeometry.indexData.getRawDataSizeInBytes()
                    ),
                    boundGeometry.indexCount
                );

                drawAttachmentGirs.emplace_back(
                    gir::DrawAttachmentIR::CreationInput{
                        name + " Draw Attachment Gir " + std::to_string(boundGeometryIndex),
                        UniqueIdentifier(),
                        gir::GIRSubtype::DRAW_COMMANDS,
                        boundGeometry.targetGeometryBindingIndex,
                        meshAttachments
                    }
                );

                boundGeometryIndex++;
            }

            std::vector<gir::BufferIR> bufferGirs = {};
            int bufferIndex = 0;
            for (auto &buffer: buffers) {
                // TODO -> support other buffer types; for now uniforms are okay (others are not much different anyway)
                if (buffer.getBufferSubtype() == Buffer::BufferSubtype::UNIFORM_BUFFER) {
                    bufferGirs.emplace_back(
                        buffer.name,
                        buffer.uid,
                        gir::GIRSubtype::BUFFER,
                        gir::BufferIR::BufferUsage::UNIFORM_BUFFER,
                        buffer.getBindingIndex(),
                        buffer.getRawDataContainer().getRawDataByteArray(),
                        static_cast<uint32_t>(buffer.getRawDataContainer().getRawDataSizeInBytes()),
                        buffer.maxBufferSize // TODO -> hook in the descriptor set index optional here
                        // buffer.getDescriptorSetIndex()
                    );
                    bufferIndex++;
                }
            }

            return gir::model::ModelIR(gir::model::ModelIR::CreationInput{
                name,
                uniqueIdentifier,
                bufferGirs,
                drawAttachmentGirs,
                position,
                orientation,
            });
        }

        [[nodiscard]] const std::vector<geometry::BoundGeometry> &getBoundGeometries() const {
            return boundGeometries;
        }

        /**
         * Potentially non-useful function that just concatenates all bound geometries into a single
         * array; note that they have to use the same vertex input config
         */
        void concatenateGeometry() {
            auto tmp = boundGeometries;
            boundGeometries.clear();

            geometry::BoundGeometry concatenatedGeometry = {};
            uint32_t boundGeometryIndex = 0;
            for (auto &boundGeometry: tmp) {
                concatenatedGeometry.indexCount += boundGeometry.indexCount;
                concatenatedGeometry.vertexCount += boundGeometry.vertexCount;

                auto indices = boundGeometry.indexData.getRawDataAsVector<uint32_t>();
                if (boundGeometryIndex) {
                    for (auto &index: indices) {
                        index += boundGeometry.vertexCount;
                    }
                }

                // obtain raw data and concatenate
                concatenatedGeometry.vertexData.appendRawData(boundGeometry.vertexData.getRawDataPointer(),
                                                              boundGeometry.vertexData.getRawDataSizeInBytes());
                concatenatedGeometry.indexData.appendRawData(indices.data(),
                                                             indices.size() * sizeof(uint32_t));
                boundGeometryIndex++;
            }
            boundGeometries = {concatenatedGeometry};
        }

        std::string name;
        util::UniqueIdentifier uniqueIdentifier;
        assimp::SceneImporter importer = assimp::SceneImporter();

    private:
        // TODO -> evaluate how the spatial / physics engine integration should best work;
        // keeping things ultra-mega-simple for now
        space::position::Position position = space::position::Position();
        space::velocity::Velocity velocity = space::velocity::Velocity();
        space::orient::Orientation orientation = space::orient::Orientation();


        // TODO - flesh out resource bindings
        std::vector<Material> materials;
        std::vector<Texture> textures;
        std::vector<Buffer> buffers;
        std::vector<Image> images;


        /**
         * (TEMP DISABLED) Here we'll allow the Model to dynamically compute the normals for a given model at creation time
         */
        bool recalculateNormals = false;

        /**
         * This is the global model transform, which should be applied first (because it positions the model in worldspace)
         */
        glm::mat4 modelTransform = glm::mat4(0);

        /**
         * NEW IDEA -> the model itself will maintain some state for mapping each bone (by name) to a model segment;
         * the model segment itself doesn't really need to know about any animation information I think.
         *
         * We'll bring back the actual ModelSegment abstraction in a later cleanup pass, but for now we'll just
         * map it directly onto our "boundGeometries" thing; that means these will also track their own skeletons
         */
        std::vector<geometry::BoundGeometry> boundGeometries;

        // note: this will probably be dynamically built up (i guess it has to be for imported models)
        std::vector<std::shared_ptr<anim::Animation> > animations;


        /**
                 *
                 * @param node_anim
                 * @param currentKeyframeTimePointInTicks note that this is in ticks! assimp doesn't directly specify
                 * @return
                 */
        static glm::vec3 computeInterpolatedPosition(const aiNodeAnim *node_anim,
                                                     const double currentKeyframeTimePointInTicks) {
            for (uint32_t i = 0; i < node_anim->mNumPositionKeys - 1; i++) {
                double keyTimeInTicks = node_anim->mPositionKeys[i + 1].mTime;
                if (currentKeyframeTimePointInTicks < keyTimeInTicks) {
                    uint32_t nextIndex = i + 1;
                    const auto currentKey = node_anim->mPositionKeys[i];
                    if (nextIndex < node_anim->mNumPositionKeys) {
                        const auto nextKey = node_anim->mPositionKeys[nextIndex];

                        const double startTimeTicks = currentKey.mTime;
                        const double endTimeTicks = nextKey.mTime;
                        const double deltaTimeTicks = endTimeTicks - startTimeTicks;
                        const double interpolationFactor =
                                (currentKeyframeTimePointInTicks - startTimeTicks) / deltaTimeTicks;

                        if (interpolationFactor < 0 || interpolationFactor > 1) {
                            // LOG!
                            return {0, 0, 0};
                        }
                        const aiVector3d deltaPosition = {
                            interpolationFactor * (nextKey.mValue.x - currentKey.mValue.x),
                            interpolationFactor * (nextKey.mValue.y - currentKey.mValue.y),
                            interpolationFactor * (nextKey.mValue.z - currentKey.mValue.z)
                        };
                        const aiVector3d result = {
                            currentKey.mValue.x + deltaPosition.x,
                            currentKey.mValue.y + deltaPosition.y,
                            currentKey.mValue.z + deltaPosition.z
                        };
                        return {result.x, result.y, result.z};
                    }
                    return {currentKey.mValue.x, currentKey.mValue.y, currentKey.mValue.z};
                }
            }
            // LOG!
            return {0, 0, 0};
        }

        static glm::quat computeInterpolatedRotation(const aiNodeAnim *node_anim,
                                                     const double currentKeyframeTimePointInTicks) {
            // I think the code *does* work, it's our goofy rotation key data with like 40 keys that are all at time point 0 and have insane values...
            std::vector<aiQuatKey> assimpRotationKeys(node_anim->mNumRotationKeys);
            for (uint32_t i = 0; i < node_anim->mNumRotationKeys; i++) {
                // occasional thro where..
                assimpRotationKeys[i] = node_anim->mRotationKeys[i];
            }

            std::ranges::sort(assimpRotationKeys,
                              [](const aiQuatKey &a, const aiQuatKey &b) { return a.mTime < b.mTime; });

            // the assimp-imported mixamo animation data seems to often include these goofy keys
            // which happen at 0.00000000000000004 ticks or whatever
            // so we'll have to filter them out
            std::vector<aiQuatKey> filteredKeys = {};
            bool zeroKeyHasBeenCopied = false;
            constexpr auto epsilon = 0.01;
            for (auto key: assimpRotationKeys) {
                if (std::abs(key.mTime) > epsilon || (key.mTime == 0 && !zeroKeyHasBeenCopied)) {
                    if (key.mTime == 0) {
                        zeroKeyHasBeenCopied = true;
                    }
                    filteredKeys.push_back(key);
                }
            }

            for (uint32_t i = 0; i < filteredKeys.size() - 1; i++) {
                double keyTimeInTicks = filteredKeys[i + 1].mTime;
                if (currentKeyframeTimePointInTicks < keyTimeInTicks) {
                    const uint32_t nextIndex = i + 1;
                    if (nextIndex < node_anim->mNumRotationKeys) {
                        const auto currentKey = filteredKeys[i];
                        const auto nextKey = filteredKeys[nextIndex];

                        const double startTimeTicks = currentKey.mTime;
                        const double endTimeTicks = nextKey.mTime;
                        const double deltaTimeTicks = endTimeTicks - startTimeTicks;
                        const double interpolationFactor =
                                (currentKeyframeTimePointInTicks - startTimeTicks) / deltaTimeTicks;
                        if (interpolationFactor < 0 || interpolationFactor > 1) {
                            // LOG!
                            return {0, 0, 0, 0};
                        }
                        const aiQuaternion &currentKeyRotation = currentKey.mValue;
                        const aiQuaternion &nextKeyRotation = nextKey.mValue;
                        aiQuaternion result = {};
                        aiQuaternion::Interpolate(result, currentKeyRotation, nextKeyRotation, interpolationFactor);
                        return normalize(glm::quat{result.w, result.x, result.y, result.z});
                    }
                }
                if (i + 1 >= filteredKeys.size() - 1) {
                    //default: return last keyframe if we're at the end
                    return {
                        filteredKeys[i].mValue.w,
                        filteredKeys[i].mValue.x,
                        filteredKeys[i].mValue.y,
                        filteredKeys[i].mValue.z
                    };
                }
            }
            // LOG!
            // DEBUG! Uh oh! This is poopoo!
            return {0, 0, 0, 0};
        }

        static glm::vec3 computeInterpolatedScaling(const aiNodeAnim *node_anim,
                                                    const double currentKeyframeTimePointInTicks) {
            if (node_anim->mNumScalingKeys == 1) {
                return {
                    node_anim->mScalingKeys[0].mValue.x,
                    node_anim->mScalingKeys[0].mValue.y,
                    node_anim->mScalingKeys[0].mValue.z
                };
            }

            for (uint32_t i = 0; i < node_anim->mNumScalingKeys - 1; i++) {
                double keyTimeInTicks = node_anim->mScalingKeys[i + 1].mTime;
                if (currentKeyframeTimePointInTicks < keyTimeInTicks) {
                    uint32_t nextIndex = i + 1;
                    const auto currentKey = node_anim->mScalingKeys[i];
                    if (nextIndex < node_anim->mNumScalingKeys) {
                        const auto nextKey = node_anim->mScalingKeys[nextIndex];

                        const double startTimeTicks = currentKey.mTime;
                        const double endTimeTicks = nextKey.mTime;
                        const double deltaTimeTicks = endTimeTicks - startTimeTicks;
                        const double interpolationFactor =
                                (currentKeyframeTimePointInTicks - startTimeTicks) / deltaTimeTicks;
                        if (interpolationFactor < 0 || interpolationFactor > 1) {
                            // LOG!
                            return {0, 0, 0};
                        }
                        const aiVector3d deltaScaling = {
                            interpolationFactor * (nextKey.mValue.x - currentKey.mValue.x),
                            interpolationFactor * (nextKey.mValue.y - currentKey.mValue.y),
                            interpolationFactor * (nextKey.mValue.z - currentKey.mValue.z)
                        };
                        const aiVector3d result = {
                            currentKey.mValue.x + deltaScaling.x,
                            currentKey.mValue.y + deltaScaling.y,
                            currentKey.mValue.z + deltaScaling.z
                        };
                        return {result.x, result.y, result.z};
                    }
                    return {currentKey.mValue.x, currentKey.mValue.y, currentKey.mValue.z};
                }
            }
            // LOG!
            return {0, 0, 0};
        }

        void constructBoundGeometriesForAssimpMesh(const aiMesh *mesh,
                                                   const std::vector<anim::Skeleton::VertexBoneAttachment> &
                                                   boneAttachments,
                                                   const std::vector<geometry::GeometryBinding> &geometryBindings,
                                                   const glm::vec4 coordinateScalingFactors) {
            unsigned bindingIndex = 0;
            for (auto &geometryBinding: geometryBindings) {
                // pre-sort by location just in case
                std::vector<geometry::AttributeDescription> sortedAttribs = geometryBinding.vertexBinding.attributes;
                std::ranges::sort(sortedAttribs,
                                  [&](const geometry::AttributeDescription &attrib1,
                                      const geometry::AttributeDescription &attrib2) {
                                      return attrib1.shaderResourceLocation < attrib2.
                                             shaderResourceLocation;
                                  });

                // given the number of vertices, we can just quickly iterate over the
                // attributes in this binding to determine the vertex stride and then
                // from there we can determine the total buffer size we need to hold all the data:
                unsigned bindingVertexStride = 0u; // vertex width for this binding
                for (auto &attribute: sortedAttribs) {
                    bindingVertexStride += getFormatElementSizeInBytes(attribute.attributeFormat);
                }

                uint32_t normalAttributeOffset = UINT32_MAX; // for calculating normals
                uint64_t vertexBufferDataSizeInBytes = bindingVertexStride * mesh->mNumVertices;
                // temporary byte array to copy vertex data into
                std::vector<uint8_t> byteArray(vertexBufferDataSizeInBytes);

                for (uint32_t vertex = 0; vertex < mesh->mNumVertices; vertex++) {
                    // TODO -> consider multithreading this if possible by submitting jobs
                    // for each attribute
                    for (auto &attributeDescription: sortedAttribs) {
                        /**
                          * Note: this check enforces that you *must* label your attributes with usages
                          * if you're using Assimp to import the model!
                          *
                                             * This field essentially tells the engine *which* type of data to copy into
                                             * this attribute's positions within the array of interleaved vertex data for this binding.
                                             *
                                             * (Recall: you can also have de-interleaved data by just using 1 attribute per binding)
                                             */
                        if (!attributeDescription.usageLabel.has_value()) {
                            // TODO -> proper logging that this happened!
                            break;
                            // when this happens our attribs are malformed so just break the loop (todo: error handling)
                        }

                        // copy the attribute data in at this location
                        // (we might need to make sure the data formats do in fact match up)
                        // DEBUGGING: this may be the source of our woes as well;
                        // we need to invert the Y and Z axes for Vulkan, but in a platform-agnostic way;
                        // I think a simple solution is to just have some "x y z scale factor" variables
                        // for loading in a particular model
                        uint32_t currentCopyOffset = attributeDescription.vertexOffset;
                        float xScaleFactor = coordinateScalingFactors.x;
                        float yScaleFactor = coordinateScalingFactors.y;
                        float zScaleFactor = coordinateScalingFactors.z;
                        float wScaleFactor = coordinateScalingFactors.w;
                        switch (attributeDescription.usageLabel.get()) {
                            // ANOTHER POTENTIAL ISSUE: our w components are ending up zero'd here...
                            case (geometry::AttributeUsage::POSITION): {
                                auto vertPos = std::vector{
                                    xScaleFactor * mesh->mVertices[vertex].x,
                                    yScaleFactor * mesh->mVertices[vertex].y,
                                    zScaleFactor * mesh->mVertices[vertex].z,
                                    wScaleFactor * 1.0f
                                    // MAYBE THIS IS WHAT WE DO? just default it to 0?
                                };
                                std::memcpy(
                                    &byteArray[vertex * bindingVertexStride + currentCopyOffset],
                                    vertPos.data(),
                                    vertPos.size() * sizeof(float)
                                );

                                break;
                            }
                            case (geometry::AttributeUsage::COLOR): {
                                // TODO -> implement this when we're adding our shading model
                                // (via adjusting the demo shaders for Gooch shading a la RTR)
                                break;
                            }
                            case (geometry::AttributeUsage::NORMAL): {
                                if (!this->recalculateNormals) {
                                    auto vertNormal = glm::vec4{
                                        xScaleFactor * mesh->mNormals[vertex].x,
                                        yScaleFactor * mesh->mNormals[vertex].y,
                                        zScaleFactor * mesh->mNormals[vertex].z,
                                        0.0f
                                    };

                                    std::memcpy(
                                        &byteArray[vertex * bindingVertexStride + currentCopyOffset],
                                        &vertNormal,
                                        sizeof(glm::vec4)
                                    );
                                }
                                if (currentCopyOffset < normalAttributeOffset) {
                                    normalAttributeOffset = currentCopyOffset;
                                }

                                break;
                            }
                            case (geometry::AttributeUsage::UV): {
                                // for single-animated-models demo there won't even be textures
                                // so I'll skip it for the time being
                                break;
                            }
                            case (geometry::AttributeUsage::ANIMATION_BONE_INDICES): {
                                // i think we should already have easy access to this information
                                // from the animation that was built before this
                                // NOTE -> we are in fact NOT setting up our bone indices/weights properly
                                auto unpackedBoneIndices = boneAttachments[vertex].boneIndices;
                                std::memcpy(
                                    &byteArray[vertex * bindingVertexStride + currentCopyOffset],
                                    unpackedBoneIndices.data(),
                                    unpackedBoneIndices.size() * sizeof(float)
                                );
                                break;
                            }
                            case (geometry::AttributeUsage::ANIMATION_BONE_WEIGHTS): {
                                auto unpackedBoneWeights = boneAttachments[vertex].boneWeights;
                                std::memcpy(
                                    &byteArray[vertex * bindingVertexStride + currentCopyOffset],
                                    unpackedBoneWeights.data(),
                                    unpackedBoneWeights.size() * sizeof(float)
                                );
                                break;
                            }
                            default: {
                                // TODO - do nothing and log!
                            }
                        }
                    }
                }

                std::vector<glm::vec4> faceNormals(mesh->mNumFaces);
                if (recalculateNormals) {
                    // here we'll just follow this procedure for computing normals:
                    // 1. init all vert normals to 0 (should be done already)

                    // 2. compute all the face normals
                    for (int index = 0; index < mesh->mNumFaces; index++) {
                        auto face = mesh->mFaces[index];

                        // assumption: we'll always have at least 3 indices per face
                        if (face.mNumIndices >= 3) {
                            // difference vector from 1st point to 2nd point
                            const auto abVector = glm::vec3(
                                mesh->mVertices[face.mIndices[0]].x - mesh->mVertices[face.mIndices[1]].
                                x,
                                mesh->mVertices[face.mIndices[0]].y - mesh->mVertices[face.mIndices[1]].
                                y,
                                mesh->mVertices[face.mIndices[0]].z - mesh->mVertices[face.mIndices[1]].
                                z
                            );
                            // difference vector from 1st point to 3rd point
                            const auto acVector = glm::vec3(
                                mesh->mVertices[face.mIndices[0]].x - mesh->mVertices[face.mIndices[2]].
                                x,
                                mesh->mVertices[face.mIndices[0]].y - mesh->mVertices[face.mIndices[2]].
                                y,
                                mesh->mVertices[face.mIndices[0]].z - mesh->mVertices[face.mIndices[2]].
                                z
                            );

                            // compute normal by taking cross product (ENSURE THE SIGNS ARE CORRECT)
                            faceNormals[index] = glm::vec4(normalize(cross(abVector, acVector)), 0.0f);

                            // 3. add each facenormal's value to each vertex normal that is part of it
                            for (uint32_t i = 0; i < face.mNumIndices; i++) {
                                uint32_t vertexIndex = face.mIndices[i];
                                auto currentNormal
                                        = (glm::vec4 *) &byteArray[
                                            vertexIndex * bindingVertexStride + normalAttributeOffset];
                                glm::vec4 tmp = *currentNormal + faceNormals[index];
                                std::memcpy(
                                    &byteArray[vertexIndex * bindingVertexStride +
                                               normalAttributeOffset],
                                    &tmp,
                                    sizeof(glm::vec4));
                            }
                        }
                    }
                    // 4. normalize the vertex normals in-place after accumulating all their face normal values
                    for (uint32_t vertex = 0; vertex < mesh->mNumVertices; vertex++) {
                        auto *vertexNormal = (glm::vec4 *) &byteArray[
                            vertex * bindingVertexStride + normalAttributeOffset];
                        *vertexNormal = normalize(*vertexNormal);
                    }
                }

                // create raw data container that will store our vertex data
                ByteArray vertexDataContainer(
                    name + " Model Vertex Data",
                    UniqueIdentifier(),
                    byteArray.data(),
                    byteArray.size() // note: this is already the size in bytes
                );

                // gather indices from assimp mesh
                // NOTE -> this might have some kind of problem?
                std::vector<uint32_t> indices = {};
                for (int faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++) {
                    for (int i = 0; i < mesh->mFaces[faceIndex].mNumIndices; i++) {
                        indices.push_back(mesh->mFaces[faceIndex].mIndices[i]);
                    }
                }

                ByteArray indexDataContainer(
                    name + " Model Index Data",
                    UniqueIdentifier(),
                    reinterpret_cast<unsigned char *>(indices.data()),
                    indices.size() * sizeof(uint32_t)
                );

                boundGeometries.push_back(
                    {
                        bindingIndex,
                        vertexDataContainer,
                        indexDataContainer,
                        mesh->mNumVertices,
                        static_cast<unsigned>(indices.size())
                    }
                );

                bindingIndex++;
            }
        }
    };
}
