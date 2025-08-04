//
// Created by paull on 2024-05-23.
//

#pragma once


namespace pEngine::girEngine::backend {
    /**
     * Current idea (subject to change):
     * This will be the base class for different presentation modes that the user may want;
     * since all presentation modes involve first acquiring a swapchain image and then presenting
     * it after rendering has completed, those are the main 2 points where polymorphism will come into play.
     *
     * This way I can easily add new presentation modes in later issues and it should nicely separate out all the
     * swapchain-specific code into this one place; as far as the actual renderer will be concerned, it shouldn't matter
     * where the image came from (ie it's a swapchain image, or otherwise (most likely) it's some kind of image you created
     * explicitly for offline rendering/dynamic textures/whatever).
     *
     * This base class will serve as the "interface" but the majority of implementation details will have to be
     * implemented as subclasses that are focused around a backend (or multiple backends if you're kooky insane)
     *
     * NOTE -> it seems like this is not the best use case for the interface->impl/dependency injeection kinda design
     * that exists ATOW. Unless of course I decide to just hide the interface method behind another method which defeats
     * the purpose of having an interface at all (since our VulkanPresentationEngine is gonna have a different func sig)
     *
     *
     */
    class PresentationEngine {
    public:
        virtual ~PresentationEngine() = default;

        // ONE THING: I gotta figure out how the specific swapchain image should be given back...
        // super simplistic way to do it (maybe?): just have this return an index to the particular swapchain image.
        // then the acquire swapchain image function would just return you the index, and you present that particular index as well
        // this design is a little flimsy in terms of allowing the user to potentially goof up the index and present/render to the wrong image;
        // however the alternative of finding a way to return a particular image handle when we don't know the actual image representation
        // seems a bit rougher.
        // This also would mean that you would have to externally track the swapchain images; however that kinda already lines up
        // with how I wrote the swapchain thing anyway. So maybe that's okay.
        // It may turn out that we may want to somehow combine the PresentationMode shit with the Swapchain stuff but for now I'll separate em.

        /**
         * I'm gonna try something weird here; return a boost optional so that you can potentially signal a failure
         * @return
         */
        virtual boost::optional<unsigned> acquireNextSwapchainImageIndex() = 0;

        // the other thing that using indices makes necessary: the presentation engine itself will need to hold onto these images.
        // That's kinda okay though I think because it can just hold onto a handle to the swapchain itself.
        // virtual bool presentSwapchainImage(unsigned swapchainImageIndex) = 0;
    };
}
