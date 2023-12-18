#pragma once

#include "defines.h"
#include <glm/glm.hpp>

namespace Posideon {
    struct Camera {
        glm::mat4 projection;
    };

    struct ExtractedView {
        glm::mat4 projection;
        glm::mat4 view;
    };
}