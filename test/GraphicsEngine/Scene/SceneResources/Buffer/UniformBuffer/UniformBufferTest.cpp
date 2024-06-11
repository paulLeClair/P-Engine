#include <gtest/gtest.h>
#include "../../../../../../src/GraphicsEngine/Scene/SceneResources/Buffer/UniformBuffer/UniformBuffer.hpp"

using namespace pEngine::girEngine::scene;

class UniformBufferTest : public ::testing::Test {
protected:
    struct UniformBufferType {
        glm::vec3 vec3;
        glm::vec2 vec2;

        bool operator==(const UniformBufferType &rhs) const {
            return vec3 == rhs.vec3 &&
                   vec2 == rhs.vec2;
        }

        bool operator!=(const UniformBufferType &rhs) const {
            return !(rhs == *this);
        }
    };

    struct AlternativeUniformBufferType {
        glm::mat4 mat4;

        struct InnerStruct {
            glm::vec3 innerVec3;
            glm::vec2 innerVec2;

            InnerStruct(const glm::vec3 &innerVec3, const glm::vec2 &innerVec2) : innerVec3(innerVec3),
                                                                                  innerVec2(innerVec2) {}

            bool operator==(const InnerStruct &rhs) const {
                return innerVec3 == rhs.innerVec3 &&
                       innerVec2 == rhs.innerVec2;
            }

            bool operator!=(const InnerStruct &rhs) const {
                return !(rhs == *this);
            }
        };

        InnerStruct innerStruct;

        AlternativeUniformBufferType(const glm::mat4 &mat4, const InnerStruct &innerStruct) : mat4(mat4), innerStruct(
                innerStruct) {}

        bool operator==(const AlternativeUniformBufferType &rhs) const {
            return mat4 == rhs.mat4 &&
                   innerStruct == rhs.innerStruct;
        }

        bool operator!=(const AlternativeUniformBufferType &rhs) const {
            return !(rhs == *this);
        }
    };

    void SetUp() override {

    }

};

TEST_F(UniformBufferTest, BasicCreation) {
    static const UniformBufferType TEST_DATA = {
            glm::vec3(0.0),
            glm::vec2(1.0)
    };

    auto buffer = std::make_shared<UniformBuffer>(UniformBuffer::CreationInput{
            "test",
            pEngine::util::UniqueIdentifier(),
            Buffer::BufferSubtype::UNIFORM_BUFFER,
            (unsigned char *) &TEST_DATA,
            sizeof(UniformBufferType)
    });

    ASSERT_TRUE(*buffer->getBufferDataAsPointer<UniformBufferType>() == TEST_DATA);
}

TEST_F(UniformBufferTest, CreateAndModifyData) {
    static const UniformBufferType TEST_DATA = {
            glm::vec3(1.0),
            glm::vec2(2.0)
    };

    static const AlternativeUniformBufferType ALT_TEST_DATA = {
            glm::mat4(1.0),
            AlternativeUniformBufferType::InnerStruct(glm::vec3(1.0), glm::vec2(2.0))
    };

    auto buffer = std::make_shared<UniformBuffer>(UniformBuffer::CreationInput{
            "test",
            pEngine::util::UniqueIdentifier(),
            Buffer::BufferSubtype::UNIFORM_BUFFER,
            (unsigned char *) &TEST_DATA,
            sizeof(UniformBufferType)
    });

    ASSERT_EQ(*buffer->getBufferDataAsPointer<UniformBufferType>(), TEST_DATA);
    ASSERT_EQ(buffer->getSizeInBytes(), sizeof(UniformBufferType));

    ASSERT_NO_THROW(
            buffer->setBufferDataFromPointer<AlternativeUniformBufferType>(&ALT_TEST_DATA,
                                                                           sizeof(AlternativeUniformBufferType))
    );
    ASSERT_EQ(*buffer->getBufferDataAsPointer<AlternativeUniformBufferType>(), ALT_TEST_DATA);
    ASSERT_EQ(buffer->getSizeInBytes(), sizeof(AlternativeUniformBufferType));
}