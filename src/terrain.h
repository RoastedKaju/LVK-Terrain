#pragma once

#include <lvk/LVK.h>

#include <vector>
#include <math.h>
#include <map>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <stb/stb_image.h>

#include "model.h"

class Terrain
{
public:
	Terrain(lvk::IContext& ctx) : context{ ctx } {}

	void loadHeightmap(const char* filename);
	void generateTerrain(uint32_t quadCountX, uint32_t quadCountY);

	inline const MeshData& getTerrainData() const { return terrainData; }

	float heightScale = 1.0f;
	float terrainScale = 1.0f;

private:
	void calculateNormals();
	void generateBuffers();
	float sampleHeightmap(float u, float v) const;

	MeshData terrainData;
	lvk::IContext& context;

	uint32_t vertsX = 0;
	uint32_t vertsY = 0;

	std::vector<float> heightmapData;
	uint32_t heightmapWidth = 0;
	uint32_t heightmapHeight = 0;
};