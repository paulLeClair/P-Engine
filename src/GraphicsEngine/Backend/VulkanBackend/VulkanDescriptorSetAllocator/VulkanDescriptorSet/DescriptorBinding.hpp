//
// Created by paull on 2024-02-08.
//

#pragma once

#include <boost/optional/optional.hpp>

#include "../../../../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"

namespace pEngine::girEngine::backend::vulkan::descriptor {
    enum class BindingType {
        UNKNOWN,
        WRITE,
        COPY_SOURCE,
        COPY_DESTINATION
    };

    struct DescriptorBinding {
        DescriptorBinding() = default;

        virtual ~DescriptorBinding() = default;

        virtual BindingType getBindingType() {
            return BindingType::UNKNOWN;
        }

        UniqueIdentifier boundResourceUid;

        explicit DescriptorBinding(const UniqueIdentifier &resource_id)
            : boundResourceUid(resource_id) {
        }

        friend bool operator==(const DescriptorBinding &lhs, const DescriptorBinding &rhs) {
            return lhs.boundResourceUid == rhs.boundResourceUid;
        }

        friend bool operator!=(const DescriptorBinding &lhs, const DescriptorBinding &rhs) {
            return !(lhs == rhs);
        }
    };

    struct WriteBinding final : DescriptorBinding {
        WriteBinding() = default;

        BindingType getBindingType() override {
            return BindingType::WRITE;
        }

        WriteBinding(const UniqueIdentifier &resourceId,
                     const uint32_t bindingIndex,
                     const uint32_t descriptorArrayIndex,
                     const uint32_t descriptorCount,
                     const VkDescriptorType descriptorType,
                     const boost::optional<VkDescriptorImageInfo> &boundImage = boost::none,
                     const boost::optional<VkDescriptorBufferInfo> &boundBuffer = boost::none,
                     const boost::optional<VkBufferView> &boundTexelBufferView = boost::none)
            : DescriptorBinding(resourceId),
              bindingIndex(bindingIndex),
              descriptorArrayIndex(descriptorArrayIndex),
              descriptorCount(descriptorCount),
              descriptorType(descriptorType),
              boundImage(boundImage),
              boundBuffer(boundBuffer),
              boundTexelBufferView(boundTexelBufferView) {
        }

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
         */
        uint32_t descriptorCount = 1;

        /**
         * \brief This is the type of Vulkan descriptor you're binding; eg uniform buffer, sampled image, what have you.
         * See the Vulkan spec for VkDescriptorType for more information \n\n
         */
        VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_MAX_ENUM;

        /**
         * \brief This contains information about any images that are being bound as part of the write binding; can be empty or
         * can contain multiple images (which should match up with descriptorCount). \n\n
         */
        boost::optional<VkDescriptorImageInfo> boundImage = boost::none;

        /**
         * \brief This contains information about any buffers that are being bound as part of the write binding; can be empty or
         * can contain multiple buffers (which should match up with descriptorCount). \n\n
         */
        boost::optional<VkDescriptorBufferInfo> boundBuffer = boost::none;

        /**
         * \brief This contains information about any texel buffers that are being bound as part of the write binding; can be empty or
         * can contain multiple texel buffers (which should match up with descriptorCount). \n\n
         *
         * Note - they will be copied into a descriptor array in the order you provide them
         */
        boost::optional<VkBufferView> boundTexelBufferView = boost::none;
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
}


