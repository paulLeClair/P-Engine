#pragma once

#include "../../EngineCore/utilities/UniqueIdentifier/UniqueIdentifier.hpp"
#include "SceneResources/Texture/Texture.hpp"
#include "SceneResources/Material/Material.hpp"
#include "SceneResources/Renderable/Renderable.hpp"
#include "SceneResources/ShaderConstant/ShaderConstant.hpp"

#include <unordered_set>

class SceneResource;

class Model;

class Buffer;

class Image;

class SceneRenderGraph;

class ShaderModule;

class Scene {
public:
    ~Scene() = default;

    virtual void registerModel(const std::shared_ptr<Model> &model) = 0;

    [[nodiscard]] virtual std::shared_ptr<Model> getModel(const std::string &name) const = 0;

    [[nodiscard]] virtual std::shared_ptr<Model> getModel(const PUtilities::UniqueIdentifier &name) const = 0;

    virtual void registerImage(const std::shared_ptr<Image> &image) = 0;

    [[nodiscard]] virtual std::shared_ptr<Image> getImage(const std::string &name) const = 0;

    [[nodiscard]] virtual std::shared_ptr<Image> getImage(const PUtilities::UniqueIdentifier &name) const = 0;

    virtual void registerBuffer(const std::shared_ptr<Buffer> &buffer) = 0;

    [[nodiscard]] virtual std::shared_ptr<Buffer> getBuffer(const std::string &name) const = 0;

    [[nodiscard]] virtual std::shared_ptr<Buffer> getBuffer(const PUtilities::UniqueIdentifier &name) const = 0;

    virtual void registerShaderConstant(const std::shared_ptr<ShaderConstant> &shaderConstant) = 0;

    /* Shader Modules Interface */

    virtual void registerShaderModule(const std::shared_ptr<ShaderModule> &createInfo) = 0;

    [[nodiscard]] virtual std::shared_ptr<ShaderModule> getShaderModule(const std::string &name) const = 0;

    /* Render Graph Getter */
    virtual std::shared_ptr<SceneRenderGraph> getSceneRenderGraph() = 0;

    [[nodiscard]] virtual const std::vector<std::shared_ptr<Model>> &getModels() const = 0;

    [[nodiscard]] virtual std::vector<std::shared_ptr<ShaderConstant>> getShaderConstants() const = 0;

    [[nodiscard]] virtual const std::vector<std::shared_ptr<Material>> &getMaterials() const = 0;

    [[nodiscard]] virtual const std::vector<std::shared_ptr<Image>> &getImages() const = 0;

    [[nodiscard]] virtual const std::vector<std::shared_ptr<Buffer>> &getBuffers() const = 0;

    [[nodiscard]] virtual const std::vector<std::shared_ptr<Texture>> &getTextures() const = 0;

    [[nodiscard]] virtual const std::vector<std::shared_ptr<ShaderModule>> &getShaderModules() const = 0;

    [[maybe_unused]][[nodiscard]] virtual std::vector<std::shared_ptr<SceneResource>> getAllResources() const = 0;

    /* Update Interface */
    // TODO - figure out how the hell we're gonna do this lol (or look up where I wrote that out lol)
    struct PerFrameUpdates {
    };

    [[nodiscard]] virtual const PerFrameUpdates &getPerFrameUpdates() const = 0;


};
