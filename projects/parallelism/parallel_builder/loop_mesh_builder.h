/**
 * @file    loop_mesh_builder.h
 *
 * @author  FULL NAME <xlogin00@stud.fit.vutbr.cz>
 *
 * @brief   Parallel Marching Cubes implementation using OpenMP loops
 *
 * @date    DATE
 **/

#ifndef LOOP_MESH_BUILDER_H
#define LOOP_MESH_BUILDER_H

#include <cstdlib>
#include <omp.h>
#include <vector>
#include "../common/base_mesh_builder.h"

class LoopMeshBuilder : public BaseMeshBuilder
{   
public:
    LoopMeshBuilder(unsigned gridEdgeSize);

protected:
    unsigned marchCubes(const ParametricScalarField &field);
    float evaluateFieldAt(const Vec3_t<float> &pos, const ParametricScalarField &field);
    void emitTriangle(const Triangle_t &triangle);
    const Triangle_t *getTrianglesArray() const { return mTriangles.data(); }
    std::vector<Triangle_t> mTriangles; ///< Temporary array of triangles
    // unsigned buildCube(const Vec3_t<float> &position, const ParametricScalarField &field, int x);
    //
    // Triangle_t *_mTriangles = (Triangle_t *)malloc(sizeof(Triangle_t) * mGridSize * mGridSize * mGridSize * 5 + 1);

private:
    // int *thread_positions = (int *)malloc(sizeof(int) * omp_get_num_threads());
    size_t *thread_positions = (size_t *)calloc(omp_get_num_threads() *2, sizeof(size_t));
    // triangles[thread_id * chunk + thread_positions[thread_id]]
    // [----|----``|----|----|----|----|----]

    uint chunk = mGridSize * mGridSize * mGridSize / omp_get_num_threads();
    uint ck = 5 * mGridSize * mGridSize * mGridSize / omp_get_num_threads();
};

#endif // LOOP_MESH_BUILDER_H
