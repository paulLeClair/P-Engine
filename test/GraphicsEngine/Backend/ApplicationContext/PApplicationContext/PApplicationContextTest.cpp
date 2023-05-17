#include "../../../../../src/GraphicsEngine/Backend/ApplicationContext/PApplicationContext/PApplicationContext.hpp"
#include <gtest/gtest.h>

using namespace PGraphics;

class PApplicationContextTest : public ::testing::Test {
protected:
    void SetUp() override {
        test = std::make_shared<PApplicationContext>();
    }

    std::shared_ptr<PApplicationContext> test;
};

TEST_F(PApplicationContextTest, CreateApplicationContext) {
    ASSERT_TRUE(test.get() != nullptr);
    ASSERT_TRUE(test->getInstance() != VK_NULL_HANDLE);
    ASSERT_TRUE(test->getLogicalDevice() != VK_NULL_HANDLE);
    ASSERT_TRUE(test->getPhysicalDevice() != VK_NULL_HANDLE);
    ASSERT_TRUE(test->getGraphicsQueue() != VK_NULL_HANDLE);
    ASSERT_TRUE(test->getWindowSystem().get() != nullptr);
}

// TODO