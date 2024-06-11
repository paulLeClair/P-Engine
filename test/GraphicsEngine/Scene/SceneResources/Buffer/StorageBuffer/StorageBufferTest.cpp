#include <gtest/gtest.h>
#include "../../../../../../src/GraphicsEngine/Scene/SceneResources/Buffer/StorageBuffer/StorageBuffer.hpp"

using namespace pEngine::girEngine::scene;

class StorageBufferTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    std::shared_ptr<Buffer> buffer;
};

TEST_F(StorageBufferTest, BasicCreation) {
    ASSERT_NO_THROW(buffer = std::make_shared<StorageBuffer>(StorageBuffer::CreationInput{
            "testStorageBuffer",
            pEngine::util::UniqueIdentifier(),
            Buffer::BufferSubtype::STORAGE_BUFFER,
            nullptr,
            0
    }));
}

// TODO - add some more tests