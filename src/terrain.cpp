#include "terrain.h"

void Terrain::loadHeightmap(const char* filename)
{
	int width, height, channels;

	// single channel greyscale
	unsigned char* data = stbi_load(filename, &width, &height, &channels, 1);

	if (!data)
	{
		fprintf(stderr, "Failed to load heightmap: %s\n", stbi_failure_reason());
		return;
	}

	heightmapWidth = static_cast<uint32_t>(width);
	heightmapHeight = static_cast<uint32_t>(height);

	heightmapData.resize(static_cast<size_t>(width * height));
	// store normalized float values
	for (int i = 0; i < width * height; ++i)
	{
		heightmapData[i] = static_cast<float>(data[i]) / 255.0f;
	}

	stbi_image_free(data);
}

void Terrain::generateTerrain(uint32_t quadCountX, uint32_t quadCountY)
{
	vertsX = quadCountX + 1;
	vertsY = quadCountY + 1;

	terrainData.verts.clear();
	terrainData.indices.clear();
	terrainData.verts.reserve(static_cast<size_t>(vertsX * vertsY));
	terrainData.indices.reserve(static_cast<size_t>(quadCountX * quadCountY * 6));

	// Vertex generation
	for (uint32_t row = 0; row < vertsY; ++row)
	{
		for (uint32_t col = 0; col < vertsX; ++col)
		{
			const float u = static_cast<float>(col) / static_cast<float>(quadCountX);
			const float v = static_cast<float>(row) / static_cast<float>(quadCountY);

			const float height = sampleHeightmap(u, v) * heightScale;

			Vertex vert{};
			vert.position = glm::vec3(
				u * terrainScale,
				height,
				v * terrainScale
			);

			//vert.texCoord = glm::vec2(u, v);
			terrainData.verts.push_back(vert);
		}
	}

	// Index generation
	for (uint32_t row = 0; row < quadCountY; ++row)
	{
		for (uint32_t col = 0; col < quadCountX; ++col)
		{
			const uint32_t TL = (row)*vertsX + (col);
			const uint32_t TR = (row)*vertsX + (col + 1);
			const uint32_t BL = (row + 1) * vertsX + (col);
			const uint32_t BR = (row + 1) * vertsX + (col + 1);

			terrainData.indices.push_back(TL);
			terrainData.indices.push_back(BL);
			terrainData.indices.push_back(BR);

			terrainData.indices.push_back(TL);
			terrainData.indices.push_back(BR);
			terrainData.indices.push_back(TR);
		}
	}

	calculateNormals();
	generateBuffers();
}

void Terrain::calculateNormals()
{
	for (uint32_t row = 0; row < vertsY; ++row)
	{
		for (uint32_t col = 0; col < vertsX; ++col)
		{
			// Clamp neighbours to edge
			const uint32_t colL = col > 0 ? col - 1 : col;
			const uint32_t colR = col < vertsX - 1 ? col + 1 : col;
			const uint32_t rowU = row > 0 ? row - 1 : row;
			const uint32_t rowD = row < vertsY - 1 ? row + 1 : row;

			// Grab neighbour positions directly from the vertex buffer
			const glm::vec3& posL = terrainData.verts[row * vertsX + colL].position;
			const glm::vec3& posR = terrainData.verts[row * vertsX + colR].position;
			const glm::vec3& posU = terrainData.verts[rowU * vertsX + col].position;
			const glm::vec3& posD = terrainData.verts[rowD * vertsX + col].position;

			// Central difference vectors
			const glm::vec3 dx = posR - posL;
			const glm::vec3 dz = posD - posU;

			terrainData.verts[row * vertsX + col].normal = glm::normalize(glm::cross(dz, dx));
		}
	}
}

void Terrain::generateBuffers()
{
	terrainData.vertexBuffer.reset();
	terrainData.indexBuffer.reset();

	// Vertex buffer
	lvk::BufferDesc vertBufDesc{};
	vertBufDesc.usage = lvk::BufferUsageBits_Vertex;
	vertBufDesc.storage = lvk::StorageType_Device;
	vertBufDesc.size = sizeof(Vertex) * terrainData.verts.size();
	vertBufDesc.data = terrainData.verts.data();
	vertBufDesc.debugName = "Buffer: vertex";
	terrainData.vertexBuffer = context.createBuffer(vertBufDesc);
	// Index Buffer
	lvk::BufferDesc indexBufDes{};
	indexBufDes.usage = lvk::BufferUsageBits_Index;
	indexBufDes.storage = lvk::StorageType_Device;
	indexBufDes.size = sizeof(uint32_t) * terrainData.indices.size();
	indexBufDes.data = terrainData.indices.data();
	indexBufDes.debugName = "Buffer: index";
	terrainData.indexBuffer = context.createBuffer(indexBufDes);
}

float Terrain::sampleHeightmap(float u, float v) const
{
	// Bilinear interpolation sampler - u,v in [0,1]

	if (heightmapData.empty()) { return 0.0f; }

	// Map [0,1] to heightmap pixel space
	const float x = u * static_cast<float>(heightmapWidth - 1);
	const float y = v * static_cast<float>(heightmapHeight - 1);

	// Integer pixel coords of the 4 surrounding pixels
	const uint32_t x0 = static_cast<uint32_t>(x);
	const uint32_t y0 = static_cast<uint32_t>(y);
	const uint32_t x1 = std::min(x0 + 1, heightmapWidth - 1);
	const uint32_t y1 = std::min(y0 + 1, heightmapHeight - 1);

	// Fractional part for interpolation weights
	const float fx = x - static_cast<float>(x0);
	const float fy = y - static_cast<float>(y0);

	// Sample 4 corners
	//  TL --- TR
	//   |     |
	//  BL --- BR
	const float TL = heightmapData[y0 * heightmapWidth + x0];
	const float TR = heightmapData[y0 * heightmapWidth + x1];
	const float BL = heightmapData[y1 * heightmapWidth + x0];
	const float BR = heightmapData[y1 * heightmapWidth + x1];

	// Bilinear interpolate: first along X, then along Y
	const float top = TL + fx * (TR - TL);
	const float bottom = BL + fx * (BR - BL);
	return top + fy * (bottom - top);
}
