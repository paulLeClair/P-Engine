//
// Created by paull on 2023-04-02.
//

#pragma once


#include "../UntemplatedIndexBuffer.hpp"

class CharIndexBufferTest : public UntemplatedIndexBuffer {
public:
    struct CreationInput {
        std::shared_ptr<Scene> parentScene;

        std::string name;
        PUtilities::UniqueIdentifier uniqueIdentifier;

        std::function<void(const Buffer &)> updateCallback;
    };

    explicit CharIndexBufferTest(const CreationInput &creationInput);

    ~CharIndexBufferTest() override = default;
};
