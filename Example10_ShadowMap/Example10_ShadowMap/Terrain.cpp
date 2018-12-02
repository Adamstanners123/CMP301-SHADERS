#include "Terrain.h"



Terrain::Terrain(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	initBuffers(device);
}


Terrain::~Terrain()
{
	// Run parent deconstructor
	BaseMesh::~BaseMesh();
}

// Build shape and fill buffers.
void Terrain::initBuffers(ID3D11Device* device)
{
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// Load heightmap data into arrays before we fill our buffers
	heightMapLoad("height2.bmp", m_bmpHeightMap);

	// Calculate index count
	indexCount *= 3;

	// Create vertex buffer
	D3D11_BUFFER_DESC vertexBufferDescOne = { sizeof(VertexType) * vertexCount, D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER, 0, 0, 0 };
	vertexData = { m_newTerrainVerts, 0 , 0 };
	m_result = device->CreateBuffer(&vertexBufferDescOne, &vertexData, &vertexBuffer);

	// Create index buffer
	D3D11_BUFFER_DESC indexBufferDesc = { sizeof(unsigned long) * indexCount, D3D11_USAGE_DEFAULT, D3D11_BIND_INDEX_BUFFER, 0, 0, 0 };
	indexData = { m_indices, 0, 0 };
	m_result = device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete[] m_terrainVerts;
	m_terrainVerts = 0;
	delete[] m_newTerrainVerts;
	m_newTerrainVerts = 0;
	delete[] m_indices;
	m_indices = 0;
}

// Send Geometry data to the GPU
void Terrain::sendData(ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top)
{
	unsigned int stride;
	unsigned int offset;

	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType);
	offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(top);
}

// Load heightmap in from file
bool Terrain::heightMapLoad(char * filename, HeightMapData & m_bmpHeightMap)
{
	FILE *heightmapFilePtr;
	BITMAPFILEHEADER heightMapFileHeader;
	BITMAPINFOHEADER heightMapInfoHeader;
	int imageSize, index;
	unsigned char height;

	// Open file
	fopen_s(&heightmapFilePtr, filename, "rb");
	if (heightmapFilePtr == NULL)
	{
		return 0;
	}

	// Read heightmap headers
	fread(&heightMapFileHeader, sizeof(BITMAPFILEHEADER), 1, heightmapFilePtr);
	fread(&heightMapInfoHeader, sizeof(BITMAPINFOHEADER), 1, heightmapFilePtr);

	// Get width/height
	m_bmpHeightMap.terrainWidth = heightMapInfoHeader.biWidth;
	m_bmpHeightMap.terrainHeight = heightMapInfoHeader.biHeight;

	// Calculate image size
	imageSize = m_bmpHeightMap.terrainWidth * m_bmpHeightMap.terrainHeight * 3;

	// Create container for image data
	unsigned char* bitmapImage = new unsigned char[imageSize];

	// Read data from file and then close it
	fseek(heightmapFilePtr, heightMapFileHeader.bfOffBits, SEEK_SET);
	fread(bitmapImage, 1, imageSize, heightmapFilePtr);
	fclose(heightmapFilePtr);

	// Initialize the heightMap array 
	m_bmpHeightMap.heightMap = new XMFLOAT3[m_bmpHeightMap.terrainWidth * m_bmpHeightMap.terrainHeight];

	// Create counter
	int k = 0;

	// For scaling height values
	float heightFactor = 10.0f;

	// Read the image data into heightmap array
	for (int j = 0; j< m_bmpHeightMap.terrainHeight; j++)
	{
		for (int i = 0; i< m_bmpHeightMap.terrainWidth; i++)
		{
			height = bitmapImage[k];

			index = (m_bmpHeightMap.terrainHeight * j) + i;

			m_bmpHeightMap.heightMap[index].x = (float)i;
			m_bmpHeightMap.heightMap[index].y = (float)height / heightFactor;
			m_bmpHeightMap.heightMap[index].z = (float)j;
		
			k += 3;
		}
	}

	k = 0;

	// Initialise vertex array
	createVertices();
	
	// Fill index array 
	calculateIndices();

	// Fill vertex array with normals
	calculateNormals();

	// Initialise new vertex array
	fillNewTerrainVerts();

	// Fill vertex array with tangents and binormals
	calculateTerrainVector();

	delete[] bitmapImage;
	bitmapImage = 0;

	return true;
}

// Initialise vertex array
void Terrain::createVertices()
{
	int cols = m_bmpHeightMap.terrainWidth;
	int rows = m_bmpHeightMap.terrainHeight;

	// Calculate counts and resize arrays
	vertexCount = rows * cols;
	indexCount = (rows - 1)*(cols - 1) * 2;

	m_terrainVerts = new VertexType[vertexCount];
	m_indices = new unsigned long[indexCount * 3];

	for (DWORD i = 0; i < rows; ++i)
	{
		for (DWORD j = 0; j < cols; ++j)
		{
			m_terrainVerts[i*cols + j].position = m_bmpHeightMap.heightMap[i*cols + j];
			m_terrainVerts[i*cols + j].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
		}
	}
}

// Fill array with normals
void Terrain::calculateNormals()
{
	int index1, index2, index3, index;

	// Vertices
	XMFLOAT3 v1, v2, v3;

	// Vectors
	XMFLOAT3 vec1, vec2, vecsum;

	// Length of vectors
	float len;

	// Create normal array
	tempNorms = new XMFLOAT3[(m_bmpHeightMap.terrainHeight - 1) * (m_bmpHeightMap.terrainWidth - 1)];

	// Calculate normals
	for (int j = 0; j< (m_bmpHeightMap.terrainHeight - 1); j++)
	{
		for (int i = 0; i< (m_bmpHeightMap.terrainWidth - 1); i++)
		{
			index1 = (j * m_bmpHeightMap.terrainWidth) + i;
			index2 = ((j + 1) * m_bmpHeightMap.terrainWidth) + (i + 1);
			index3 = ((j + 1) * m_bmpHeightMap.terrainWidth) + i;

			v1 = m_bmpHeightMap.heightMap[index1];

			v2 = m_bmpHeightMap.heightMap[index2];

			v3 = m_bmpHeightMap.heightMap[index3];

			// Calculate vector for first tri
			vec1.x = v1.x - v3.x;
			vec1.y = v1.y - v3.y;
			vec1.z = v1.z - v3.z;

			// Calculate vector for second tri
			vec2.x = v3.x - v2.x;
			vec2.y = v3.y - v2.y;
			vec2.z = v3.z - v2.z;

			index = (j * (m_bmpHeightMap.terrainWidth - 1)) + i;

			// Get cross product of vecs
			tempNorms[index].x = (vec1.y * vec2.z) - (vec1.z * vec2.y);
			tempNorms[index].y = (vec1.z * vec2.x) - (vec1.x * vec2.z);
			tempNorms[index].z = (vec1.x * vec2.y) - (vec1.y * vec2.x);

			// Get length
			len = (float)sqrt((tempNorms[index].x * tempNorms[index].x) + (tempNorms[index].y * tempNorms[index].y) + (tempNorms[index].z * tempNorms[index].z));

			// Normalize 
			tempNorms[index].x = (tempNorms[index].x / len);
			tempNorms[index].y = (tempNorms[index].y / len);
			tempNorms[index].z = (tempNorms[index].z / len);
		}
	}

	calculateNormalsSum(tempNorms);

}

// Get sum of normals
void Terrain::calculateNormalsSum(XMFLOAT3* tempNorms)
{
	int count, index;

	float len;

	XMFLOAT3 vecsum;

	// Get sum of normals
	for (int j = 0; j< m_bmpHeightMap.terrainHeight; j++)
	{
		for (int i = 0; i< m_bmpHeightMap.terrainWidth; i++)
		{
			vecsum.x = 0; vecsum.y = 0; vecsum.z = 0;

			count = 0;

			// Bottom left
			if (((i - 1) >= 0) && ((j - 1) >= 0))
			{
				index = ((j - 1) * (m_bmpHeightMap.terrainWidth - 1)) + (i - 1);

				vecsum.x += tempNorms[index].x;
				vecsum.y += tempNorms[index].y;
				vecsum.z += tempNorms[index].z;
				count++;
			}
			// Top left 
			if (((i - 1) >= 0) && (j<(m_bmpHeightMap.terrainHeight - 1)))
			{
				index = (j * (m_bmpHeightMap.terrainWidth - 1)) + (i - 1);

				vecsum.x += tempNorms[index].x;
				vecsum.y += tempNorms[index].y;
				vecsum.z += tempNorms[index].z;
				count++;
			}

			// Top right
			if ((i < (m_bmpHeightMap.terrainWidth - 1)) && (j < (m_bmpHeightMap.terrainHeight - 1)))
			{
				index = (j * (m_bmpHeightMap.terrainWidth - 1)) + i;

				vecsum.x += tempNorms[index].x;
				vecsum.y += tempNorms[index].y;
				vecsum.z += tempNorms[index].z;
				count++;
			}

			// Bottom right
			if ((i<(m_bmpHeightMap.terrainWidth - 1)) && ((j - 1) >= 0))
			{
				index = ((j - 1) * (m_bmpHeightMap.terrainWidth - 1)) + i;

				vecsum.x += tempNorms[index].x;
				vecsum.y += tempNorms[index].y;
				vecsum.z += tempNorms[index].z;
				count++;
			}
			
			// Get average
			vecsum.x = (vecsum.x / (float)count);
			vecsum.y = (vecsum.y / (float)count);
			vecsum.z = (vecsum.z / (float)count);

			// Get length
			len = (float)sqrt((vecsum.x * vecsum.x) + (vecsum.y * vecsum.y) + (vecsum.z * vecsum.z));

			index = (j * m_bmpHeightMap.terrainWidth) + i;

			// Normalize
			m_terrainVerts[index].normal.x = (vecsum.x / len);
			m_terrainVerts[index].normal.y = (vecsum.y / len);
			m_terrainVerts[index].normal.z = (vecsum.z / len);
		}
	}

	delete[] tempNorms;
	tempNorms = 0;
}

// Fill index array 
void Terrain::calculateIndices()
{
	int cols = m_bmpHeightMap.terrainWidth;
	int rows = m_bmpHeightMap.terrainHeight;
	
	int k = 0;

	for (int i = 0; i < rows - 1; i++)
	{
		for (int j = 0; j < cols - 1; j++)
		{
			m_indices[k] = i * cols + j;				// Top left  

			m_indices[k + 1] = i * cols + j + 1;		// Top right  

			m_indices[k + 2] = (i + 1)*cols + j;		// Bottom left  

			m_indices[k + 3] = (i + 1)*cols + j;		// Bottom left  

			m_indices[k + 4] = i * cols + j + 1;        // Top right  

			m_indices[k + 5] = (i + 1)*cols + j + 1;    // Bottom right  

			k += 6;									
		}

	}
}

// Fill new vertex array and re-assign indices
void Terrain::fillNewTerrainVerts()
{
	int cols = m_bmpHeightMap.terrainWidth;
	int rows = m_bmpHeightMap.terrainHeight;

	// Calculate new vertex count
	vertexCount = indexCount * 3;

	int k = 0;

	// Re size new vertex array
	m_newTerrainVerts = new VertexType[vertexCount];

	// Fill vertex array with heightmap data
	for (int i = 0; i < rows - 1; i++)
	{
		for (int j = 0; j < cols - 1; j++)
		{

			m_newTerrainVerts[k] = m_terrainVerts[m_indices[k]];
			m_newTerrainVerts[k].texture.x = 0.0f;
			m_newTerrainVerts[k].texture.y = 0.0f;
			m_indices[k] = k;

			m_newTerrainVerts[k + 1] = m_terrainVerts[m_indices[k + 1]];
			m_newTerrainVerts[k + 1].texture.x = 1.0f;
			m_newTerrainVerts[k + 1].texture.y = 0.0f;
			m_indices[k + 1] = k + 1;

			m_newTerrainVerts[k + 2] = m_terrainVerts[m_indices[k + 2]];
			m_newTerrainVerts[k + 2].texture.x = 0.0f;
			m_newTerrainVerts[k + 2].texture.y = 1.0f;
			m_indices[k + 2] = k + 2;

			m_newTerrainVerts[k + 3] = m_terrainVerts[m_indices[k + 3]];
			m_newTerrainVerts[k + 3].texture.x = 0.0f;
			m_newTerrainVerts[k + 3].texture.y = 1.0f;
			m_indices[k + 3] = k + 3;

			m_newTerrainVerts[k + 4] = m_terrainVerts[m_indices[k + 4]];
			m_newTerrainVerts[k + 4].texture.x = 1.0f;
			m_newTerrainVerts[k + 4].texture.y = 0.0f;
			m_indices[k + 4] = k + 4;

			m_newTerrainVerts[k + 5] = m_terrainVerts[m_indices[k + 5]];
			m_newTerrainVerts[k + 5].texture.x = 1.0f;
			m_newTerrainVerts[k + 5].texture.y = 1.0f;
			m_indices[k + 5] = k + 5 ;

			k += 6; 
		}
	}
}

// Fills new vertex array with tangents and binormals
void Terrain::calculateTerrainVector()
{
	// Determine how many faces are in terrain mesh
	int no_of_faces = vertexCount / 3;
	XMFLOAT3 tangent = { 0, 0, 0 };
	XMFLOAT3 binormal = tangent;
	VertexType temp_verts_one, temp_verts_two, temp_verts_three;

	int k = 0;

	// Fill array
	for (int i = 0; i < no_of_faces; i++)
	{
		temp_verts_one = m_newTerrainVerts[k];
		k++;

		temp_verts_two = m_newTerrainVerts[k];
		k++;

		temp_verts_three = m_newTerrainVerts[k];
		k++;

		calculateBiNormal(temp_verts_one, temp_verts_two, temp_verts_three, tangent);

		m_newTerrainVerts[k - 1].tangent = tangent;
		m_newTerrainVerts[k - 2].tangent = tangent;
		m_newTerrainVerts[k - 3].tangent = tangent;

	}
}

// Calculate binormals and tangents
void Terrain::calculateBiNormal(VertexType temp_verts_one, VertexType temp_verts_two, VertexType temp_verts_three, XMFLOAT3& tangent)
{
	XMFLOAT3 vector_one, vector_two;
	XMFLOAT2 uv_vector_one, uv_vector_two;
	float denominator = 0;
	float len;

	// Create vectors
	vector_one.x = temp_verts_two.position.x - temp_verts_one.position.x;
	vector_one.y = temp_verts_two.position.y - temp_verts_one.position.y;
	vector_one.z = temp_verts_two.position.z - temp_verts_one.position.z;

	vector_two.x = temp_verts_three.position.x - temp_verts_one.position.x;
	vector_two.y = temp_verts_three.position.y - temp_verts_one.position.y;
	vector_two.z = temp_verts_three.position.z - temp_verts_one.position.z;

	uv_vector_one.x = temp_verts_two.texture.x - temp_verts_one.texture.x;
	uv_vector_one.y = temp_verts_two.texture.y - temp_verts_one.texture.y;

	uv_vector_two.x = temp_verts_three.texture.x - temp_verts_one.texture.x;
	uv_vector_two.y = temp_verts_three.texture.y - temp_verts_one.texture.y;

	// Calculate the denominator
	denominator = 1.0f / (uv_vector_one.x * uv_vector_two.y - uv_vector_two.x * uv_vector_one.y);

	tangent.x = (uv_vector_two.y * vector_one.x - uv_vector_one.y * vector_two.x) * denominator;
	tangent.y = (uv_vector_two.y * vector_one.y - uv_vector_one.y * vector_two.y) * denominator;
	tangent.z = (uv_vector_two.y * vector_one.z - uv_vector_one.y * vector_two.z) * denominator;

	// Calculate the length of the tangent.
	len = (float)sqrt((tangent.x * tangent.x) + (tangent.y * tangent.y) + (tangent.z * tangent.z));

	// Normalize the tangent and then store it.
	tangent.x = tangent.x / len;
	tangent.y = tangent.y / len;
	tangent.z = tangent.z / len;

}

