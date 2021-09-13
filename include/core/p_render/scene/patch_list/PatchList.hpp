#pragma once

#include "../Renderable.hpp"

// TODO

namespace scene {

struct PatchListCreateInfo {

};

class PatchList : public Renderable { 
  public:
    PatchList(const PatchListCreateInfo &info);
    ~PatchList();


    bool isPatchList() override;

};

}