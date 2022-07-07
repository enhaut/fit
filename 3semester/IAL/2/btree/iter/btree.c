/*
 * Binárny vyhľadávací strom — iteratívna varianta
 *
 * S využitím dátových typov zo súboru btree.h, zásobníkov zo súborov stack.h a
 * stack.c a pripravených kostier funkcií implementujte binárny vyhľadávací
 * strom bez použitia rekurzie.
 */

#include "../btree.h"
#include "stack.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * Inicializácia stromu.
 *
 * Užívateľ musí zaistiť, že incializácia sa nebude opakovane volať nad
 * inicializovaným stromom. V opačnom prípade môže dôjsť k úniku pamäte (memory
 * leak). Keďže neinicializovaný ukazovateľ má nedefinovanú hodnotu, nie je
 * možné toto detegovať vo funkcii.
 */
void bst_init(bst_node_t **tree) {
  *tree = NULL;
}

/*
 * Nájdenie uzlu v strome.
 *
 * V prípade úspechu vráti funkcia hodnotu true a do premennej value zapíše
 * hodnotu daného uzlu. V opačnom prípade funckia vráti hodnotu false a premenná
 * value ostáva nezmenená.
 *
 * Funkciu implementujte iteratívne bez použitia vlastných pomocných funkcií.
 */
bool bst_search(bst_node_t *tree, char key, int *value) {
  while (tree)
  {
    if (tree->key == key)
    {
      *value = tree->value;
      return true;
    }
    tree = (key < tree->key) ? tree->left : tree->right;
  }

  return false;
}

/*
 * Vloženie uzlu do stromu.
 *
 * Pokiaľ uzol so zadaným kľúčom v strome už existuje, nahraďte jeho hodnotu.
 * Inak vložte nový listový uzol.
 *
 * Výsledný strom musí spĺňať podmienku vyhľadávacieho stromu — ľavý podstrom
 * uzlu obsahuje iba menšie kľúče, pravý väčšie.
 *
 * Funkciu implementujte iteratívne bez použitia vlastných pomocných funkcií.
 */
void bst_insert(bst_node_t **tree, char key, int value) {
  bst_node_t *root = *tree;  // store the root of tree
  bst_node_t *node = (bst_node_t *)malloc(sizeof(bst_node_t));
  if (!node)
    return;  // could not allocate memory for node

  node->key = key;
  node->value = value;
  node->right = NULL;
  node->left = NULL;

  if (!(*tree))  // inserting node would become the root
    *tree = node;  // set the node as a root
  else{
    bst_node_t **iter = tree;
    while (*iter)  // find the parent for new child
    {
      if (((*tree)->key > key && (!(*tree)->left)) ||
          (((*tree)->key < key) && (!(*tree)->right)) ||
          ((*tree)->key == key))
        break;

      *iter = ((*tree)->key > key) ? ((*tree)->left) : ((*tree)->right);
    }

    if ((*iter)->key == key)
    {
      free(node);
      (*iter)->value = value;
    }
    else {
      if ((*iter)->key > key)
        (*iter)->left = node;
      else
        (*iter)->right = node;

      *tree = root;  // set the root back, it is moved in this function
      // I know, it is not the best solution, but I am lazy to refactor this
    }
  }
}

/*
 * Pomocná funkcia ktorá nahradí uzol najpravejším potomkom.
 *
 * Kľúč a hodnota uzlu target budú nahradené kľúčom a hodnotou najpravejšieho
 * uzlu podstromu tree. Najpravejší potomok bude odstránený. Funkcia korektne
 * uvoľní všetky alokované zdroje odstráneného uzlu.
 *
 * Funkcia predpokladá že hodnota tree nie je NULL.
 *
 * Táto pomocná funkcia bude využitá pri implementácii funkcie bst_delete.
 *
 * Funkciu implementujte iteratívne bez použitia vlastných pomocných funkcií.
 */
void bst_replace_by_rightmost(bst_node_t *target, bst_node_t **tree) {
  if (!target || !(*tree))
    return;

 bst_node_t *prev = *tree;
 bst_node_t *to_delete = (*tree)->left;

 while (to_delete->right)
 {
   prev = to_delete;
   to_delete = to_delete->right;
 }
 if (prev == target && prev->right)  // parent is the target, but parent
   bst_dispose(&(prev->right));

 prev->right = to_delete->left;

 target->key = to_delete->key;
 target->value = to_delete->value;
 if (target->left == to_delete)
   target->left = NULL;


  free(to_delete);
}

/*
 * Odstránenie uzlu v strome.
 *
 * Pokiaľ uzol so zadaným kľúčom neexistuje, funkcia nič nerobí.
 * Pokiaľ má odstránený uzol jeden podstrom, zdedí ho otec odstráneného uzla.
 * Pokiaľ má odstránený uzol oba podstromy, je nahradený najpravejším uzlom
 * ľavého podstromu. Najpravejší uzol nemusí byť listom!
 * Funkcia korektne uvoľní všetky alokované zdroje odstráneného uzlu.
 *
 * Funkciu implementujte iteratívne pomocou bst_replace_by_rightmost a bez
 * použitia vlastných pomocných funkcií.
 */
void bst_delete(bst_node_t **tree, char key) {
 if (!(*tree) || !key)
   return;

 bst_node_t *prev = *tree;
 bst_node_t *to_delete = *tree;

 while (to_delete)
 {
   if (to_delete->key == key)
     break;

   prev = to_delete;
   to_delete = (to_delete->key > key) ? (to_delete->left) : (to_delete->right);
 }
 if (!to_delete)
   return;  // requested key does not exist

 bst_node_t *orphans = NULL;

 if (to_delete->left && to_delete->right)
 {
   bst_replace_by_rightmost(to_delete, &(to_delete));
   return;
 } else if (to_delete->left)
   orphans = to_delete->left;
 else if (to_delete->right)
   orphans = to_delete->right;

 if (prev && prev->right == to_delete)
   prev->right = orphans;
 else if (prev && prev->left == to_delete)
   prev->left = orphans;

 if (*tree == to_delete)  // removing root
 {
   if (to_delete->right)
     bst_dispose(&(to_delete->right));
   if (to_delete->left)
     bst_dispose(&(to_delete->left));
 }
 if (*tree == to_delete)
   *tree = NULL;  // deleted item is the only one remaining in the tree
 free(to_delete);
}


/*
 * Zrušenie celého stromu.
 *
 * Po zrušení sa celý strom bude nachádzať v rovnakom stave ako po
 * inicializácii. Funkcia korektne uvoľní všetky alokované zdroje rušených
 * uzlov.
 *
 * Funkciu implementujte iteratívne pomocou zásobníku uzlov a bez použitia
 * vlastných pomocných funkcií.
 */
void bst_dispose(bst_node_t **tree) {
 if (!(*tree))
   return;
 stack_bst_t stack;
 stack_bst_init(&stack);

  do {
    if (!(*tree) && !stack_bst_empty(&stack))
    {
      *tree = stack_bst_top(&stack);
      stack_bst_pop(&stack);
    }else if ((*tree)){
      if ((*tree)->right)
        stack_bst_push(&stack, (*tree)->right);

      bst_node_t *tmp = *tree;
      *tree = (*tree)->left;
      free(tmp);
    }
  } while ((*tree) || !stack_bst_empty(&stack));

  bst_init(tree);
}

/*
 * Pomocná funkcia pre iteratívny preorder.
 *
 * Prechádza po ľavej vetve k najľavejšiemu uzlu podstromu.
 * Nad spracovanými uzlami zavola bst_print_node a uloží ich do zásobníku uzlov.
 *
 * Funkciu implementujte iteratívne pomocou zásobníku uzlov a bez použitia
 * vlastných pomocných funkcií.
 */
void bst_leftmost_preorder(bst_node_t *tree, stack_bst_t *to_visit) {
  while (tree)
  {
    stack_bst_push(to_visit, tree);
    bst_print_node(tree);

    tree = tree->left;
  }
}

/*
 * Preorder prechod stromom.
 *
 * Pre aktuálne spracovávaný uzol nad ním zavolajte funkciu bst_print_node.
 *
 * Funkciu implementujte iteratívne pomocou funkcie bst_leftmost_preorder a
 * zásobníku uzlov bez použitia vlastných pomocných funkcií.
 */
void bst_preorder(bst_node_t *tree) {
  stack_bst_t stack;
  stack_bst_init(&stack);

  bst_leftmost_preorder(tree, &stack);

  while (!stack_bst_empty(&stack))
  {
    tree = stack_bst_top(&stack);
    stack_bst_pop(&stack);
    bst_leftmost_preorder(tree->right, &stack);
  }
}

/*
 * Pomocná funkcia pre iteratívny inorder.
 *
 * Prechádza po ľavej vetve k najľavejšiemu uzlu podstromu a ukladá uzly do
 * zásobníku uzlov.
 *
 * Funkciu implementujte iteratívne pomocou zásobníku uzlov a bez použitia
 * vlastných pomocných funkcií.
 */
void bst_leftmost_inorder(bst_node_t *tree, stack_bst_t *to_visit) {
  while (tree)
  {
    stack_bst_push(to_visit, tree);
    tree = tree->left;
  }
}

/*
 * Inorder prechod stromom.
 *
 * Pre aktuálne spracovávaný uzol nad ním zavolajte funkciu bst_print_node.
 *
 * Funkciu implementujte iteratívne pomocou funkcie bst_leftmost_inorder a
 * zásobníku uzlov bez použitia vlastných pomocných funkcií.
 */
void bst_inorder(bst_node_t *tree) {
    stack_bst_t stack;
    stack_bst_init(&stack);

    bst_leftmost_inorder(tree, &stack);

    while (!stack_bst_empty(&stack))
    {
      tree = stack_bst_top(&stack);
      stack_bst_pop(&stack);
      bst_print_node(tree);
      bst_leftmost_inorder(tree->right, &stack);
    }
}

/*
 * Pomocná funkcia pre iteratívny postorder.
 *
 * Prechádza po ľavej vetve k najľavejšiemu uzlu podstromu a ukladá uzly do
 * zásobníku uzlov. Do zásobníku bool hodnôt ukladá informáciu že uzol
 * bol navštívený prvý krát.
 *
 * Funkciu implementujte iteratívne pomocou zásobníkov uzlov a bool hodnôt a bez použitia
 * vlastných pomocných funkcií.
 */
void bst_leftmost_postorder(bst_node_t *tree, stack_bst_t *to_visit,
                            stack_bool_t *first_visit) {
  while (tree)
  {
    stack_bst_push(to_visit, tree);
    stack_bool_push(first_visit, true);
    tree = tree->left;
  }
}

/*
 * Postorder prechod stromom.
 *
 * Pre aktuálne spracovávaný uzol nad ním zavolajte funkciu bst_print_node.
 *
 * Funkciu implementujte iteratívne pomocou funkcie bst_leftmost_postorder a
 * zásobníkov uzlov a bool hodnôt bez použitia vlastných pomocných funkcií.
 */
void bst_postorder(bst_node_t *tree) {
  bool from;
  stack_bst_t stack;
  stack_bool_t boolStack;

  stack_bst_init(&stack);
  stack_bool_init(&boolStack);

  bst_leftmost_postorder(tree, &stack, &boolStack);

  while (!stack_bst_empty(&stack))
  {
    tree = stack_bst_top(&stack);
    from = stack_bool_top(&boolStack);
    stack_bool_pop(&boolStack);

    if (from)
    {
      stack_bool_push(&boolStack, false);
      bst_leftmost_postorder(tree->right, &stack, &boolStack);
    }else{
      stack_bst_pop(&stack);
      bst_print_node(tree);
    }
  }
}
