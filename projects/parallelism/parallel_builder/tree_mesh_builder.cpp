/**
 * @file    tree_mesh_builder.cpp
 *
 * @author  FULL NAME <xlogin00@stud.fit.vutbr.cz>
 *
 * @brief   Parallel Marching Cubes implementation using OpenMP tasks + octree early elimination
 *
 * @date    DATE
 **/

#include <iostream>
#include <math.h>
#include <limits>

#include "tree_mesh_builder.h"

TreeMeshBuilder::TreeMeshBuilder(unsigned gridEdgeSize)
    : BaseMeshBuilder(gridEdgeSize, "Octree")
{
    mTriangles.reserve(mGridResolution*mGridResolution*mGridResolution);
}

unsigned TreeMeshBuilder::marchCubes(const ParametricScalarField &field)
{
    // Suggested approach to tackle this problem is to add new method to
    // this class. This method will call itself to process the children.
    // It is also strongly suggested to first implement Octree as sequential
    // code and only when that works add OpenMP tasks to achieve parallelism.

    return recursiveDecomposition(Vec3_t<float>(0, 0, 0), mGridSize, field);
}


#define STEP 2
unsigned TreeMeshBuilder::recursiveDecomposition(const Vec3_t<float> &cube, int gridSize, const ParametricScalarField &field)
{
    int totalTriangles = 0;

	if (isEmpty(cube, gridSize, field))
	  return 0;

	if (gridSize <= 1)
	  return buildCube(cube, field);
    
    auto halfCubeSize = gridSize / STEP;

	for (int x = 0; x < STEP; x++)
	{
        for (int y = 0; y < STEP; y++)
        {
            for (int z = 0; z < STEP; z++)
            {
	            float c_x = cube.x + x*halfCubeSize;
	            float c_y = cube.y + y*halfCubeSize;
	            float c_z = cube.z + z*halfCubeSize;
	            
                Vec3_t<float> halfCube = {
                    c_x,
                    c_y,
                    c_z
                };

                totalTriangles += recursiveDecomposition(halfCube, halfCubeSize, field);
            }
        }
	}
    return totalTriangles;
}

bool TreeMeshBuilder::isEmpty(const Vec3_t<float> &offset, float gridSize, const ParametricScalarField &field)
{
    Vec3_t<float> mid = {
        offset.x * mGridResolution + gridSize * mGridResolution / 2.0f,
        offset.y * mGridResolution + gridSize * mGridResolution / 2.0f,
        offset.z * mGridResolution + gridSize * mGridResolution / 2.0f 
    };
    float fp = evaluateFieldAt(mid, field);

    return fp > mIsoLevel + sqrt(3.0f)/2.0f*gridSize*mGridResolution;
}

float TreeMeshBuilder::evaluateFieldAt(const Vec3_t<float> &pos, const ParametricScalarField &field)
{

    const Vec3_t<float> *pPoints = field.getPoints().data();
    const unsigned count = unsigned(field.getPoints().size());

    float value = std::numeric_limits<float>::max();
    
    #pragma omp simd simdlen(96) reduction(min: value)
    for(unsigned i = 0; i < count; ++i)
    {
        float distanceSquared  = (pos.x - pPoints[i].x) * (pos.x - pPoints[i].x);
        distanceSquared       += (pos.y - pPoints[i].y) * (pos.y - pPoints[i].y);
        distanceSquared       += (pos.z - pPoints[i].z) * (pos.z - pPoints[i].z);

        value = std::min(value, distanceSquared);
    }

    return sqrt(value);
}

void TreeMeshBuilder::emitTriangle(const BaseMeshBuilder::Triangle_t &triangle)
{
    mTriangles.push_back(triangle);
}
