#include "../../../../../include/core/p_render/render_graph/resources/RenderResource.hpp"

RenderResource::RenderResource(RenderResource::Type type, unsigned int index, const std::string &name) : _type(type), _index(index), _name(name) {

}

void RenderResource::addWritePass(unsigned int index) {
    _writePasses.insert(index);
}

void RenderResource::addReadPass(unsigned int index) {
    _readPasses.insert(index);
}

std::unordered_set<unsigned int> &RenderResource::getWritePasses() {
    return _writePasses;
}

std::unordered_set<unsigned int> &RenderResource::getReadPasses() {
    return _readPasses;
}

