//
// Created by paull on 2023-10-13.
//

#pragma once

#include "ColorBlendConfiguration/ColorBlendConfiguration.hpp"
#include "DepthStencilConfiguration/DepthStencilConfiguration.hpp"
#include "DynamicStateConfiguration/DynamicStateConfiguration.hpp"
#include "MultiblendConfiguration/MultisampleConfiguration.hpp"
#include "PrimitiveAssemblyConfiguration/PrimitiveAssemblyConfiguration.hpp"
#include "RasterizationConfiguration/RasterizationConfiguration.hpp"
#include "TessellationConfiguration/TessellationConfiguration.hpp"
#include "VertexInputConfiguration/VertexInputConfiguration.hpp"

namespace pEngine::girEngine::scene::graph::renderPass::pipeline {

    /**
     * TODO - fill this in with all the stuff needed to build a graphics pipeline IR... \n\n
     *
     * ALSO TODO - move the info in this blurb into the documentation (docs will make this project look juicier to people)
     *
     * The first pass over this struct will probably just be filling it with one field for each of
     * the graphics pipeline configuration IR classes; Since many of the individual configuration IR classes
     * themselves require different enums and structs to be defined, it probably makes more sense to mirror
     * the classes in this directory instead of trying to "unwrap" it all here, or else this file will be
     * too bloated for no real benefit. \n\n
     *
     * A later task can come back and evaluate how it's all fitting together and refactor as needed I think.
     * For now it's quick and dirty.\n\n
     *
     * I think it makes sense anyway to have the default/engine-provided Scene implementation basically be
     * a free-floating implementation which just has the goal of providing a robust interface for creating
     * fancy graphics applications with only a few C++ classes (or at least relatively few).\n\n
     *
     * Then it's fine if the scene abstractions match up with the gir, because that's just one way of doing it,
     * and the focus of the scene really is to provide an interface for easily building up a GIR scene and incorporating
     * external code for gameplay logic/etc, while providing lots of handy tools for animation and physics and stuff. \n\n
     *
     * Besides, hypothetically you could write your own weird little mini-front-end that just builds GIR and then even
     * that would work with the backend. In general I think the compiler frontend-IR-backend model works pretty well
     * here and will hopefully help keep the codebase more manageable to work with.
     */
    struct GraphicsPipelineConfiguration {

        ColorBlendConfiguration colorBlendConfiguration = {};

        DepthStencilConfiguration depthStencilConfiguration = {};

        DynamicStateConfiguration dynamicStateConfiguration = {};

        MultisampleConfiguration multisampleConfiguration = {};

        PrimitiveAssemblyConfiguration primitiveAssemblyConfiguration = {};

        RasterizationConfiguration rasterizationConfiguration = {};

        TessellationConfiguration tessellationConfiguration = {};

        VertexInputConfiguration vertexInputConfiguration = {};

    };

}
