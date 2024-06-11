//
// Created by paull on 2022-07-01.
//

#include "../../../../../src/GraphicsEngine/Backend/ApplicationContext/VulkanApplicationContext/VulkanApplicationContext.hpp"
#include "../../../../../src/GraphicsEngine/Backend/VulkanBackend/VulkanShaderModule/VulkanShaderModule.hpp"

#include <gtest/gtest.h>


class VulkanShaderModuleTest : public ::testing::Test {
protected:
    VulkanShaderModuleTest() {
//        testInstance = std::make_shared<VulkanInstance>(VulkanInstance::CreationInput{"test"});
//
//        testPhysicalDevice = std::make_shared<VulkanPhysicalDevice>(
//                VulkanPhysicalDevice::CreationInput{"test", testInstance->getVkInstance()});
//
//        testLogicalDevice = std::make_shared<VulkanLogicalDevice>(
//                VulkanLogicalDevice::CreationInput{"test", testPhysicalDevice->getPhysicalDevice()});
    }

//    std::shared_ptr<VulkanInstance> testInstance;
//    std::shared_ptr<VulkanPhysicalDevice> testPhysicalDevice;
//    std::shared_ptr<VulkanLogicalDevice> testLogicalDevice;

};


TEST_F(VulkanShaderModuleTest, CreateSimpleVertexShader) {
//    PGraphics::VulkanShaderModule::CreationInput createInfo = {};
//    createInfo.device = testLogicalDevice->getLogicalDevice();
//    createInfo.shaderModuleSpirVName = "testVertShader.spv";
//    createInfo.shaderUsage = PGraphics::VulkanShaderModule::ShaderUsage::VERTEX;
//    createInfo.shaderLanguage = PGraphics::VulkanShaderModule::ShaderLanguage::GLSL;
//    ASSERT_NO_THROW(PGraphics::VulkanShaderModule testVertModule(createInfo));
}

TEST_F(VulkanShaderModuleTest, CreateSimpleFragmentShader) {
//    PGraphics::VulkanShaderModule::CreationInput createInfo = {};
//    createInfo.device = testLogicalDevice->getLogicalDevice();
//    createInfo.shaderModuleSpirVName = "testFragShader.spv";
//    createInfo.shaderUsage = PGraphics::VulkanShaderModule::ShaderUsage::FRAGMENT;
//    createInfo.shaderLanguage = PGraphics::VulkanShaderModule::ShaderLanguage::GLSL;
//    ASSERT_NO_THROW(PGraphics::VulkanShaderModule testFragModule(createInfo));
}
