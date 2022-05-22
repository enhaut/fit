//======== Copyright (c) 2021, FIT VUT Brno, All rights reserved. ============//
//
// Purpose:     Test Driven Development - priority queue code
//
// $NoKeywords: $ivs_project_1 $tdd_code.cpp
// $Author:     JMENO PRIJMENI <xlogin00@stud.fit.vutbr.cz>
// $Date:       $2021-01-04
//============================================================================//
/**
 * @file tdd_code.cpp
 * @author JMENO PRIJMENI
 * 
 * @brief Implementace metod tridy prioritni fronty.
 */

#include <stdlib.h>
#include <stdio.h>

#include "tdd_code.h"

//============================================================================//
// ** ZDE DOPLNTE IMPLEMENTACI **
//
// Zde doplnte implementaci verejneho rozhrani prioritni fronty (Priority Queue)
// 1. Verejne rozhrani fronty specifikovane v: tdd_code.h (sekce "public:")
//    - Konstruktor (PriorityQueue()), Destruktor (~PriorityQueue())
//    - Metody Insert/Remove/Find a GetHead
//    - Pripadne vase metody definovane v tdd_code.h (sekce "protected:")
//
// Cilem je dosahnout plne funkcni implementace prioritni fronty implementovane
// pomoci tzv. "double-linked list", ktera bude splnovat dodane testy 
// (tdd_tests.cpp).
//============================================================================//

PriorityQueue::PriorityQueue()
{
    m_pHead = nullptr;
}

PriorityQueue::~PriorityQueue()
{
    Element_t *to_delete;
    while ((to_delete = GetHead()))
        Remove(to_delete->value);
    m_pHead = nullptr;
}

void PriorityQueue::Insert(int value)
{
    auto *new_element = new Element_t{nullptr, value};

    Element_t *actual = GetHead();
    if (!actual)
        m_pHead = new_element;

    Element_t *before = nullptr;
    while (actual)
    {
        if (value > actual->value)
        {
            new_element->pNext = actual;
            if (!before)    // actual item is the first one
                m_pHead = new_element;
            else
                before->pNext = new_element;
            break;
        }else if (value < actual->value && !actual->pNext)
        {
            actual->pNext = new_element;
            break;
        }
        before = actual;
        actual = actual->pNext;
    }
}

bool PriorityQueue::Remove(int value)
{
    Element_t *before = nullptr;
    Element_t *actual = GetHead();

    while (actual)
    {
        if (actual->value == value)
        {
            if (!before)    // actual item is the first one
                m_pHead = actual->pNext;
            else
                before->pNext = actual->pNext;

            delete actual;
            return true;
        }
        before = actual;
        actual = actual->pNext;
    }

    return false;
}

PriorityQueue::Element_t *PriorityQueue::Find(int value)
{
    Element_t *next = GetHead();

    while (next)
    {
        if (next->value == value)
            break;

        next = next->pNext;
    }
    return next;
}

size_t PriorityQueue::Length()
{
    int length = 0;
    Element_t *next_element = GetHead();

    while (next_element)
    {
        length++;
        next_element = next_element->pNext;
    }

	return length;
}

PriorityQueue::Element_t *PriorityQueue::GetHead()
{
    return m_pHead;
}

/*** Konec souboru tdd_code.cpp ***/
