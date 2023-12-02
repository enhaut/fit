/**
 * @file    tree_mesh_builder.h
 *
 * @author  FULL NAME <xlogin00@stud.fit.vutbr.cz>
 *
 * @brief   Parallel Marching Cubes implementation using OpenMP tasks + octree early elimination
 *
 * @date    DATE
 **/

#ifndef TREE_MESH_BUILDER_H
#define TREE_MESH_BUILDER_H

#include <vector>
#include "../common/base_mesh_builder.h"

class TreeMeshBuilder : public BaseMeshBuilder
{
public:
    TreeMeshBuilder(unsigned gridEdgeSize);

protected:
    std::vector<std::vector<Triangle_t>> triangles;
    std::vector<Triangle_t> mTriangles;

    unsigned marchCubes(const ParametricScalarField &field);
    float evaluateFieldAt(const Vec3_t<float> &pos, const ParametricScalarField &field);
    void emitTriangle(const Triangle_t &triangle);
    const Triangle_t *getTrianglesArray() const {return mTriangles.data();};
    size_t recursiveDecomposition(const Vec3_t<float> &offset, int gridSize, const ParametricScalarField &field);
    bool isEmpty(const Vec3_t<float> &offset, float gridSize, const ParametricScalarField &field);

};

#endif // TREE_MESH_BUILDER_H
