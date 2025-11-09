#include "HeightMap.h"
#include <iostream>
#include <chrono>

HeightMap::HeightMap(const std::string& name)
{
	int iWidth, iHeight, iChannels;
	unsigned char* data = SOIL_load_image(name.c_str(),
		&iWidth, &iHeight, &iChannels, 1);

	if (!data) {
		std::cout << "HeightMap load failed for " << name << std::endl;
		return;
	}

	numVertices = iWidth * iHeight;
	numIndices = (iWidth - 1) * (iHeight - 1) * 6;	
	vertices = new Vector3[numVertices];
	textureCoords = new Vector2[numVertices];
	indices = new unsigned int[numIndices];

	Vector3 vertexScale = Vector3(1.0f, 1.0f, 1.0f);
	Vector2 textureScale = Vector2(1.0f / float(iWidth - 1), 1.0f / float(iHeight - 1));

	for (int z = 0; z < iHeight; z++) {
		for (int x = 0; x < iWidth; x++) {
			int offset = (z * iWidth) + x;
			unsigned char h = data[offset];
			// apply per-axis vertexScale
			vertices[offset] = Vector3(x * vertexScale.x, (float)h * vertexScale.y, z * vertexScale.z);

			// map to 0..1 across the whole heightmap
			textureCoords[offset] = Vector2(float(x) * textureScale.x, float(z) * textureScale.y);
		}
	}
	SOIL_free_image_data(data);

	int i = 0;
	for (int z = 0; z < iHeight - 1; z++) {
		for (int x = 0; x < iWidth - 1; x++) {
			int a = (z * iWidth) + x;
			int b = (z * (iWidth) + (x + 1));
			int c = ((z + 1) * iWidth) + (x+1);
			int d = ((z + 1) * iWidth) + x;

			indices[i++] = a;
			indices[i++] = c;
			indices[i++] = b;

			indices[i++] = c;
			indices[i++] = a;
			indices[i++] = d;

		}
	}
	GenerateNormals();
	GenerateTangents();
	BufferData();
	heightMapSize.x = vertexScale.x * (iWidth-1);
	heightMapSize.y = vertexScale.y * 255.0f;
	heightMapSize.z = vertexScale.z * (iHeight - 1);
}