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
    // const Triangle_t *getTrianglesArray() const { return nullptr; }
    const Triangle_t *getTrianglesArray() const { return mTriangles.data(); }
    std::vector<Triangle_t> mTriangles; ///< Temporary array of triangles
};

#endif // LOOP_MESH_BUILDER_H
