//
// Created by paull on 2023-03-31.
//

#pragma once

#include <boost/optional/optional.hpp>

#include "../../../../../GraphicsIR/VertexAttributeIR/VertexAttributeIR.hpp"
#include "../../../formats/ResourceFormat/ResourceFormat.hpp"

// TODO - have to maybe do a bit of refactoring of how we're tracking/representing vertex attributes
// one potential option: just use templates and make it so that you can obtain various bits of info about the
// individual attributes (eg the stride n shit like that)
// since we want to be able to bind in different vertex buffers to different attributes potentially,
// but I think they also have to be contiguous inside a buffer, maybe all we have to do is just provide
// attribute sizes and have it be computed dynamically...?

// the other thing is to consider breaking things up so that you basically supply the buffer(s) you want
// assigned to contiguous attributes that are obtained from the shader, and the design is focused around
// providing a set of attributes... that could make more sense and be slightly more flexible

// the idea would be:
// -> provide a bunch of pre-set (templated) attributes (position, UV coords, normal, tangent, etc) that are
// each assigned a particular binding index as well as the vertex buffer that's being bound to those (contiguous) attributes
// ->

// oo the only thing though - how would we handle the vertex data in that case?

// one option could be to say "we trust you, author" and have the data be copied in/out directly and
// assume it's in the correct format for what you're doing; this is the easiest for now but stinks of "it just works"

// another option might be to use a bit of a hybrid approach; we add in the vertex attribute information stuff
// and refactor how vertex types are represented (with templates, as planned); then we can easily get/set the data
// as an array of the particular vertex type you're using, and also compute strides and what not;
// but we also have the option of having the user pre-pack the data into separate buffers with the attributes they
// want to, or interleave the data into one big buffer (which is another can of worms)

// okay so I think the idea is that in shaders you don't even necessarily have to specify an attribute index (ie layout(location = whatever))
// which means we either:

// 1. acquire the attribute indices for each attribute based on the bound buffers;
// -> eg if we have 4 attributes (no layout specified) and we have 2 buffers bound, one with pos/uv and one with tan/normal,
// then we would assign the attrib indices as 0 and 1 for each of the two bound buffers and also copy in indices that the buffers are bound to
// for acquiring the final Vulkan objects that are needed for vertex input specification in pipeline creation.
// this would also pave the way for more high-level specification of how you want to have your vertex shader inputs bound; ideally i want the front-end to hide
// a ton of the GIR and backend info if possible

// 2. use the specified layout location if that information is present in the shader reflection info;
// this way, you can also have your vertex buffer data layout written into your shader if that's the vibe you want for
// the graphics app you're making

// this seems fairly simple i guess, and hopefully won't be a ton of extra work

// ya this file needs a big overhaul methinks


namespace pEngine::girEngine::scene::vertex {

    /**
     * DEPRECATED
     */
    struct VertexAttributeDescription {
        // ideally this should match up with the attribute name in the actual shader; not sure if it'll be actually used
        std::string attributeName;

        // TODO - verify that this lines up enough with what vulkan needs us to specify
        /**
         * This (I think) is shader-specific, it labels which of the shader's 32-bit interface slots that the
         * attribute memory starts in (in certain cases, you can have a single attribute taking up multiple slots). \n\n
         *
         * This is able to be specified inside the shader, in which case the value from the shader will override this
         * value. We should also be able to support the case that the user has the attributes
         * specified using the "attribute" keyword, which I think forces us to determine a location on a per-binding basis
         *
         * Honestly I think I'll just make this an optional for now, since we might as well allow it to just be reflected from the shader
         * or figured out automatically
         */
        boost::optional<unsigned> attributeShaderLocation = boost::none;

        unsigned bindingIndex = 0;

        ResourceFormat attributeFormat = ResourceFormat::UNDEFINED;

        /**
         * This should correspond to where this attribute is located inside the vertex element memory,
         * which will be used to access this particular attribute inside the vertex shader.\n\n
         *
         * It's very important to set this properly so that the vertex shader accesses the correct spot in memory
         * for this attribute.
         *
         * However I'm not entirely sure that we can't just determine this dynamically... but something like that
         * might require info we don't have. To be safe I'll just leave it as a specified thing for now and re-evaluate later
         */
        unsigned attributeByteOffset = 0u;


    };


};

// TODO - expand with other vertex types that we want to support
