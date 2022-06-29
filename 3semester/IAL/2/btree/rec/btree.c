/*
 * Binárny vyhľadávací strom — rekurzívna varianta
 *
 * S využitím dátových typov zo súboru btree.h a pripravených kostier funkcií
 * implementujte binárny vyhľadávací strom pomocou rekurzie.
 */

#include "../btree.h"
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
 * Funkciu implementujte rekurzívne bez použitia vlastných pomocných funkcií.
 */
bool bst_search(bst_node_t *tree, char key, int *value) {
  if (!tree)
    return false;

  if (tree->key == key)
  {
    if (value)
      *value = tree->value;
    return true;
  }else{
    bst_node_t *search_in = (key < tree->key) ? tree->left : tree->right;
    return bst_search(search_in, key, value);
  }
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
 * Funkciu implementujte rekurzívne bez použitia vlastných pomocných funkcií.
 */
void bst_insert(bst_node_t **tree, char key, int value) {
  if (!(*tree))  // leaf node found
  {
    (*tree) = (bst_node_t *)malloc(sizeof(bst_node_t));
    if (!(*tree))
      return; // could not allocate memory for node

    (*tree)->key = key;
    (*tree)->value = value;

    (*tree)->left = NULL;
    (*tree)->right = NULL;
  }else if ((*tree)->key == key)
    (*tree)->value = value;
  else
  {
    bst_node_t **insert_to = ((*tree)->key > key) ? &((*tree)->left) : &((*tree)->right);
    bst_insert(insert_to, key, value);
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
 * Funkciu implementujte rekurzívne bez použitia vlastných pomocných funkcií.
 */
void bst_replace_by_rightmost(bst_node_t *target, bst_node_t **tree) {
  if (!target || !(*tree))
    return;

  if ((*tree)->right)  // current node is not right most
    bst_replace_by_rightmost(target, &((*tree)->right));
  else  // node is right most
  {
    target->key = (*tree)->key;
    target->value = (*tree)->value;

    bst_node_t *left = (*tree)->left;
    free(*tree);  // this node could not have right child
    *tree = left;  // the left child is "inherited"
  }
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
 * Funkciu implementujte rekurzívne pomocou bst_replace_by_rightmost a bez
 * použitia vlastných pomocných funkcií.
 */
void bst_delete(bst_node_t **tree, char key) {
  if (!(*tree))
    return;

  if ((*tree)->key != key)  // node is not the corresponding one to delete
  {
    bst_node_t **to_delete = ((*tree)->key > key) ? &((*tree)->left) : &((*tree)->right);
    bst_delete(to_delete, key);
  } else {
    if ((*tree)->left && (*tree)->right)  // node has both children
      bst_replace_by_rightmost(*tree, &((*tree)->left));
    else
    {
      bst_node_t *next_node = NULL;
      if ((*tree)->left) // node has left child without right
        next_node = (*tree)->left;
      else if ((*tree)->right) // node has right child without left
        next_node = (*tree)->right;
      else
        next_node = NULL;  // node has no children

      free(*tree);
      *tree = next_node;
    }
  }
}

/*
 * Zrušenie celého stromu.
 *
 * Po zrušení sa celý strom bude nachádzať v rovnakom stave ako po
 * inicializácii. Funkcia korektne uvoľní všetky alokované zdroje rušených
 * uzlov.
 *
 * Funkciu implementujte rekurzívne bez použitia vlastných pomocných funkcií.
 */
void bst_dispose(bst_node_t **tree) {
  if (!(*tree))  // stop the recursion at the leaves
    return;

  bst_dispose(&((*tree)->left));
  bst_dispose(&((*tree)->right));
  free(*tree);
  *tree = NULL;
}

/*
 * Preorder prechod stromom.
 *
 * Pre aktuálne spracovávaný uzol nad ním zavolajte funkciu bst_print_node.
 *
 * Funkciu implementujte rekurzívne bez použitia vlastných pomocných funkcií.
 */
void bst_preorder(bst_node_t *tree) {
  if (!tree)
    return;

  bst_print_node(tree);
  bst_preorder(tree->left);
  bst_preorder(tree->right);
}

/*
 * Inorder prechod stromom.
 *
 * Pre aktuálne spracovávaný uzol nad ním zavolajte funkciu bst_print_node.
 *
 * Funkciu implementujte rekurzívne bez použitia vlastných pomocných funkcií.
 */
void bst_inorder(bst_node_t *tree) {
  if (!tree)
    return;

  bst_inorder(tree->left);
  bst_print_node(tree);
  bst_inorder(tree->right);
}
/*
 * Postorder prechod stromom.
 *
 * Pre aktuálne spracovávaný uzol nad ním zavolajte funkciu bst_print_node.
 *
 * Funkciu implementujte rekurzívne bez použitia vlastných pomocných funkcií.
 */
void bst_postorder(bst_node_t *tree) {
  if (!tree)
    return;

  bst_postorder(tree->left);
  bst_postorder(tree->right);
  bst_print_node(tree);
}
