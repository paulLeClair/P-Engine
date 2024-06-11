//
// Created by paull on 2023-03-31.
//

#pragma once

#include <boost/optional/optional.hpp>
#include "../../../../../../lib/glm/glm.hpp"

#include "../../../../../GraphicsIR/VertexAttributeIR/VertexAttributeIR.hpp"

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


namespace pEngine::girEngine::scene::vertex {
    // new design: pure abstract base class that provides a method for providing strides/etc for each attribute...
    // not entirely sure the best design there

    // this is tricky...
    // on the one hand we could design it around having each group of attributes be logically tied to 1 buffer,
    // but that's actually trash cause you might be doing other things with your organization of vertex data.

    // I guess we can define vertex types that can provide information about each attribute
    // and then from there you can bind buffers to each attribute...? or group of attributes...?

    // this way if the layout info is specified we can just pull that in and then use the scene-level vertex buffer binding
    // info to resolve that info, while allowing for dynamic resolution of layout info as well (if only buffer binding info is specified
    // and shaders don't provide layout info)

    /**
     * Hmm... okay so this seems to make a bit of sense... but I need to think about where this should be integrated
     * and how the user should be made to provide this data.
     *
     * Certainly some of it would be best sourced from the vertex buffer itself; but on the other hand I gotta think about
     * how you would facilitate easily creating more vertex buffers of a given vertex type; we wouldn't want to be
     * specifying a lot of this stuff for every vertex buffer created.
     *
     * One idea that could bear fruit (if developed): add in another construct (something like a `VertexBufferBinding`) to the scene, probably
     * in the render pass or something, where you specify the specific vertex buffers that are bound to specific groups of attributes.
     *
     *
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

        gir::vertex::AttributeComponentSignedness componentSignedness = gir::vertex::AttributeComponentSignedness::UNKNOWN;

        /**
         * This describes the types of the individual components of this attribute.
         */
        gir::vertex::AttributeComponentSpace componentSpace = gir::vertex::AttributeComponentSpace::UNKNOWN;

        /**
         * This describes the specific component format of the attribute;
         */
        gir::vertex::AttributeComponentComposition componentComposition = gir::vertex::AttributeComponentComposition::UNKNOWN;

        /**
         * This should describe the size of the components in bytes; obviously this isn't a true
         * catch-all because there are types with different component sizes b/w them, but they're generally
         * special cases and I can avoid them for now... besides you can probably deduce whether to use those ones in other ways.
         */
        size_t individualComponentSizesInBytes = 0u;


        /**
         * This describes whether the attribute is packed / compressed in some way.
         */
        gir::vertex::AttributeDataPacking dataPacking = gir::vertex::AttributeDataPacking::NONE;
    };


};

// TODO - expand with other vertex types that we want to support
