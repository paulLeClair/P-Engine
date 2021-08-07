#pragma once

#include "../../PRender.hpp"

#include <unordered_set>

class RenderResource {
  public:
    virtual ~RenderResource() = default;

    enum class Type {
        Buffer,
        Image
    };

    // get resource type 
    Type getType() {
        return _type;
    }

    // get index in render graph 
    const unsigned int &getIndex() const {
        return _index;
    }

    const unsigned int getPhysicalIndex() const {
        return _physicalIndex;
    }

    const std::string &getResourceName() const {
        return _name;
    }

    // set resource name
    void setResourceName(const std::string &name) {
        _name = name;
    }

    // physical resource functions
        // these are used when creating physical resources in the Vulkan API during render graph bake()
    void setPhysicalIndex(unsigned int index) {
        _physicalIndex = index;
    }
    
    // resource queue usages 
    void addResourceQueueUsage(ResourceQueueUsageFlags queue) {
        _usages = _usages | (uint32_t)queue;
    }

    ResourceQueueUsageFlags getResourceQueueUsages() {
        return _usages;
    }

    // register passes that write/read this resource
    void addWritePass(unsigned int writtenInPassIndex);
    void addReadPass(unsigned int readInPassIndex);

    // get indices (in render graph) of passes that write/read this resource respectively
    std::unordered_set<unsigned int> &getWritePasses();
    std::unordered_set<unsigned int> &getReadPasses();

  protected:
    // RenderResource(Type type, unsigned int index);
    RenderResource(Type type, unsigned int index, const std::string &name = "");

    Type _type;
    
    // Resource index in the render graph 
    unsigned int _index; 

    // index in the array of physical resources that will be managed by the render graph
    unsigned int _physicalIndex = ~0u;

    // resource name 
    std::string _name;

    // queue type usages for the resource
    ResourceQueueUsageFlags _usages;

    std::unordered_set<unsigned int> _writePasses; // passes the resource is written in
    std::unordered_set<unsigned int> _readPasses; // passes the resource is read in

};