#pragma once

#include "../PEngineCore.hpp"
#include "./PEngineMode.hpp"

#include <string>

class RenderGraph;

// default mode
class CoreMenuMode : public PEngineMode {
  public:
    CoreMenuMode(const std::string &name, std::shared_ptr<PEngine> corePtr);
    ~CoreMenuMode();

    void enterMode() override;

  private:

    std::shared_ptr<RenderGraph> coreMenuGraph_ = nullptr;

    void mainLoop() override;

    // void exitMode() override;

    // core menu functions
    void drawCoreMenu();

};