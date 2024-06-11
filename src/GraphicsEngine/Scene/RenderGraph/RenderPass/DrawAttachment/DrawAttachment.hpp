//
// Created by paull on 2023-10-02.
//

#pragma once

#include <utility>
#include <vector>
#include "../../../SceneResources/Renderable/Renderable.hpp"
#include "../../../SceneResources/Model/Model.hpp"

namespace pEngine::girEngine::scene::graph::renderPass {

    /**
     * To keep this simple for the first pass, I'll just have this consist of some
     * collection of scene objects that are being bound to a particular render pass
     *
     * In the future I may have to use this as a base class or provide some other options
     * for non-traditional rendering pipelines (eg mesh shading type things, raytracing, etc)
     */
    class DrawAttachment {
    public:
        /**
          * This shadows the gir::DrawAttachmentIR::DrawType for now(?) - it denotes how
          * the attached geometry is to be drawn
          */
        enum class DrawType {
            UNKNOWN,
            DRAW_VERTICES,
            INDEXED_DRAW,
            POINT_SET,
            // TODO - add any others
        };

        DrawAttachment(DrawType drawType,
                       const std::vector<vertex::VertexAttributeDescription> &vertexAttribs,
                       const util::Hash &vertexAttributeHash,
                       const std::vector<std::shared_ptr<Model>> &initialModels)
                : drawType(drawType),
                  vertexAttributeHash(vertexAttributeHash),
                  vertexAttributes(vertexAttribs) {
            // idea: filter out the vertex buffers that we're binding
            for (const auto &model: initialModels) {
                for (const auto &renderable: model->getRenderables()) {
                    for (const auto &buffer: renderable->getVertexBuffers()) {
                        const auto &vertexBuffer = std::dynamic_pointer_cast<VertexBuffer>(buffer);
                        if (vertexBuffer->vertexAttributeHash == vertexAttributeHash) {
                            // add it in baybe
                            vertexBuffers.push_back(buffer);
                        }
                    }
                    for (const auto &indexBuffer: renderable->getIndexBuffers()) {
                        // TODO - figure out what we gotta do to get index buffer bindings goin
                        // (should be simpler and follow the same lines as vertex buffers I hope)
                        // I'll leave actually handling index buffers for a little bit later - generally
                        // the details of this can be left to when we're actually submitting geometry for drawing
                    }
                }
            }
        }

        DrawAttachment(DrawType drawType,
                       const std::vector<vertex::VertexAttributeDescription> &vertexAttribs,
                       const util::Hash &vertexAttributeHash,
                       const std::vector<std::shared_ptr<Renderable>> &initialRenderables)
                : drawType(drawType),
                  vertexAttributeHash(vertexAttributeHash),
                  vertexAttributes(vertexAttribs) {
            // TODO - filter buffers outta renderables
        }

        DrawAttachment(DrawType drawType,
                       const std::vector<vertex::VertexAttributeDescription> &vertexAttribs,
                       const util::Hash &vertexAttributeHash,
                       const std::vector<std::shared_ptr<Renderable>> &renderables,
                       const std::vector<std::shared_ptr<Model>> &models)
                : drawType(drawType),
                  vertexAttributeHash(vertexAttributeHash),
                  vertexAttributes(vertexAttribs) {
            // TODO - filter buffers outta renderables and models
        }

        DrawAttachment() = default;

        DrawType drawType = DrawType::UNKNOWN;


        // I think the new design will be that each attachment corresponds to a particular set of input attributes;
        // this might change but I think that will probably suffice...

        // one thing now that I'm trying to flesh this out a bit: we gotta probably restructure how the renderables & models are stored
        // we have to store, for all the vertex buffers contained within each bound renderable & model:

        // -> (globally for the whole DrawAttachment struct) the user-specified vertex attribute information; this will be
        // checked against what gets reflected (or at least certain things will be? maybe we won't need to check much? idk yet)

        // for the purposes of having a particular draw attachment correspond to one vertex binding
        // in the graphics pipeline, I'm not sure whether I'll have to instead have each bound renderable provide
        // a set of attributes; I think it might work to have each draw attachment only contain the attributes for a
        // particular binding, but then you'd have to add into the scene objects some way to store vertex buffers
        // alongside which vertex attributes that the buffer is composed of;

        // I think I'll have to sketch it out a bit more to get a clear mental picture:

        // -> in the scene, you have your renderables (at the base level) which contain some sort of geometry data that
        // is contained in a vertex buffer

        // -> each vertex buffer maintains a set of attribute descriptions (describing 1 attrib each), and hashes them
        // as well

        // -> when you want to draw some renderables, you have to give them to a render pass; each scene render pass
        // logically corresponds with a vulkan render pass (the new 1.2 variety to start with);

        // -> the render pass accepts renderables in the form of one of these draw attachments; each draw attachment
        // (which will be stored in a vector of draw attachments) defines a particular vertex binding (in vulkan),
        // ie it will be given the binding index that it has in the (scene & gir) render pass attachments

        // -> hmm maybe i was right before; we can just have the draw attachment contain a list of input attribute descriptions
        // (which can be baked into some new GIR shite) and then a set of vertex buffers who should have a matching
        // input hash which should be very easy to check hopefully ;)

        // THE MISSING INGREDIENT I THINK: we just gotta make it so that each draw attachment takes in (as ctor args):
        // a.) the vertex attribute descriptions (so they can be hashed)
        // b.) the renderables that you want to attach; it will filter through each of them (comparing the hashes)
        // (alternative: could make it just take in the list of vertex buffers directly; probably best done as a separate ctor)

        std::vector<vertex::VertexAttributeDescription> vertexAttributes = {};

        util::Hash vertexAttributeHash = 0;

        std::vector<std::shared_ptr<Buffer>> vertexBuffers = {};

        // TODO - index buffer shite
        std::vector<std::shared_ptr<Buffer>> indexBuffers = {};

        // UNRELATED TO ABOVE: do I need to store layout information somewhere?
    };

} // scene
