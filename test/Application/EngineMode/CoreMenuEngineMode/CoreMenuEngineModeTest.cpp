//
// Created by paull on 2022-09-18.
//

#include "gtest/gtest.h"
#include "../../../../src/Application/EngineMode/CoreMenuEngineMode/CoreMenuEngineMode.hpp"
#include "../../../../src/GraphicsEngine/Backend/VulkanBackend/VulkanBackend.hpp"

using namespace pEngine::app::mode;

class CoreMenuEngineModeTest : public ::testing::Test {
protected:
    void SetUp() override {
//        constexpr uint32_t TEST_VULKAN_API_VERSION = VK_MAKE_API_VERSION(0, 1, 3, 0);
//        // idk what the "variant" arg is supposed to be
//        constexpr uint32_t TEST_APP_ENGINE_VERSION = VK_MAKE_VERSION(0, 1, 0);
//
//        testThreadPool = std::make_shared<BS::thread_pool>(1); // single thread for now
//
//        testScene = std::make_shared<pEngine::girEngine::scene::Scene>(
//                pEngine::girEngine::scene::Scene::CreationInput{
//                        "testScene",
//                        "renderGraphLabel"
//                });
//
//        testBackend = std::make_shared<pEngine::girEngine::backend::vulkan::VulkanBackend>(
//                pEngine::girEngine::backend::vulkan::VulkanBackend::CreationInput{
//                        "pEngineApp",
//                        "girEngine",
//                        testThreadPool,
//                        {
//                                pEngine::girEngine::backend::appContext::vulkan::VulkanInstance::SupportedInstanceExtension::SURFACE_EXTENSION
//                        },
//                        {pEngine::girEngine::backend::appContext::vulkan::VulkanInstance::SupportedLayers::VALIDATION_LAYER},
//                        "SingularPhysicalDevice",
//                        "SingularLogicalDevice",
//                        TEST_APP_ENGINE_VERSION,
//                        TEST_APP_ENGINE_VERSION,
//                        TEST_VULKAN_API_VERSION
//                }
//        );
//
//        testCore = std::make_shared<pEngine::core::EngineCore<scene::Scene, backend::vulkan::VulkanBackend> >(
//                pEngine::core::EngineCore<scene::Scene, backend::vulkan::VulkanBackend>::CreationInput(
//                        "testCore",
//                        pEngine::util::UniqueIdentifier(),
//                        0, // for default
//                        testScene,
//                        testBackend
//                ));
    }

    std::shared_ptr<BS::thread_pool> testThreadPool = nullptr;

    std::shared_ptr<pEngine::girEngine::scene::Scene> testScene = nullptr;

    std::shared_ptr<backend::vulkan::VulkanBackend> testBackend = nullptr;

    std::shared_ptr<pEngine::core::EngineCore<scene::Scene, backend::vulkan::VulkanBackend> > testCore = nullptr;

    std::shared_ptr<pEngine::app::mode::CoreMenuEngineMode<
            pEngine::girEngine::scene::Scene,
            pEngine::girEngine::backend::vulkan::VulkanBackend>
    > coreMenuMode = nullptr;
};

TEST_F(CoreMenuEngineModeTest, BasicCreationWithoutBypassFlagOrBake) {
//    std::string testModeName = "coreMenuEngineMode";
//    const UniqueIdentifier &uniqueIdentifier = pEngine::util::UniqueIdentifier();
//    coreMenuMode = std::make_shared<
//            pEngine::app::mode::CoreMenuEngineMode<scene::Scene, backend::vulkan::VulkanBackend> >(
//            pEngine::app::mode::CoreMenuEngineMode<scene::Scene, backend::vulkan::VulkanBackend>::CreationInput{
//                    testModeName,
//                    uniqueIdentifier,
//                    testCore,
//                    nullptr, // no next engine mode
//                    {} // no imgui callbacks
//                    // leave flag to skip main loop unchecked
//            });
//    ASSERT_EQ(testModeName, coreMenuMode->getName());
//    ASSERT_EQ(uniqueIdentifier, coreMenuMode->getUid());
}

TEST_F(CoreMenuEngineModeTest, CreateAndEnterModeWithBypassFlagActivated) {
    // since we don't want to be entering into any update->render loops while testing,
    // i think i should probably add a flag that makes the core menu skip the main loop
//    std::string testModeName = "coreMenuEngineMode";
//    const UniqueIdentifier &uniqueIdentifier = pEngine::util::UniqueIdentifier();
//    coreMenuMode = std::make_shared<
//            pEngine::app::mode::CoreMenuEngineMode<scene::Scene, backend::vulkan::VulkanBackend> >(
//            pEngine::app::mode::CoreMenuEngineMode<scene::Scene, backend::vulkan::VulkanBackend>::CreationInput{
//                    testModeName,
//                    uniqueIdentifier,
//                    testCore,
//                    nullptr, // no next engine mode
//                    {}, // no imgui callbacks
//                    true // activate bypass of main loop -> mode will transition straight to finish after run()
//            });
//    ASSERT_EQ(testModeName, coreMenuMode->getName());
//    ASSERT_EQ(uniqueIdentifier, coreMenuMode->getUid());

    // assert that the engine mode can complete (while also skipping the mode's main loop)
//    ASSERT_NO_THROW(coreMenuMode->enter());
}

// TODO - test scene bake output in more detail
