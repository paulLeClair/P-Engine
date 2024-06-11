//
// Created by paull on 2023-10-02.
//

#pragma once

#include <memory>
#include "../../../SceneResources/Texture/Texture.hpp"

namespace pEngine::girEngine::scene::graph::renderPass {

    /**
     * To make the interface more consistent, I think I'll use a similar *Attachment structure for
     * all the different RenderPass bindings. \n\n
     *
     * Okay we're at the point in scene baking now that we need to flesh these out.
     * I think for the ImageAttachment stuff I basically just looked into what was needed for image attachments
     * in Vulkan, so we can follow a similar pattern here.
     *
     * Since textures involve an image, I think it does make sense to have this inherit from ImageAttachment potentially,
     * or at least duplicate whatever state is relevant for that.
     *
     * Textures make use of samplers so I imagine there will be some sampler configuration here too...?
     *
     * I'm still not super educated on textures/samplers in Vulkan, but I think we can use these assumptions:
     *
     * - the sampler for a texture is supplied when you're creating a descriptor set (so we want to store all the info
     * that has to be provided there in this struct)
     *
     * I'm not really sure, but I think we might just be able to shove all the scene::Texture and scene::Sampler stuff
     * into this class, 1-1 pretty much, and then we can ensure we have everything we need once we get into the
     * backend bake juicy shite.
     *
     * In that case, this scene class is probably okay to just hold on to the bound texture, and then we can
     * rip all the fields out and stick them in the gir::renderPass::TextureAttachmentIR class?
     */
    struct TextureAttachment {

        /**
         * This denotes a particular shader stage that you're binding the texture to
         */
        enum class ShaderStage {
            VERTEX,
            FRAGMENT
        };

        std::shared_ptr<Texture> textureResource = nullptr;

        std::vector<ShaderStage> shaderStages = {ShaderStage::FRAGMENT};
    };

} // scene
