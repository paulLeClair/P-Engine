//
// Created by paull on 2023-11-26.
//

#pragma once

namespace pEngine::girEngine::backend::vulkan {

    /**
     * Just making this stub class right now so I remember that we can set it up so that
     * each draw call can be set up so the frame execution controller basically just
     * calls "device.getCommandBuffer()" and then from there we just set up the command buffer
     * wrapper class (this class, which is what the device gives the frame execution controller)
     * so that it accepts a VulkanProgram that it will bind; I'll have to adapt the details to my
     * much more bare-bones and baby-brain engine but it should be a useful overall design to start from.
     */
    class VulkanCommandBuffer {

    };

} // vulkan
