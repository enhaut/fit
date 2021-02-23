//======== Copyright (c) 2021, FIT VUT Brno, All rights reserved. ============//
//
// Purpose:     White Box - Tests suite
//
// $NoKeywords: $ivs_project_1 $white_box_code.cpp
// $Author:     SAMUEL DOBROŇ <xdobro23@stud.fit.vutbr.cz>
// $Date:       $2021-02-23
//============================================================================//
/**
 * @file white_box_tests.cpp
 * @author SAMUEL DOBROŇ
 * 
 * @brief Implementace testu prace s maticemi.
 */

#include "gtest/gtest.h"
#include "white_box_code.h"

typedef struct{
    size_t rows;
    size_t cols;
    Matrix matrix;
}MatrixHolder;

class BaseMatrix : public ::testing::Test
{
protected:
    MatrixHolder small = {3, 4, Matrix(3, 4)};
    MatrixHolder big = {9, 22, Matrix(9, 22)};

    virtual void SetUp()
    {
        for (int row = 0; row < small.rows; row++)
            for (int col = 0; col < small.cols; col++)
                small.matrix.set(row, col, row+col);
            
        for (int row = 0; row < big.rows; row++)
            for (int col = 0; col < big.cols; col++)
                big.matrix.set(row, col, row+col);
    }
};

TEST_F(BaseMatrix, Constructor)
{
    // Testing constructor with no parameters
    auto zero_matrix_small = new Matrix();
    ASSERT_EQ(zero_matrix_small->get(0, 0), 0);

    // Testing throws of constructor
    int rows = 3;
    int cols = 4;
    ASSERT_THROW(Matrix(rows, 0), std::runtime_error);
    ASSERT_THROW(Matrix(0, cols), std::runtime_error);

    ASSERT_NO_THROW(Matrix(rows, cols));

    // Testing constructor with parameters
    auto zero_matrix_big = new Matrix(rows, cols);
    for (; rows; rows--)
    {
        for (; cols; cols--)
        {
            ASSERT_EQ(zero_matrix_big->get(rows - 1, cols - 1), 0); // rows and cols are indexed from 1
        }
    }
}


/*** Konec souboru white_box_tests.cpp ***/
