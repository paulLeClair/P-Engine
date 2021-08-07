#pragma once

// this will be the generic physical resource class

namespace Backend {

class Resource {
  public:
    
    // gonna try using these 
    virtual bool isImage() const {
        return false;
    }

    virtual bool isBuffer() const {
        return false;
    }

    virtual bool isSwapchainImage() const {
      return false;
    }

  protected:
    Resource() {
      //pass
    }
    ~Resource() = default;

    // i don't think the base resource class needs to do much

};

}