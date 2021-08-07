#pragma once

#include <string>
#include <memory>

class PEngine;

class PEngineMode {
  public:
    PEngineMode(const std::string &name, std::shared_ptr<PEngine> corePtr);
    virtual ~PEngineMode() = default;

    // i guess i'll just have engine modes be enterable?
    virtual void enterMode() = 0;

  protected:
    virtual void mainLoop() = 0;
    virtual void exitMode(const std::string &nextModeName); // honestly this could be implemented to just call enterMode() on a given nextModeName or whatever, doesn't need to be specific to each mode prob
        // since it's just supposed to transition into a new mode from the current mode 

    /* mode core state */
    std::string _name;

    // hmm... need to access the engine core state
    std::shared_ptr<PEngine> _core;

};