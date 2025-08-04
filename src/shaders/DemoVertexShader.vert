#version 460

// basic vertex shader for animated model demo; keeping things as simple as possible for now

layout (location = 0) in vec4 position;
layout (location = 1) in vec4 normal;
layout (location = 3) in uvec4 boneIndices; // 4 bone matrix indices, one for each possible bone binding (per-vertex)
layout (location = 4) in vec4 boneWeights; // 4 matrix weights, one for each possible bone binding (per-vertex)

layout (location = 0) out vec4 vPos;
layout (location = 1) out vec4 vNormal;

layout (set = 0, binding = 0) uniform ModelMatrixBlock {
    mat4 mvpMatrix; // pre-computed model-view-projection matrix
    mat4 normalMatrix; // pre-computed transform for normals (inverse of transpose of MVP matrix)
};

// for each (animated) model, we'll also need to be binding a buffer containing all the different bone matrices
#define MAX_BONES_PER_VERTEX 4 // as is the convention we'll limit to 4 bones allowed to influence a given vertex
#define MAX_BONES 255
layout (set = 0, binding = 1) uniform BoneTransforms {
    mat4 boneMatrices[MAX_BONES];
};

void main() {
    // compute weighted sum of bone transform matrices for this vertex
    mat4 blendedBoneTransforms = mat4(0.0);
    for (int vertexBoneIndex = 0; vertexBoneIndex < MAX_BONES_PER_VERTEX; vertexBoneIndex++) {
        if (boneWeights[vertexBoneIndex] > 0) {
            blendedBoneTransforms += boneMatrices[boneIndices[vertexBoneIndex]] * boneWeights[vertexBoneIndex];
        }
    }

    gl_Position = mvpMatrix * blendedBoneTransforms * position;
    vPos = gl_Position;

    vNormal = vec4(normalize(mat3(normalMatrix) * transpose(inverse(mat3(blendedBoneTransforms))) * normal.xyz), 0.0);
}
