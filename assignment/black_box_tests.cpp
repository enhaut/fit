//======== Copyright (c) 2017, FIT VUT Brno, All rights reserved. ============//
//
// Purpose:     Red-Black Tree - public interface tests
//
// $NoKeywords: $ivs_project_1 $black_box_tests.cpp
// $Author:     SAMUEL DOBROŇ <xdobro23@stud.fit.vutbr.cz>
// $Date:       $2021-02-21
//============================================================================//
/**
 * @file black_box_tests.cpp
 * @author SAMUEL DOBROŇ
 * 
 * @brief Implementace testu binarniho stromu.
 */

#include <vector>

#include "gtest/gtest.h"

#include "red_black_tree.h"


class EmptyTree : public ::testing::Test
{
protected:
    BinaryTree bintree;
    int keys[3] = {-1, 0, 1};
};

class NonEmptyTree : public ::testing::Test
{
protected:
    virtual void SetUp() {

        for(int value : existing_keys)
            bintree.InsertNode(value);
    }

    int existing_keys[10] = {-1, 0, 1, 2, 3, 4, 5, 6, 7, 8};
    int non_existing_keys[3] = {-2, 9, 10};
    BinaryTree bintree;
};

class TreeAxioms : public ::testing::Test
{
protected:
    virtual void SetUp() {

        for(int value : keys)
            bintree.InsertNode(value);
    }

    int keys[10] = {-1, 0, 1, 2, 3, 4, 5, 6, 7, 8};
    BinaryTree bintree;
};

TEST_F(EmptyTree, InsertNode)
{
    for (auto node_key : keys)
    {
        auto added = bintree.InsertNode(node_key);
        ASSERT_EQ(added.second->key, node_key);
        EXPECT_TRUE(added.first);

        auto wont_add = bintree.InsertNode(node_key);
        ASSERT_EQ(wont_add.second, added.second);
        EXPECT_FALSE(wont_add.first);
    }
}

TEST_F(EmptyTree, DeleteNode)
{
    for (auto node_key : keys)
    {
        ASSERT_FALSE(bintree.DeleteNode(node_key));     // node_key is not in the tree, it should return "false"
    }
}

TEST_F(EmptyTree, FindNode)
{
    for (auto node_key : keys)
    {
        ASSERT_EQ(bintree.FindNode(node_key), nullptr);
    }
}

TEST_F(NonEmptyTree, InsertNode)
{
    for (auto node_key : existing_keys)
    {
        auto wont_add = bintree.InsertNode(node_key);
        ASSERT_FALSE(wont_add.first);
        ASSERT_EQ(wont_add.second->key, node_key);
    }

    for (auto node_key : non_existing_keys)
    {
        auto added = bintree.InsertNode(node_key);
        ASSERT_TRUE(added.first);
        ASSERT_EQ(added.second->key, node_key);
    }
}

TEST_F(NonEmptyTree, DeleteNode)
{
    for (auto node_key : existing_keys)
    {
        ASSERT_TRUE(bintree.DeleteNode(node_key));
        ASSERT_FALSE(bintree.DeleteNode(node_key));     // node is already deleted
    }
}

TEST_F(NonEmptyTree, FindNode)
{
    for (auto node_key : existing_keys)
    {
        ASSERT_NE(bintree.FindNode(node_key), nullptr);
        ASSERT_EQ(bintree.FindNode(node_key)->key, node_key);
    }

    for (auto node_key : non_existing_keys)
    {
        ASSERT_EQ(bintree.FindNode(node_key), nullptr);
    }
}

TEST_F(TreeAxioms, Axiom1)
{
    std::vector<Node_t *> leafs;
    bintree.GetLeafNodes(leafs);

    for (auto leaf : leafs)
    {
        ASSERT_TRUE(leaf->color == BLACK);
    }
}

TEST_F(TreeAxioms, Axiom2)
{
    std::vector<Node_t *> leafs;
    bintree.GetAllNodes(leafs);

    for (auto leaf : leafs)
    {
        if (leaf->color != RED)     // just skipping non red leafnodes
            continue;
        ASSERT_TRUE(leaf->pLeft->color == BLACK);
        ASSERT_TRUE(leaf->pRight->color == BLACK);
    }
}

TEST_F(TreeAxioms, Axiom3)
{
    std::vector<Node_t *> leafs;
    bintree.GetLeafNodes(leafs);

    int reference_black_count = -1;

    for (auto leaf_node : leafs)
    {
        int black_count = 0;

        auto leaf = leaf_node;
        while (leaf && leaf != bintree.GetRoot())
        {
            if (leaf->color == BLACK)
                black_count++;

            leaf = leaf->pParent;
        }

        if (reference_black_count < 0)           // set reference count
            reference_black_count = black_count;

        ASSERT_EQ(black_count, reference_black_count);
    }
}

/*** Konec souboru black_box_tests.cpp ***/
