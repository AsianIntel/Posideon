#include "gltf_loader.h"

#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/parser.hpp>
#include <fastgltf/tools.hpp>

#include "render/renderer.h"

namespace Posideon {
    std::optional<std::vector<std::shared_ptr<GltfAsset>>> load_gltf_meshes(Renderer* renderer, std::filesystem::path path) {
        fastgltf::GltfDataBuffer data;
        data.loadFromFile(path);

        constexpr auto gltf_options = fastgltf::Options::LoadGLBBuffers | fastgltf::Options::LoadExternalBuffers;
        
        fastgltf::Parser parser {};

        auto load = parser.loadBinaryGLTF(&data, path.parent_path(), gltf_options);
        POSIDEON_ASSERT(load)
        fastgltf::Asset gltf = std::move(load.get());

        std::vector<std::shared_ptr<GltfAsset>> meshes;
        std::vector<uint32_t> indices;
        std::vector<Vertex> vertices;
        for (fastgltf::Mesh& mesh : gltf.meshes) {
            GltfAsset new_mesh;
            new_mesh.name = mesh.name;

            indices.clear();
            vertices.clear();

            for (auto&& p: mesh.primitives) {
                GltfSurface new_surface;
                new_surface.start_index = static_cast<uint32_t>(indices.size());
                new_surface.count = static_cast<uint32_t>(gltf.accessors[p.indicesAccessor.value()].count);

                size_t initial_vertex = vertices.size();
                {
                    fastgltf::Accessor& index_accessor = gltf.accessors[p.indicesAccessor.value()];
                    indices.reserve(indices.size() + index_accessor.count);

                    fastgltf::iterateAccessor<uint32_t>(gltf, index_accessor,
                        [&](uint32_t idx) {
                            indices.push_back(idx + initial_vertex);      
                        }
                    );
                }

                {
                    fastgltf::Accessor& position_accessor = gltf.accessors[p.findAttribute("POSITION")->second];
                    vertices.resize(vertices.size() + position_accessor.count);
                    fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, position_accessor,
                        [&](glm::vec3 v, size_t index) {
                            Vertex new_vertex;
                            new_vertex.position = v;
                            new_vertex.normal = { 1, 0, 0 };
                            new_vertex.color = glm::vec4(1.0f),
                            new_vertex.uv_x = 0;
                            new_vertex.uv_y = 0;
                            vertices[initial_vertex + index] = new_vertex;
                        }
                    );
                }

                {
                    auto normals = p.findAttribute("NORMAL");
                    if (normals != p.attributes.end()) {
                        fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, gltf.accessors[normals->second],
                            [&](glm::vec3 v, size_t index) {
                                vertices[initial_vertex + index].normal = v;
                            }
                        );
                    }
                }

                {
                    auto uv = p.findAttribute("TEXCOORD_0");
                    if (uv != p.attributes.end()) {
                        fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, gltf.accessors[uv->second],
                            [&](glm::vec2 v, size_t index) {
                                vertices[initial_vertex + index].uv_x = v.x;
                                vertices[initial_vertex + index].uv_y = v.y;
                            }
                        );
                    }
                }

                {
                    auto colors = p.findAttribute("COLOR_0");
                    if (colors != p.attributes.end()) {
                        fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, gltf.accessors[colors->second],
                            [&](glm::vec4 v, size_t index) {
                                vertices[initial_vertex + index].color = v;
                            }
                        );
                    }
                }

                new_mesh.surfaces.push_back(new_surface);
            }

            constexpr bool override_colors = true;
            if (override_colors) {
                for (Vertex& vertex: vertices) {
                    vertex.color = glm::vec4(vertex.normal, 1.0f);
                }
            }
            new_mesh.mesh_buffers = renderer->create_mesh(indices, vertices);
            meshes.emplace_back(std::make_shared<GltfAsset>(std::move(new_mesh)));
        }

        return meshes;
    }
}