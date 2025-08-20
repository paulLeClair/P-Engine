#pragma once

#include <string>
#include <vector>

#include <filesystem>

#include "../SceneResource.hpp"
#include "../../../../../utilities/assimp/SceneImporter.hpp"
#include "../Buffer/Buffer.hpp"
#include "../Material/Material.hpp"

#include "../../../../GraphicsIR/model/ModelIR/ModelIR.hpp"
#include "../../SceneSpace/Orientation/Orientation.hpp"
#include "../../SceneSpace/Position/Position.hpp"
#include "../../SceneSpace/Velocity/Velocity.hpp"
#include "../GeometryBinding/GeometryBinding.hpp"

// ugly macros for now
#define MAX_BONES_PER_MODEL 255
#define MAX_BONES_PER_VERTEX 4

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
                                                             images(creationInput.images) {
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

        std::string name;
        UniqueIdentifier uniqueIdentifier;
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
    };
}
