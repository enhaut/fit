/**
 * @file    tree_mesh_builder.cpp
 *
 * @author  Samuel Dobron <xlogin00@stud.fit.vutbr.cz>
 *
 * @brief   Parallel Marching Cubes implementation using OpenMP tasks + octree early elimination
 *
 * @date    01.12.2023
 **/

#include <iostream>
#include <math.h>
#include <limits>
#include <omp.h>

#include "../common/base_mesh_builder.h"
#include "tree_mesh_builder.h"

TreeMeshBuilder::TreeMeshBuilder(unsigned gridEdgeSize)
    : BaseMeshBuilder(gridEdgeSize, "Octree")
{
    for (int i = 0; i < omp_get_max_threads(); i++)
        triangles.push_back(std::vector<Triangle_t>());
}


unsigned TreeMeshBuilder::marchCubes(const ParametricScalarField &field)
{
    // Suggested approach to tackle this problem is to add new method to
    // this class. This method will call itself to process the children.
    // It is also strongly suggested to first implement Octree as sequential
    // code and only when that works add OpenMP tasks to achieve parallelism.

    size_t renderedTriangles;
    #pragma omp parallel
    {
        #pragma omp single nowait
        renderedTriangles = recursiveMarchCubes(0, Vec3_t<float>(0, 0, 0), mGridSize, field);
    }

    mTriangles.reserve(renderedTriangles);

    for (auto &threadArray : triangles)
        mTriangles.insert(mTriangles.end(), threadArray.begin(), threadArray.end());

    return renderedTriangles;
}


#define STEP 2
#define MAX_TASKS_DEPTH 20
size_t TreeMeshBuilder::recursiveMarchCubes(int depth, const Vec3_t<float> &cube, int gridSize, const ParametricScalarField &field)
{
    int totalTriangles = 0;

	if (isEmpty(cube, gridSize, field))
	  return 0;

	if (gridSize <= 1)
	  return buildCube(cube, field);
    
    auto halfCubeSize = gridSize / STEP;

    #pragma omp simd reduction(+: totalTriangles)
	for (int x = 0; x < STEP; x++)
	{
        for (int y = 0; y < STEP; y++)
        {
            for (int z = 0; z < STEP; z++)
            #pragma omp task firstprivate(x, y, z, depth) shared(totalTriangles) final(depth >= MAX_TASKS_DEPTH)
            {
	            float c_x = cube.x + x*halfCubeSize;
	            float c_y = cube.y + y*halfCubeSize;
	            float c_z = cube.z + z*halfCubeSize;
	            
                Vec3_t<float> halfCube = {
                    c_x,
                    c_y,
                    c_z
                };

                #pragma omp atomic update
                totalTriangles += recursiveMarchCubes(depth + 1, halfCube, halfCubeSize, field);
            }
        }
	}

    #pragma omp taskwait
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
    triangles[omp_get_thread_num()].push_back(triangle);
}
