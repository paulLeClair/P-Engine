//
// Created by paull on 2023-11-15.
//

#pragma once

#include <vector>
#include <memory>
#include "GraphicsIntermediateRepresentation.hpp"

namespace pEngine::girEngine::gir::generator {

    class GirGenerator {
    public:
        ~GirGenerator() = default;

        [[nodiscard]] virtual std::vector<std::shared_ptr<pEngine::girEngine::gir::GraphicsIntermediateRepresentation>>
        bakeToGirs() const = 0;

    };

} // generator
