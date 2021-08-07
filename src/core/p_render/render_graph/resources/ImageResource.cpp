#include "../../../../../include/core/p_render/render_graph/resources/ImageResource.hpp"

ImageResource::ImageResource(unsigned int index, const std::string &name, const AttachmentInfo &info) : RenderResource(RenderResource::Type::Image, index, name) {
    _attachmentInfo = info;
}