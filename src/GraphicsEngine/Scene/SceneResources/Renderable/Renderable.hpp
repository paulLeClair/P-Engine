#pragma once

#include "../SceneResource.hpp"
#include "../Buffer/VertexBuffer/VertexBuffer.hpp"
#include "../Buffer/IndexBuffer/IndexBuffer.hpp"

#include <utility>
#include <stdexcept>


namespace pEngine::girEngine::scene {

    // TIME TO DO A REFACTOR HERE
    // well a medium one really.
    // I think the main idea is that I can hopefully streamline this (maybe remove the RenderableType stuff?)
    // and have it so that renderables are able to contain vertex buffers and index buffers and that's it...?

    // one thing: i think the way I was trying to have different types of renderables all be represented as 1 class
    // is flawed; i should probably split each of them out under a "renderable" namespace and have a class for
    // point data, a class for pure vertex-based meshes, a class for index triangle meshes, a class for tess patches, etc
    // there might be a good amount of duplication between them (which maybe I can factor out into a base class),
    // but I think different types of renderables have to be handled differently anyway so the complexity is unavoidable.

    // this would mean I'd have to make sure that draw attachments respect that info so it can be processed into whatever
    // other information is needed; hmmmm actually upon thinking about this a bit more, maybe we can just keep it kinda how it is,
    // and then the draw attachment can be responsible for filtering the attached renderables/buffers out...
    // THAT COULD WORK because then we could keep the idea of the scene draw attachment only maintaining a list of concrete renderables
    // that we then filter out in the gir side of things and then of course the backend will process that info into the applicable vulkan stuff

    /**
     * Now that we're baking scene geometry, we'll need to figure out renderables as well...
     *
     * I think honestly we might as well just tie in vertex/index buffers here...
     * But also allow them to be created separately as just pure buffers.
     *
     * This could be an issue: Resource bakeToGir() hasn't been done for these yet and they're composite objects;
     * on the other hand, maybe it's fine because I may have forgotten that I handled this in the frigging DrawAttachmentIR;
     * that's probably where renderables are handled.
     *
     * In the future if this turns out to be too gross then we could redo it in a way where the scene and IR match up
     * 1-1 as in some of the other areas of the scene & gir namespaces.
     */
    class Renderable {
    public:
        enum class RenderableType {
            UNKNOWN,
            POINT_SET,
            TRIANGLE_MESH,
            INDEXED_TRIANGLE_MESH,
            TESSELLATION_PATCH
        };

        struct CreationInput {
            std::string name;
            util::UniqueIdentifier uniqueIdentifier;
            RenderableType type = RenderableType::UNKNOWN;

            std::vector<std::shared_ptr<Buffer>> vertexBuffers;
            std::vector<std::shared_ptr<Buffer>> indexBuffers;
        };

        explicit Renderable(const CreationInput &creationInput) :
                name(creationInput.name),
                uniqueIdentifier(creationInput.uniqueIdentifier),
                type(creationInput.type),
                vertexBuffers(creationInput.vertexBuffers),
                indexBuffers(creationInput.indexBuffers) {

        }

        [[nodiscard]] const std::string &getName() const {
            return name;
        }

        [[nodiscard]] const util::UniqueIdentifier &getUid() const {
            return uniqueIdentifier;
        }

        [[nodiscard]] RenderableType getType() const {
            return type;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<Buffer>> &getVertexBuffers() const {
            return vertexBuffers;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<Buffer>> &getIndexBuffers() const {
            return indexBuffers;
        }

    private:
        std::string name;
        util::UniqueIdentifier uniqueIdentifier;

        RenderableType type = RenderableType::UNKNOWN;

        std::vector<std::shared_ptr<Buffer>> vertexBuffers;
        std::vector<std::shared_ptr<Buffer>> indexBuffers;
    };

}