#include <gtest/gtest.h>
#include "../../../../../../src/GraphicsEngine/Scene/SceneResources/Buffer/TexelBuffer/TexelBuffer.hpp"

using namespace pEngine::girEngine::scene;

class TexelBufferTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    std::shared_ptr<Buffer> buffer;
};

TEST_F(TexelBufferTest, BasicCreation) {
    ASSERT_NO_THROW(buffer = std::make_shared<TexelBuffer>(TexelBuffer::CreationInput{
            "test texel buffer",
            pEngine::util::UniqueIdentifier(),
            Buffer::BufferSubtype::TEXEL_BUFFER,
            nullptr,
            0,
            {} // default sampler settings
    }));

}
