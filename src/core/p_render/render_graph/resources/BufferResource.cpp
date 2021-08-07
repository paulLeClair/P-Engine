#include "../../../../../include/core/p_render/render_graph/resources/BufferResource.hpp"

BufferResource::BufferResource(unsigned int index, const std::string &name, const BufferInfo &info) : RenderResource(RenderResource::Type::Buffer, index, name) {
    bufferInfo_ = info;
}