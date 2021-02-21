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
        bintree.InsertNode(node_key);
        ASSERT_TRUE(bintree.DeleteNode(node_key));      // node_key has been added, so it should returns "true"
        ASSERT_FALSE(bintree.DeleteNode(node_key));     // node_key is not in the tree again
    }
}

TEST_F(EmptyTree, FindNode)
{
    for (auto node_key : keys)
    {
        ASSERT_EQ(bintree.FindNode(node_key), nullptr);
        bintree.InsertNode(node_key);
        EXPECT_EQ(bintree.FindNode(node_key)->key, node_key);
        ASSERT_NE(bintree.FindNode(node_key), nullptr);
    }
}

//============================================================================//
// ** ZDE DOPLNTE TESTY **
//
// Zde doplnte testy Red-Black Tree, testujte nasledujici:
// 1. Verejne rozhrani stromu
//    - InsertNode/DeleteNode a FindNode
//    - Chovani techto metod testuje pro neprazdny strom.
// 2. Axiomy (tedy vzdy platne vlastnosti) Red-Black Tree:
//    - Vsechny listove uzly stromu jsou *VZDY* cerne.
//    - Kazdy cerveny uzel muze mit *POUZE* cerne potomky.
//    - Vsechny cesty od kazdeho listoveho uzlu ke koreni stromu obsahuji
//      *STEJNY* pocet cernych uzlu.
//============================================================================//

/*** Konec souboru black_box_tests.cpp ***/
