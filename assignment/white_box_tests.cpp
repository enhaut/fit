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

TEST_F(BaseMatrix, set)
{
    for (int row = 0; row < small.rows; row++)
    {
        for (int col = 0; col < small.cols; col++)
        {
            ASSERT_TRUE(small.matrix.set(row, col, row + col + 10));
            EXPECT_EQ(small.matrix.get(row, col), row + col + 10);
        }
    }

    ASSERT_FALSE(small.matrix.set(small.rows + 1, small.cols, 0));
    ASSERT_FALSE(small.matrix.set(small.rows, small.cols + 1, 0));

    std::vector<std::vector< double >> matrix_to_set = {
                                                        {10.0, 5.7, 3.3, 9},
                                                        {8.2, 3.14, 9.9, 100.22},
                                                        {0.8, 0.1, 7.7, -9.0}
                                                        };

    ASSERT_TRUE(small.matrix.set(matrix_to_set));

    std::vector<std::vector< double >> deformed_matrix = {{1, 2, 3}, {3, 4, 5}};
    ASSERT_FALSE(small.matrix.set(deformed_matrix));

    deformed_matrix[1] = {3, 4};
    ASSERT_FALSE(small.matrix.set(deformed_matrix));
}

TEST_F(BaseMatrix, get)
{
    ASSERT_THROW(small.matrix.get(-1, small.cols - 1), std::runtime_error);
    ASSERT_THROW(small.matrix.get(small.rows - 1, -1), std::runtime_error);
}

TEST_F(BaseMatrix, opratorEQ)
{
    auto zero_matrix = new Matrix();
    ASSERT_THROW((bool)(small.matrix == *zero_matrix), std::runtime_error);

    auto same_matrix = new Matrix(small.rows, small.cols);
    EXPECT_FALSE((bool)(small.matrix == *same_matrix));


    for (int row = 0; row < small.rows; row++)
        for (int col = 0; col < small.cols; col++)
            same_matrix->set(row, col, small.matrix.get(row, col));

    EXPECT_TRUE((bool)(small.matrix == *same_matrix));
}

TEST_F(BaseMatrix, OperatorPlus)
{
    ASSERT_THROW((small.matrix + big.matrix), std::runtime_error);

    Matrix to_add = Matrix(big.rows, big.cols);
    for (int row = 0; row < big.rows; row++)
        for (int col = 0; col < big.cols; col++)
            to_add.set(row, col, big.matrix.get(row, col)*-1);

   auto sum_result = big.matrix + to_add;

   Matrix result = Matrix(big.rows, big.cols);
   ASSERT_TRUE((bool)(sum_result == result));
}

/*** Konec souboru white_box_tests.cpp ***/
