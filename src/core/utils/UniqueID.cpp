#include "../../../include/core/utils/UniqueID.hpp"

// initialize the static ID variable to 0
UniqueID::uid_t UniqueID::nextID = 0;

UniqueID::UniqueID() {
    id_ = nextID++; // set new ID and increment the static ID var
}


UniqueID::UniqueID(const UniqueID &original) {
    // copy constructor i think
    id_ = original.id_;
}

UniqueID &UniqueID::operator=(const UniqueID &original) {
    id_ = original.id_;
    return *this;
}
