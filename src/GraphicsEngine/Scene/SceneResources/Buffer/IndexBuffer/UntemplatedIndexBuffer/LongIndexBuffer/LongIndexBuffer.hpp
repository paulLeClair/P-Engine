//
// Created by paull on 2023-04-02.
//

#pragma once


#include "../UntemplatedIndexBuffer.hpp"

class LongIndexBuffer : public UntemplatedIndexBuffer {
public:
    struct CreationInput {
        std::shared_ptr<Scene> parentScene;

        std::string name;
        PUtilities::UniqueIdentifier uniqueIdentifier;

        std::function<void(const Buffer &)> updateCallback;
    };

    explicit LongIndexBuffer(const CreationInput &creationInput);

    ~LongIndexBuffer() override = default;
};
