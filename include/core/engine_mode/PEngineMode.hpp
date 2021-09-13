#pragma once

#include <string>
#include <memory>

class PEngine;

class PEngineMode {
  public:
    PEngineMode(const std::string &name, std::shared_ptr<PEngine> corePtr);
    virtual ~PEngineMode() = default;

    virtual void enterMode() = 0;

  protected:
    virtual void mainLoop() = 0;
    virtual void exitMode(const std::string &nextModeName);

    /* mode core state */
    std::string _name;

    std::shared_ptr<PEngine> _core;

};