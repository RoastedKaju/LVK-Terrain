#pragma once

#include <iostream>
#include <filesystem>
#include <vector>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
};

struct MeshData
{
	std::vector<Vertex> verts;
	std::vector<uint32_t> indices;
	lvk::Holder<lvk::BufferHandle> vertexBuffer;
	lvk::Holder<lvk::BufferHandle> indexBuffer;
};

struct UniformData
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
	glm::vec4 color;
	glm::vec4 cameraPosition;
	uint32_t outputTexId;
};