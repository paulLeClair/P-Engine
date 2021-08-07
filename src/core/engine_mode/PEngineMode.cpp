#include "../../../include/core/engine_mode/PEngineMode.hpp"

#include "../../../include/core/PEngineCore.hpp"

PEngineMode::PEngineMode(const std::string &name, std::shared_ptr<PEngine> core) : _name(name), _core(core) {

}

void PEngineMode::exitMode(const std::string &nextModeName) {
    _core->enterMode(nextModeName);
}