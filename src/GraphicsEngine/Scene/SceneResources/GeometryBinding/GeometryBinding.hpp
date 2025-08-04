//
// Created by paull on 2024-10-01.
//

#pragma once

#include "VertexData/VertexDataBinding.hpp"
#include "IndexData/IndexDataBinding.hpp"

namespace pEngine::girEngine::scene::geometry {
    /**
     * This is a very simple first-pass for traditional tris & indices model representations.
     * The idea is that it contains the attribute information that the
     * renderer needs to be able to feed the geometry to graphics pipelines.\n\n
     *
     * Fundamentally, it defines the binding interface for a particular batch of vertex data
     * that is meant to hook into a compatible shader (ie one whose binding interface works with
     * this particular definition). \n\n
     *
     * We're only supporting simple traditional graphics pipeline vertex + index buffer bindings
     * but that will change in the future.
     *
     * DESIGN: i'm not entirely sure where I want these to be created, nor am I sure about
     * where they should even be integrated in the current Scene ecosystem (of horrible mutant creatures
     * begging to be put out of their misery).
     *
     * I'm thinking it could make sense to associate these with maybe the vertex shader attachment?
     * Ultimately it's defining the gpu<->vertexData interface for that particular shader,
     * and I can't think of any reasons to specify it separately.
     *
     * I'll try just adding it as an input to the registerVertexShader() or whatever it's called in
     * the render pass
     */
    struct GeometryBinding {
        // ultra simple for now; for now we just pair up vertex and index data bindings
        // and then externally to this struct we can have a collection of buffer data mapped to
        // each geometry binding;

        uint32_t bindingIndex;
        VertexDataBinding vertexBinding;
        IndexDataBinding indexBinding;
    };

    /**
     * For the single-animated-model demo, we'll be exclusively supporting
     * traditional vertex & index buffer geometry, and we'll be assuming that our data
     * will come in the form of a vertex buffer with matching index buffer.\n\n
     *
     * Note: this is not to be confused with GeometryBinding, which configures a particular
     * binding point to which these BoundGeometry will bind their vertex and index data.\n\n
     *
     * However, the @targetGeometryBindingIndex field is intended to specify which binding slot
     * the geometry data is bound to for a given container
    */
    struct BoundGeometry {
        unsigned targetGeometryBindingIndex = 0;

        util::ByteArray vertexData;

        util::ByteArray indexData;

        // I think I'm gonna need to add more information here; namely vertex count for now
        // but it might be convenient to store other information (eg some of the vertex info)
        unsigned vertexCount = 0;

        unsigned indexCount = 0;
    };
}
