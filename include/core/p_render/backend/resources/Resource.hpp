#pragma once

// this will be the generic graphics-backend resource class

namespace backend {

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
      
    }
    ~Resource() = default;

    // i don't think the base resource class needs to do much

};

}