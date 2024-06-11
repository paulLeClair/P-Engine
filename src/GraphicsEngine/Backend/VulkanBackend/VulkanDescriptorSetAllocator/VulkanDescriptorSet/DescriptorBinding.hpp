//
// Created by paull on 2024-02-08.
//

#pragma once

#include <boost/optional/optional.hpp>

#include "../../../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"

namespace pEngine::girEngine::backend::vulkan::descriptor::set {
    /**
     * \brief This is the particular type of binding, whether it's a copy or write binding basically.
     */
    enum class BindingType {
        UNKNOWN,
        WRITE,
        COPY_SOURCE,
        COPY_DESTINATION
    };

    /**
     * \brief The virtual (and empty lol) base class for DescriptorBindings, which are used
     * as part of binding descriptors to a particular descriptor set.
     *
     * TODO - consider whether to give this a template argument that allows it to maintain a
     * handle to a particular resource as opposed to a unique identifier.
     */
    struct DescriptorBinding {
        virtual ~DescriptorBinding() = default;

        virtual BindingType getBindingType() {
            return BindingType::UNKNOWN;
        }

        util::UniqueIdentifier boundResourceUid;
        // TODO - replace this with a template argument reference to some object
        // on second thought, only write bindings need an actual resource bound in, so we can make just the write
        // binding a template class and then just have this class define the getBindingType() method as its main
        // purpose

        explicit DescriptorBinding(const util::UniqueIdentifier &resource_id)
            : boundResourceUid(resource_id) {
        }

        friend bool operator==(const DescriptorBinding &lhs, const DescriptorBinding &rhs) {
            return lhs.boundResourceUid == rhs.boundResourceUid;
        }

        friend bool operator!=(const DescriptorBinding &lhs, const DescriptorBinding &rhs) {
            return !(lhs == rhs);
        }
    };

    /**
    * \brief In this case, the specified resource is written into a binding as represented here;
    * a particular resource (VkImage, VkBuffer, or VkTexelBufferView) will be bound to the descriptor.
    *
     *
     * TODO - consider whether we break this up so that you can only bind a single handle of each resource type
     * We can then add support for arrays of resources later, and support them currently by just requiring you to
     * specify a binding for each array resource (which kinda defeats the purpose of arrays but alas, we're still
     * in the R&D stages)
    */
    struct WriteBinding final : DescriptorBinding {
        BindingType getBindingType() override {
            return BindingType::WRITE;
        }

        // we don't need a destination set, cause I think we'll be getting that implicitly from the
        // descriptor set (wrapper) whose "update()" we're calling

        /**
         * \brief This is the binding slot within the descriptor set (so it must be between 0 and the size of the set);
         * the descriptor at this index is what will be written to when this write descriptor binding is applied.
         */
        uint32_t bindingIndex = 0;

        /**
         * \brief If an *array* of descriptors is found at the given @bindingIndex then this can be used to
         * access any other elements; otherwise (ie when you have a singular descriptor) it must be set to 0.
         */
        uint32_t descriptorArrayIndex = 0;

        /**
         * \brief If you have an array of elements and want to update them all at once, you can specify a number
         * of descriptors; starting from the supplied array index, it will iterate through and copy the
         * resources' contents into the array according to the count. \n\n
         *
         * If this is used, the resources copied will be taken from one of the vectors below
         */
        uint32_t descriptorCount = 1;

        /**
         * \brief This is the type of Vulkan descriptor you're binding; eg uniform buffer, sampled image, what have you.
         * See the Vulkan spec for VkDescriptorType for more information \n\n
         *
         * This value is used to determine which of the bound descriptor arrays are read from.
         */
        VkDescriptorType descriptorType;

        /**
         * \brief This contains information about any images that are being bound as part of the write binding; can be empty or
         * can contain multiple images (which should match up with descriptorCount). \n\n
         *
         * Note - they will be copied into a descriptor array in the order you provide them
         */
        boost::optional<VkDescriptorImageInfo> boundImage = boost::none;

        /**
         * \brief This contains information about any buffers that are being bound as part of the write binding; can be empty or
         * can contain multiple buffers (which should match up with descriptorCount). \n\n
         *
         * Note - they will be copied into a descriptor array in the order you provide them
         */
        boost::optional<VkDescriptorBufferInfo> boundBuffer = boost::none;

        /**
         * \brief This contains information about any texel buffers that are being bound as part of the write binding; can be empty or
         * can contain multiple texel buffers (which should match up with descriptorCount). \n\n
         *
         * Note - they will be copied into a descriptor array in the order you provide them
         */
        boost::optional<VkBufferView> boundTexelBufferView = boost::none;

        WriteBinding(const UniqueIdentifier &resourceId, const uint32_t bindingIndex, const uint32_t descriptorArrayIndex,
                     const uint32_t descriptorCount, const VkDescriptorType descriptorType,
                     const boost::optional<VkDescriptorImageInfo> &boundImage,
                     const boost::optional<VkDescriptorBufferInfo> &boundBuffer,
                     const boost::optional<VkBufferView> &boundTexelBufferView) : DescriptorBinding(resourceId),
            bindingIndex(bindingIndex),
            descriptorArrayIndex(
                descriptorArrayIndex),
            descriptorCount(descriptorCount),
            descriptorType(descriptorType),
            boundImage(boundImage),
            boundBuffer(boundBuffer),
            boundTexelBufferView(
                boundTexelBufferView) {
        }
    };


    struct CopyDescriptorBinding : DescriptorBinding {
        /**
        * \brief If you're copying between an array of descriptors within the descriptor sets,
        * this is the number of descriptors to copy from the source to destination array.
        */
        uint32_t descriptorCount = 0;

        /**
         * \brief This is the binding slot index within the source set that you want to copy to the destination set.
         */
        uint32_t sourceBindingIndex = 0;

        /**
         * \brief If copying between arrays of descriptors, this is the array element to start copying from within
         * the source descriptor set.
         */
        uint32_t sourceArrayElement;

        /**
         * \brief This is the binding slot index within the destination set that you want to copy into.
         */
        uint32_t destinationBindingIndex = 0;

        /**
         * \brief If copying between arrays of descriptors, this is the array element to copy into within the
         * destination set's descriptor array at the specified binding index.
         */
        uint32_t destinationArrayElement = 0;

    protected:
        CopyDescriptorBinding(const UniqueIdentifier uid, const uint32_t descriptorCount,
                              const uint32_t sourceBindingIndex,
                              const uint32_t sourceArrayElement, const uint32_t destinationBindingIndex,
                              const uint32_t destinationArrayElement)
            : DescriptorBinding(uid),
              descriptorCount(descriptorCount),
              sourceBindingIndex(sourceBindingIndex),
              sourceArrayElement(sourceArrayElement),
              destinationBindingIndex(destinationBindingIndex),
              destinationArrayElement(destinationArrayElement) {
        }
    };

    /**
    * \brief In this case, we copy one descriptor to another descriptor, which involves specifying a source
    * and destination; This particular binding corresponds to the case where the descriptor set who is
    * receiving the binding is acting as the *source* of the copy.
    *
    */
    struct SourceCopyBinding final : CopyDescriptorBinding {
        BindingType getBindingType() override {
            return BindingType::COPY_SOURCE;
        }

        /**
         * \brief This is a handle to to the external descriptor set that will receive the copied information
         * from the source descriptor set.
         */
        VkDescriptorSet destinationDescriptorSet = VK_NULL_HANDLE;

        SourceCopyBinding(const UniqueIdentifier uid, const uint32_t descriptorCount,
                          const uint32_t sourceBindingIndex,
                          const uint32_t sourceArrayElement, const uint32_t destinationBindingIndex,
                          const uint32_t destinationArrayElement, VkDescriptorSet destinationDescriptorSet)
            : CopyDescriptorBinding(uid, descriptorCount, sourceBindingIndex, sourceArrayElement,
                                    destinationBindingIndex, destinationArrayElement),
              destinationDescriptorSet(destinationDescriptorSet) {
        }
    };

    /**
    * \brief In this case, we copy one descriptor to another descriptor, which involves specifying a source
    * and destination; This particular binding corresponds to the case where the descriptor set who is
    * receiving the binding is acting as the *destination* of the copy.
    */
    struct DestinationCopyBinding final : CopyDescriptorBinding {
        BindingType getBindingType() override {
            return BindingType::COPY_DESTINATION;
        }

        /**
         * \brief This is a handle to the external source descriptor set that will
         * be used as the copy source
         */
        VkDescriptorSet sourceDescriptorSet = VK_NULL_HANDLE;

        DestinationCopyBinding(const UniqueIdentifier uid, const uint32_t descriptorCount,
                               const uint32_t sourceBindingIndex,
                               const uint32_t sourceArrayElement, const uint32_t destinationBindingIndex,
                               const uint32_t destinationArrayElement, VkDescriptorSet sourceDescriptorSet)
            : CopyDescriptorBinding(uid, descriptorCount, sourceBindingIndex, sourceArrayElement,
                                    destinationBindingIndex, destinationArrayElement),
              sourceDescriptorSet(sourceDescriptorSet) {
        }
    };

    // TODO - add another instance for the case where you just want to specify a general copy between 2 vulkan::backend descriptor set objects
}


