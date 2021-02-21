//======== Copyright (c) 2017, FIT VUT Brno, All rights reserved. ============//
//
// Purpose:     Red-Black Tree - public interface tests
//
// $NoKeywords: $ivs_project_1 $black_box_tests.cpp
// $Author:     JMENO PRIJMENI <xlogin00@stud.fit.vutbr.cz>
// $Date:       $2017-01-04
//============================================================================//
/**
 * @file black_box_tests.cpp
 * @author JMENO PRIJMENI
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

//============================================================================//
// ** ZDE DOPLNTE TESTY **
//
// Zde doplnte testy Red-Black Tree, testujte nasledujici:
// 2. Axiomy (tedy vzdy platne vlastnosti) Red-Black Tree:
//    - Vsechny listove uzly stromu jsou *VZDY* cerne.
//    - Kazdy cerveny uzel muze mit *POUZE* cerne potomky.
//    - Vsechny cesty od kazdeho listoveho uzlu ke koreni stromu obsahuji
//      *STEJNY* pocet cernych uzlu.
//============================================================================//

/*** Konec souboru black_box_tests.cpp ***/
