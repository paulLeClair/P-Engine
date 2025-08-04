//
// Created by paull on 2024-10-12.
//

#pragma once

#include "../GraphicsIntermediateRepresentation.hpp"
#include "../VertexAttributeIR/VertexAttributeIR.hpp"

namespace pEngine::girEngine::gir::vertex {

    // I'm going to subclass this but I kinda hate this whole inheritance-based design...
    // all it's doing is just labeling which classes can be considered GIR, and even that
    // is flimsy because I already didn't use inheritance for certain subclasses; the line becomes
    // blurry when you start having GIR classes that themselves are composed of smaller structs;
    // the more I think about it, I'm pretty sure I could get the exact same shit happening by just
    // having each class/struct be completely C-style, and instead of a base class I maintain a large
    // enum (or some other label representation) where the GIR classes just include a particular value
    // for that enum...
    // on the other hand, I'm not sure how much of a big rewrite that would entail given that it's not as
    // easy to assert "this class has a GIR label" when you don't use inheritance; and we could probably
    // achieve a lot of streamlining by instead just making this GIR parent class add one single member
    // (which would be the label) and avoid ripping out every possible instance of inheritance.
    // At least for the single-model demo homestretch, I don't wanna get bogged down on a huge refactor like that
    struct VertexInputBindingIR : public GraphicsIntermediateRepresentation {

        struct CreationInput : public GraphicsIntermediateRepresentation::CreationInput {

            std::vector<VertexAttributeIR> attributes = {};


        };

        explicit VertexInputBindingIR(const CreationInput &creationInput)
                : GraphicsIntermediateRepresentation(creationInput), attributes(creationInput.attributes) {

        }

        VertexInputBindingIR() : GraphicsIntermediateRepresentation({}), attributes({}) {

        }

        VertexInputBindingIR(const VertexInputBindingIR &other) = default;

        // similar to the scene::GeometryBinding thing, I'll just try having the render pass store a sequence of
        // these bindings, which themselves are a sequence of attributes
        std::vector<VertexAttributeIR> attributes = {};

        // TODO -> any index buffer configuration stuff (for now I'm just gonna copy the 32-bit indices straight through
        // from assimp)

    };

}