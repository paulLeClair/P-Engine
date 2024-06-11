#pragma once

#include "../GraphicsIntermediateRepresentation.hpp"
#include "../ResourceIR/BufferIR/BufferIR.hpp"
#include "../VertexAttributeIR/VertexAttributeIR.hpp"

#include "../../../utilities/Hash/Hash.hpp"

namespace pEngine::girEngine::gir {

    /**
     * Instead of my pre-existing idea for this whole thing, I'm going to try and
     * unify geometry and draw command representation and make the overall interface as simple as possible.\n\n
     *
     * For now, it'll just support simple indexed draws, but I'll extend this to be able to include
     * stuff like tessellation patches or whatever else comes up.\n\n
     *
     * Basically, draw commands are obtained by the user setting up *geometry bindings*
     * in the scene's RenderPass classes; this tells the system that the user's intent
     * is to have a model (or renderable) drawn as part of a particular render pass in a scene.
     * It's assumed that the scene object in the geometry binding (ie either a model or renderable)
     * will be able to provide vertex and index buffers that will be bound for the draw call itself.
    */
    class DrawAttachmentIR : public GraphicsIntermediateRepresentation {
    public:
        /**
         * Each enum class member denotes a possible type of draw call. \n\n
         *
         * I'll go through and extend this with all the different varieties soon but for now
         * I think we can just focus on trying to get this to be able to support a `vkCmdDrawIndexed`
         * in the backend.\n\n
         *
         * Once that works we can start expanding this to support stuff like point clouds,
         * instanced draws, etc.
         */
        enum class DrawType {
            UNKNOWN,
            INDEXED_DRAW,
            POINT_CLOUD,
            INSTANCED_DRAW,
            // TODO - add stuff like tessellation, mesh shading stuff, raytracing, etc
        };

        struct CreationInput : public GraphicsIntermediateRepresentation::CreationInput {
            DrawType drawType = DrawType::UNKNOWN;

            std::vector<gir::vertex::VertexAttributeIR> vertexAttributes = {};

            util::Hash vertexAttributeHash = 0;

            // trying to store all vertex buffers and index buffers
            // inside draw commands;
            std::vector<std::shared_ptr<BufferIR>> vertexBuffers;
            std::vector<std::shared_ptr<BufferIR>> indexBuffers;

            // TODO - add any other types of bindings needed for a draw call
            // for now I'll leave it just with vertex & index buffers and draw type
        };

        explicit DrawAttachmentIR(const CreationInput &creationInput)
                : GraphicsIntermediateRepresentation(creationInput),
                  drawType(creationInput.drawType),
                  vertexAttributes(creationInput.vertexAttributes),
                  vertexAttributeHash(creationInput.vertexAttributeHash),
                  vertexBuffers(creationInput.vertexBuffers),
                  indexBuffers(creationInput.indexBuffers) {

        }

        ~DrawAttachmentIR() override = default;

        [[nodiscard]] DrawType getDrawType() const {
            return drawType;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<BufferIR>> &getVertexBuffers() const {
            return vertexBuffers;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<BufferIR>> &getIndexBuffers() const {
            return indexBuffers;
        }

        [[nodiscard]] const std::vector<vertex::VertexAttributeIR> &getVertexAttributes() const {
            return vertexAttributes;
        }

        [[nodiscard]] util::Hash getVertexAttributeHash() const {
            return vertexAttributeHash;
        }

    private:
        DrawType drawType;

        std::vector<vertex::VertexAttributeIR> vertexAttributes = {};

        util::Hash vertexAttributeHash = 0;

        std::vector<std::shared_ptr<BufferIR>> vertexBuffers;
        std::vector<std::shared_ptr<BufferIR>> indexBuffers;

    };

}