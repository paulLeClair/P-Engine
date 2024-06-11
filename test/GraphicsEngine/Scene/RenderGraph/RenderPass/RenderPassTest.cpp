//
// Created by paull on 2023-10-27.
//

#include <gtest/gtest.h>

#include "../../../../../src/GraphicsEngine/Scene/RenderGraph/RenderPass/RenderPass.hpp"
#include "../../../../../src/GraphicsEngine/Scene/SceneResources/Buffer/UniformBuffer/UniformBuffer.hpp"
#include "../../../../../src/GraphicsEngine/Scene/SceneResources/Buffer/StorageBuffer/StorageBuffer.hpp"
#include "../../../../../src/GraphicsEngine/Scene/SceneResources/Buffer/TexelBuffer/TexelBuffer.hpp"
#include "../../../../../src/GraphicsEngine/Scene/SceneResources/Renderable/TriangleMesh/TriangleMesh.hpp"

static const char *const TEST_VERTEX_SHADER_SPV_FILENAME = "testVertShader";

static const char *const TEST_FRAGMENT_SHADER_SPV_FILENAME = "testFragShader";

using namespace pEngine::girEngine::scene;

class RenderPassTest : public ::testing::Test {
protected:
    void SetUp() override {
        testVertexShaderModule = std::make_shared<ShaderModule>(
                ShaderModule::CreationInput{
                        "emptyTestVertexShader",
                        pEngine::util::UniqueIdentifier(),
                        "main",
                        TEST_VERTEX_SHADER_SPV_FILENAME,
                        ShaderModule::ShaderUsage::VERTEX_SHADER,
                        ShaderModule::ShaderLanguage::GLSL
                });

        testFragmentShaderModule = std::make_shared<ShaderModule>(
                ShaderModule::CreationInput{
                        "emptyTestFragmentShader",
                        pEngine::util::UniqueIdentifier(),
                        "main",
                        TEST_FRAGMENT_SHADER_SPV_FILENAME,
                        ShaderModule::ShaderUsage::FRAGMENT_SHADER,
                        ShaderModule::ShaderLanguage::GLSL
                });
    }

    void setUpBakeableRenderPass() {
//        ASSERT_NO_THROW(renderPass = std::make_shared<graph::renderPass::RenderPass>(
//                graph::renderPass::RenderPass::CreationInput{
//                        "test",
//                        pEngine::util::UniqueIdentifier(),
//                        RenderPass::Subtype::DYNAMIC_RENDER_PASS,
//                        {} // blank to use defaults
//                })
//        );

        // add simple vertex and fragment shader attachments
        ASSERT_NO_THROW(
                renderPass->setVertexShaderModule(graph::renderPass::ShaderAttachment{testVertexShaderModule})
        );
        ASSERT_NO_THROW(
                renderPass->setFragmentShaderModule(
                        graph::renderPass::ShaderAttachment{testFragmentShaderModule})
        );
    }

    static void
    renderPassBakeTest(const std::shared_ptr<RenderPass> &renderPass,
                       const std::vector<std::shared_ptr<pEngine::girEngine::gir::GraphicsIntermediateRepresentation> >
                       &sceneResources,
                       const std::vector<std::shared_ptr<pEngine::girEngine::gir::GraphicsIntermediateRepresentation> >
                       &sceneShaderModules) {
        // bake should succeed or at least not throw
        ASSERT_NO_THROW(
                renderPass->bakeToGIR(sceneResources, sceneShaderModules)
        );
    }

    std::shared_ptr<graph::renderPass::RenderPass> renderPass = nullptr;

    // test shader module data
    std::shared_ptr<ShaderModule> testVertexShaderModule = nullptr;

    std::shared_ptr<ShaderModule> testFragmentShaderModule = nullptr;

    // test image data
    const std::shared_ptr<Image> testImageForColorAttachment = std::make_shared<Image>(
            Image::CreationInput{
                    "testColorAttachment",
                    pEngine::util::UniqueIdentifier(),
                    TexelFormat::R8G8B8A8_SRGB, // arbitrary format
                    Image::ImageExtent2D{10, 10}, // arbitrary extent
                    Image::MipMapConfiguration{}, // defaults
                    Image::ImageArrayConfiguration{}, // defaults
                    Image::TexelTilingArrangement{}, // defaults
                    1, // sample once
                    Image::ImageUsage::ColorAttachment, // arbitrary
                    nullptr, // no initialization data
                    0 // no initialization data
            });
    const std::shared_ptr<Image> testImageForInputAttachment = std::make_shared<Image>(
            Image::CreationInput{
                    "testInputAttachment",
                    pEngine::util::UniqueIdentifier(),
                    TexelFormat::R8G8B8A8_SRGB, // arbitrary format
                    Image::ImageExtent2D{10, 10}, // arbitrary extent
                    Image::MipMapConfiguration{}, // defaults
                    Image::ImageArrayConfiguration{}, // defaults
                    Image::TexelTilingArrangement{}, // defaults
                    1, // sample once
                    Image::ImageUsage::InputAttachment,
                    nullptr, // no initialization data
                    0 // no initialization data
            });

    const std::shared_ptr<Image> testDepthStencilImageForAttachment = std::make_shared<Image>(
            Image::CreationInput{
                    "testDepthStencilAttachment",
                    pEngine::util::UniqueIdentifier(),
                    TexelFormat::R8G8B8A8_SRGB, // arbitrary format
                    Image::ImageExtent2D{10, 10}, // arbitrary extent
                    Image::MipMapConfiguration{}, // defaults
                    Image::ImageArrayConfiguration{}, // defaults
                    Image::TexelTilingArrangement{}, // defaults
                    1, // sample once
                    Image::ImageUsage::DepthStencilAttachment,
                    nullptr, // no initialization data
                    0 // no initialization data
            });

    const std::shared_ptr<Image> testStorageImageForAttachment = std::make_shared<Image>(
            Image::CreationInput{
                    "testStorageAttachment",
                    pEngine::util::UniqueIdentifier(),
                    TexelFormat::R8G8B8A8_SRGB, // arbitrary format
                    Image::ImageExtent2D{10, 10}, // arbitrary extent
                    Image::MipMapConfiguration{}, // defaults
                    Image::ImageArrayConfiguration{}, // defaults
                    Image::TexelTilingArrangement{}, // defaults
                    1, // sample once
                    Image::ImageUsage::Storage,
                    nullptr, // no initialization data
                    0 // no initialization data
            });

    // test texture data
    const std::shared_ptr<Image> testSampledImageForTexture = std::make_shared<Image>(
            Image::CreationInput{
                    "testTextureSampledImage",
                    pEngine::util::UniqueIdentifier(),
                    TexelFormat::R8G8B8A8_SRGB, // arbitrary format
                    Image::ImageExtent2D{10, 10}, // arbitrary extent
                    Image::MipMapConfiguration{}, // defaults
                    Image::ImageArrayConfiguration{}, // defaults
                    Image::TexelTilingArrangement{}, // defaults
                    1, // sample once
                    Image::ImageUsage::SampledTextureImage,
                    nullptr, // no initialization data
                    0 // no initialization data
            });

    const std::shared_ptr<Texture> testTexture = std::make_shared<Texture>(Texture::CreationInput{
            "testTexture",
            pEngine::util::UniqueIdentifier(),
            {}, // blank/default sampler settings
            testSampledImageForTexture
    });

    // test buffer data
    const std::shared_ptr<UniformBuffer> testUniformBuffer = std::make_shared<UniformBuffer>(
            UniformBuffer::CreationInput{
                    "testUniformBuffer",
                    pEngine::util::UniqueIdentifier(),
                    Buffer::BufferSubtype::UNIFORM_BUFFER,
                    nullptr,
                    0
            }
    );

    const std::shared_ptr<StorageBuffer> testStorageBuffer = std::make_shared<StorageBuffer>(
            StorageBuffer::CreationInput{
                    "testStorageBuffer",
                    pEngine::util::UniqueIdentifier(),
                    Buffer::BufferSubtype::STORAGE_BUFFER,
                    nullptr,
                    0
            }
    );

    // TODO - storage texel buffers

    const std::shared_ptr<TexelBuffer> testTexelBuffer = std::make_shared<TexelBuffer>(
            TexelBuffer::CreationInput{
                    "testTexelBuffer",
                    pEngine::util::UniqueIdentifier(),
                    Buffer::BufferSubtype::TEXEL_BUFFER,
                    nullptr,
                    0
            }
    );

    // test shader constant data
    const std::shared_ptr<ShaderConstant> testShaderConstant = std::make_shared<ShaderConstant>(
            ShaderConstant::CreationInput{
                    "testShaderConstant",
                    pEngine::util::UniqueIdentifier(),
                    {ShaderConstant::ShaderStage::FRAGMENT}, // arbitrary stages for test
                    0, // no offset
                    0, // empty so size is zero
                    nullptr, // no init data
                    0 // no init data
            }
    );

    // dummy geometry data
    const std::vector<unsigned int> testIndices = {0};
    const std::vector<unsigned char> testVertices = {0};

    const std::shared_ptr<IndexBuffer<unsigned int> > testRenderableIndexBuffer
            = std::make_shared<IndexBuffer<unsigned int> >(IndexBuffer<unsigned int>::CreationInput{
                    "testRenderableIndexBuffer",
                    pEngine::util::UniqueIdentifier(),
                    Buffer::BufferSubtype::INDEX_BUFFER,
                    testIndices
            });

    const std::shared_ptr<VertexBuffer> testRenderableVertexBuffer = std::make_shared<VertexBuffer
    >(
            VertexBuffer::CreationInput{
                    "testRenderableVertexBuffer",
                    pEngine::util::UniqueIdentifier(),
                    Buffer::BufferSubtype::VERTEX_BUFFER,
                    testVertices
            }
    );

    const std::shared_ptr<TriangleMesh> testRenderable = std::make_shared<TriangleMesh>(
            TriangleMesh::CreationInput{
                    "testTriangleMeshRenderable",
                    pEngine::util::UniqueIdentifier(),
                    Renderable::RenderableType::TRIANGLE_MESH,
                    {testRenderableVertexBuffer},
                    {testRenderableIndexBuffer}
            });
};

TEST_F(RenderPassTest, CreateEmptyRenderPass) {
//    ASSERT_NO_THROW(renderPass = std::make_shared<graph::renderPass::RenderPass>(
//            graph::renderPass::RenderPass::CreationInput{
//                    "test",
//                    pEngine::util::UniqueIdentifier(),
//                    RenderPass::Subtype::DYNAMIC_RENDER_PASS,
//                    {} // blank to use defaults
//            })
//    );
//
    // bake should fail because of missing required information
    ASSERT_ANY_THROW(renderPass->bakeToGIR({}, {}));
}

TEST_F(RenderPassTest, CreateRenderPassAndAddImageAttachments) {
    setUpBakeableRenderPass();

    // add image attachments
    ASSERT_TRUE(
            renderPass->addColorAttachment(graph::renderPass::ImageAttachment{testImageForColorAttachment}));
    ASSERT_TRUE(
            renderPass->addInputAttachment(graph::renderPass::ImageAttachment{testImageForInputAttachment})
    );
    ASSERT_TRUE(renderPass->addDepthStencilAttachment(
            graph::renderPass::ImageAttachment{testDepthStencilImageForAttachment})
    );
    ASSERT_TRUE(renderPass->addStorageImageAttachment(
            graph::renderPass::ImageAttachment{testStorageImageForAttachment})
    );

    // bake render pass
    std::vector<std::shared_ptr<pEngine::girEngine::gir::GraphicsIntermediateRepresentation> > sceneResources = {
            testImageForColorAttachment->bakeToGIR(),
            testImageForInputAttachment->bakeToGIR(),
            testDepthStencilImageForAttachment->bakeToGIR(),
            testStorageImageForAttachment->bakeToGIR()
    };
    std::vector<std::shared_ptr<pEngine::girEngine::gir::GraphicsIntermediateRepresentation> > sceneShaderModules = {
            testVertexShaderModule->bakeToGIR(),
            testFragmentShaderModule->bakeToGIR()
    };

    renderPassBakeTest(renderPass, sceneResources, sceneShaderModules);
}

TEST_F(RenderPassTest, CreateBakeableRenderPassAndAddSimpleTextureAttachments) {
    setUpBakeableRenderPass();

    // add texture attachments
    ASSERT_TRUE(
            renderPass->addTextureAttachment(graph::renderPass::TextureAttachment{
                    testTexture,
                    {graph::renderPass::TextureAttachment::ShaderStage::FRAGMENT}
            })
    );

    // bake render pass
    std::vector<std::shared_ptr<pEngine::girEngine::gir::GraphicsIntermediateRepresentation> > sceneResources = {
            testTexture->bakeToGIR()
    };
    std::vector<std::shared_ptr<pEngine::girEngine::gir::GraphicsIntermediateRepresentation> > sceneShaderModules = {
            testVertexShaderModule->bakeToGIR(),
            testFragmentShaderModule->bakeToGIR()
    };

    // bake should succeed (or at least not throw)
    renderPassBakeTest(renderPass, sceneResources, sceneShaderModules);
}

TEST_F(RenderPassTest, CreateBakeableRenderPassAndAddBufferAttachments) {
    setUpBakeableRenderPass();

    // add buffer attachments
    ASSERT_TRUE(
            renderPass->addUniformBufferAttachment(graph::renderPass::BufferAttachment{testUniformBuffer}));
    ASSERT_TRUE(
            renderPass->addStorageBufferAttachment(graph::renderPass::BufferAttachment{testStorageBuffer})
    );
    // TODO - storage texel buffers

    ASSERT_TRUE(renderPass->addTexelBufferAttachment(
            graph::renderPass::BufferAttachment{testTexelBuffer})
    );

    // bake render pass
    std::vector<std::shared_ptr<pEngine::girEngine::gir::GraphicsIntermediateRepresentation> > sceneResources = {
            testUniformBuffer->bakeToGIR(),
            testStorageBuffer->bakeToGIR(),
            testTexelBuffer->bakeToGIR()
    };
    std::vector<std::shared_ptr<pEngine::girEngine::gir::GraphicsIntermediateRepresentation> > sceneShaderModules = {
            testVertexShaderModule->bakeToGIR(),
            testFragmentShaderModule->bakeToGIR()
    };

    // bake should succeed
    renderPassBakeTest(renderPass, sceneResources, sceneShaderModules);
}

TEST_F(RenderPassTest, CreateBakeableRenderPassAndAddShaderConstantAttachment) {
    setUpBakeableRenderPass();

    // add shader constant attachments
    ASSERT_TRUE(
            renderPass->addShaderConstantAttachment(graph::renderPass::ShaderConstantAttachment{
                    testShaderConstant
            })
    );

    // bake render pass
    std::vector<std::shared_ptr<pEngine::girEngine::gir::GraphicsIntermediateRepresentation> > sceneResources = {
            testShaderConstant->bakeToGIR()
    };
    std::vector<std::shared_ptr<pEngine::girEngine::gir::GraphicsIntermediateRepresentation> > sceneShaderModules = {
            testVertexShaderModule->bakeToGIR(),
            testFragmentShaderModule->bakeToGIR()
    };

    // bake should succeed
    renderPassBakeTest(renderPass, sceneResources, sceneShaderModules);
}

TEST_F(RenderPassTest, CreateBakeableRenderPassAndAddSimpleGeometryAttachments) {
    setUpBakeableRenderPass();

    // add some geometry attachments
//    ASSERT_TRUE(
//            renderPass->addGeometryAttachment(
//                    DrawAttachment{
//                            DrawAttachment::DrawType::INDEXED_DRAW,
//                            {testRenderable}
//                    })
//    );

    // bake render pass
    std::vector<std::shared_ptr<pEngine::girEngine::gir::GraphicsIntermediateRepresentation> > sceneResources = {
            testRenderableIndexBuffer->bakeToGIR(),
            testRenderableVertexBuffer->bakeToGIR()
    };
    std::vector<std::shared_ptr<pEngine::girEngine::gir::GraphicsIntermediateRepresentation> > sceneShaderModules = {
            testVertexShaderModule->bakeToGIR(),
            testFragmentShaderModule->bakeToGIR()
    };
    renderPassBakeTest(renderPass, sceneResources, sceneShaderModules);
}

// TODO - add more complicated tests and edge cases
