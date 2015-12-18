#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <sysexits.h>
#include "bpt.h"
#include "lists.h"

/*
 *  All bpt_nodes have an array of keys, and an array of values.
 *  In non-leaf bpt_nodes, values[i] points to the bpt_node that is either
 *  greater than or equal to key[i-1] or less than key[i].
 */

void (*repair)(struct bpt_node *, struct bpt_node *) = NULL;
void (*append)(void *, void **) = NULL;
uint32_t ORDER = 4;

//{{{ struct bpt_node *bpt_to_node(void *n)
struct bpt_node *bpt_to_node(void *n)
{
    return (struct bpt_node*)n;
}
//}}}

//{{{int b_search(int key, int *D, int D_size)
int b_search(uint32_t key, uint32_t *D, uint32_t D_size)
{
    int lo = -1, hi = D_size, mid;
    while ( hi - lo > 1) {
        mid = (hi + lo) / 2;
        if ( D[mid] < key )
            lo = mid;
        else
            hi = mid;
    }

    return hi;
}
//}}}

//{{{ struct bpt_node *bpt_new_node()
struct bpt_node *bpt_new_node()
{
    struct bpt_node *n = (struct bpt_node *)malloc(sizeof(struct bpt_node));
    n->parent = NULL;
    n->keys = (uint32_t *) 
            malloc( (ORDER+1) * sizeof(uint32_t)); //adding one help bpt_insert
    n->num_keys = 0;
    n->is_leaf = 0;
    n->flags = 0;
    n->pointers = (void **) malloc((ORDER+2) * sizeof(void *));
    n->next = NULL;
    n->leading = NULL;

    return n;
}
//}}}

//{{{ struct bpt_node *bpt_find_leaf(struct bpt_node *curr, int key)
struct bpt_node *bpt_find_leaf(struct bpt_node *curr, uint32_t key)
{
    if (curr == NULL)
        return NULL;

    while (curr->is_leaf != 1) {
        int i = bpt_find_insert_pos(curr, key);
        if ((i < curr->num_keys) && (curr->keys[i] == key))
            i+=1;
        curr = (struct bpt_node *)curr->pointers[i];
    }
    return curr;
}
//}}}

//{{{int bpt_find_insert_pos(struct bpt_node *leaf, int key)
int bpt_find_insert_pos(struct bpt_node *leaf, uint32_t key)
{
    return b_search(key, leaf->keys, leaf->num_keys);
}
//}}}

//{{{struct bpt_node *bpt_place_new_key_value(struct bpt_node *root,
struct bpt_node *bpt_place_new_key_value(struct bpt_node *root,
                                         struct bpt_node **target_bpt_node,
                                         int *target_key_pos,
                                         uint32_t key,
                                         void *value)
{
    int bpt_insert_key_pos = bpt_find_insert_pos(*target_bpt_node, key);

    int bpt_insert_value_pos = bpt_insert_key_pos;

    if ((*target_bpt_node)->is_leaf == 0)
        bpt_insert_value_pos += 1;

    if ((*target_bpt_node)->is_leaf == 1)
        *target_key_pos = bpt_insert_key_pos;


    if (((*target_bpt_node)->is_leaf == 1) &&
         (*target_key_pos < ((*target_bpt_node)->num_keys)) &&
        ((*target_bpt_node)->keys[*target_key_pos] == key )) {

        if (append != NULL)
            append(value, &((*target_bpt_node)->pointers[*target_key_pos]));
        else
            (*target_bpt_node)->pointers[*target_key_pos] = value;

        return root;
    }

    // move everything over
    int i;

#if DEBUG
    fprintf(stderr, "bpt_place_new_key_value:\t"
                    "target_bpt_node:%p\t"
                    "target_bpt_node->num_keys:%d\t"
                    "bpt_insert_key_pos:%d\t"
                    "target_bpt_node->is_leaf:%d\n",
                    *target_bpt_node,
                    (*target_bpt_node)->num_keys,
                    bpt_insert_key_pos,
                    (*target_bpt_node)->is_leaf
                    );
#endif

    for (i = (*target_bpt_node)->num_keys; i > bpt_insert_key_pos; --i) {
#if DEBUG
        fprintf(stderr, "bpt_place_new_key_value:\ti:%d\tk:%d\n", 
                        i,
                        (*target_bpt_node)->keys[i-1]);
#endif
        (*target_bpt_node)->keys[i] = (*target_bpt_node)->keys[i-1];
    }

    if ((*target_bpt_node)->is_leaf == 1) {
        for (i = (*target_bpt_node)->num_keys; i > bpt_insert_value_pos; --i) 
            (*target_bpt_node)->pointers[i] = (*target_bpt_node)->pointers[i-1];
    } else {
        for (i = (*target_bpt_node)->num_keys+1; i > bpt_insert_value_pos; --i) {

#if DEBUG
            fprintf(stderr, "bpt_place_new_key_value:\ti:%d\tv:%p\n", 
                            i,
                            (*target_bpt_node)->pointers[i-1]);
#endif

            (*target_bpt_node)->pointers[i] = (*target_bpt_node)->pointers[i-1];
        }
    }


#if DEBUG
    fprintf(stderr, "bpt_place_new_key_value:\ti:%d\tkey:%d\n",
                    bpt_insert_key_pos,
                    key);
    fprintf(stderr, "bpt_place_new_key_value:\ti:%d\tvalue:%p\n",
                    bpt_insert_value_pos,
                    value);
#endif

    (*target_bpt_node)->keys[bpt_insert_key_pos] = key;
    (*target_bpt_node)->pointers[bpt_insert_value_pos] = value;

    (*target_bpt_node)->num_keys += 1;

    // If there are now too many values in the bpt_node, split it
    if ((*target_bpt_node)->num_keys > ORDER) {
        struct bpt_node *lo_result_bpt_node = NULL, *hi_result_bpt_node = NULL;
        int lo_hi_split_point = 0;
        struct bpt_node *r = bpt_split_node(root,
                                            *target_bpt_node,
                                            &lo_result_bpt_node,
                                            &hi_result_bpt_node,
                                            &lo_hi_split_point,
                                            repair);
        if ((*target_bpt_node)->is_leaf) {
            if (bpt_insert_key_pos < lo_hi_split_point)
                *target_bpt_node = lo_result_bpt_node;
            else {
                *target_bpt_node = hi_result_bpt_node;
                *target_key_pos = bpt_insert_key_pos - lo_hi_split_point;
            }
        }

        return r;
    } else {
        return root;
    }
}
//}}}

//{{{struct bpt_node *bpt_split_node(struct bpt_node *root, struct bpt_node *bpt_node)
struct bpt_node *bpt_split_node(struct bpt_node *root,
                        struct bpt_node *bpt_node,
                        struct bpt_node **lo_result_bpt_node,
                        struct bpt_node **hi_result_bpt_node,
                        int *lo_hi_split_point,
                        void (*repair)(struct bpt_node *, struct bpt_node *))
{

#if DEBUG
    fprintf(stderr, "bpt_split_node():\n");
#endif

    struct bpt_node *n = bpt_new_node();

#if DEBUG
    fprintf(stderr, "bpt_split_node():\tbpt_new_node:%p\n", n);
#endif

    // set the split location
    int split_point = (ORDER + 1)/2;

    *lo_result_bpt_node = bpt_node;
    *hi_result_bpt_node = n;
    *lo_hi_split_point = split_point;

    // copy the 2nd 1/2 of the values over to the new bpt_node
    int bpt_node_i, new_bpt_node_i = 0;
    for (bpt_node_i = split_point; bpt_node_i < bpt_node->num_keys; ++bpt_node_i) {
        n->keys[new_bpt_node_i] = bpt_node->keys[bpt_node_i];
        n->pointers[new_bpt_node_i] = bpt_node->pointers[bpt_node_i];
        n->num_keys += 1;
        new_bpt_node_i += 1;
    }

    // if the bpt_node is not a leaf, the far right pointer must be coppied too
    if (bpt_node->is_leaf == 0) {
        n->pointers[new_bpt_node_i] = bpt_node->pointers[bpt_node_i];
        n->pointers[0] = NULL;
    }

    // set status of new bpt_node
    n->is_leaf = bpt_node->is_leaf;
    n->parent = bpt_node->parent;

    bpt_node->num_keys = split_point;

    if (bpt_node->is_leaf == 0) {
#if DEBUG
        fprintf(stderr, "bpt_split_node():\tsplit non-leaf\n");
#endif
        // if the bpt_node is not a leaf, then update the parent pointer of the 
        // children
        for (bpt_node_i = 1; bpt_node_i <= n->num_keys; ++bpt_node_i) 
            ( (struct bpt_node *)n->pointers[bpt_node_i])->parent = n;
    } else {
        // if the bpt_node is a leaf, then connect the two
        n->next = bpt_node->next;
        bpt_node->next = n;

#if DEBUG
        fprintf(stderr,
                "bpt_split_node():\tsplit leaf old->next:%p new->next:%p\n",
                bpt_node->next,
                n->next);
#endif
    }

    if (repair != NULL) {
        repair(bpt_node, n);
    }

    if (bpt_node == root) {
#if DEBUG
            fprintf(stderr, "bpt_split_node():\tsplit root\tk:%d\n", n->keys[0]);
#endif

        // if we just split the root, create a new root witha single value
        struct bpt_node *new_root = bpt_new_node();
        new_root->is_leaf = 0;
        new_root->keys[0] = n->keys[0];
        new_root->pointers[0] = (void *)bpt_node; 
        new_root->pointers[1] = (void *)n; 
        new_root->num_keys += 1;
        bpt_node->parent = new_root;
        n->parent = new_root;
        return new_root;
    } else {
#if DEBUG
            fprintf(stderr, "bpt_split_node():\tsplit non-root\n");
#endif
        // if we didnt split the root, place the new value in the parent bpt_node
        return bpt_place_new_key_value(root,
                                       &(bpt_node->parent),
                                       NULL,
                                       n->keys[0],
                                       (void *)n);
    }
}
//}}}

//{{{struct bpt_node *bpt_insert(struct bpt_node *root, int key, void *value)
struct bpt_node *bpt_insert(struct bpt_node *root,
                            uint32_t key,
                            void *value,
                            struct bpt_node **leaf,
                            int *pos)
{

#if DEBUG
    fprintf(stderr, "bpt_insert():\tk:%d\n", key);
#endif

    if (root == NULL) {
        root = bpt_new_node();
        root->is_leaf = 1;
        root->keys[0] = key;
        root->pointers[0] = value;
        root->num_keys += 1;

        *leaf = root;
        *pos = 0;

        return root;
    } else {
        *leaf = bpt_find_leaf(root, key);
#if DEBUG
        fprintf(stderr, "bpt_insert():\tleaf:%p\n", *leaf);
#endif
        root = bpt_place_new_key_value(root,
                                   leaf,
                                   pos,
                                   key,
                                   value);
        return root;
    }
}
//}}}

//{{{void bpt_print_tree(struct bpt_node *curr, int level)
void bpt_print_tree(struct bpt_node *curr, int level)
{
    int i;
    for (i = 0; i < level; ++i)
        printf(" ");

    printf("keys %p(%d)(%d):", curr,curr->num_keys, curr->is_leaf);

    for (i = 0; i < curr->num_keys; ++i)
        printf(" %d", curr->keys[i]);
    printf("\n");

    for (i = 0; i < level; ++i)
        printf(" ");
    printf("values:");

    if (curr->is_leaf == 0) {
        for (i = 0; i <= curr->num_keys; ++i)
            printf(" %p", (struct bpt_node *)curr->pointers[i]);
    } else {
        for (i = 0; i < curr->num_keys; ++i) {
            int *v = (int *)curr->pointers[i];
            printf(" %d", *v);
        }
    }
    printf("\n");

    if (curr->is_leaf == 0) {
        for (i = 0; i <= curr->num_keys; ++i)
            bpt_print_tree((struct bpt_node *)curr->pointers[i], level+1);
    }
}
//}}}

//{{{ void bpt_print_node(struct bpt_node *n)
void bpt_print_node(struct bpt_node *n)
{
    int i;
    if (n->is_leaf)
        printf("+");
    else
        printf("-");
    printf("\t%p\t%p\t",n, n->parent);

    for (i = 0; i < n->num_keys; ++i) {
        if (i!=0)
            printf(" ");
        printf("%u", n->keys[i]);
    }
    printf("\n");
}
//}}}

//{{{void print_values(struct bpt_node *root)
void print_values(struct bpt_node *root)
{
    struct bpt_node *curr = root;
    while (curr->is_leaf != 1) {
        curr = (struct bpt_node *)curr->pointers[0];
    }

    printf("values: ");
    int i, *v;
    while (1) {
        for (i = 0; i < curr->num_keys; ++i) {
            v = (int *)curr->pointers[i];
            printf(" %d", *v);
        }
        if (curr->next == NULL)
            break;
        else
            curr = curr->next;
    }
    printf("\n");
}
//}}}

//{{{ void *bpt_find(struct bpt_node *root, struct bpt_node **leaf, uint32_t
void *bpt_find(struct bpt_node *root,
               struct bpt_node **leaf,
               int *pos,
               uint32_t key) 
{
#if DEBUG
    fprintf(stderr, "bpt_find\n");
#endif

    if (root == NULL)
        return NULL;

    *leaf = bpt_find_leaf(root, key);
    int bpt_insert_key_pos = bpt_find_insert_pos(*leaf, key);

#if DEBUG
    fprintf(stderr,
            "key:%d pos:%d num_key:%d\n",
            key,
            bpt_insert_key_pos,
            (*leaf)->num_keys);
#endif

    *pos = bpt_insert_key_pos;
    if ((bpt_insert_key_pos + 1) > (*leaf)->num_keys) 

        return NULL;
    else if (key != (*leaf)->keys[bpt_insert_key_pos])
        return NULL;
    else {
        return (*leaf)->pointers[bpt_insert_key_pos];
    }
}
//}}}

//{{{ void bpt_destroy_tree(struct bpt_node **curr)
void bpt_destroy_tree(struct bpt_node **curr)
{
#if 1
    if (*curr == NULL) {
        return;
    } else if ((*curr)->is_leaf == 1) {
        free((*curr)->keys);
        free((*curr)->pointers);
        free(*curr);
        *curr = NULL;
        return;
    } else {
        uint32_t i;
        for (i = 0; i <= (*curr)->num_keys; ++i) 
            bpt_destroy_tree((struct bpt_node **)&((*curr)->pointers[i]));
        free((*curr)->keys);
        free((*curr)->pointers);
        free(*curr);
        *curr = NULL;
        return;
    } 
#endif
}
//}}}

//{{{void bpt_write_tree(struct bpt_node *root,
void bpt_write_tree(struct bpt_node *root,
                    FILE *f,
                    struct ordered_set **addr_to_id,
                    struct indexed_list **id_to_offset_size,
                    long *table_offset)
{
    /*
     * Two ordered sets are used to help managage the serialization.  One is a
     * mapping from memory address to id (addr_to_id), and the other is from id
     * to file offset (id_to_offset).  First the tree is processed using BFS,
     * and the memory address of each not, its pointers, and its leading value
     * are added to the addr_to_id table.  The tree is then processed again
     * using BFS to store the values to file.  Within each node the pointers
     * are replaced with ids from the addr_to_id before saving.  Leaves of the
     * tree contain information that is not saved to disk with this function.
     * These extra values are in the addr_to_id set that need to be added to 
     * id_to_offset table then written to the file at the table_offset offset.
     *
     * The first value in the file is the order, then number of values in the
     * addr_to_id set, and then followed by the nodes
     *
     * Each node is stored in the following order:
     *  is_leaf     -> uin32_t
     *  parent      -> uint32_t
     *  num_keys    -> uin32_t
     *  keys        -> ORDER * uint32_t
     *  pointers    -> (ORDER+1) * uint32_t
     *  next        -> uint32_t
     *  leading     -> uint32_t
     *
     *  Total size is:
     *  (7+ORDER+ORDER+1) * uint32_t -> 8+2*ORDER
     */

    // make table of memory addr -> id
    struct pointer_uint_pair *pu_p, *pu_r;
    *addr_to_id = ordered_set_init(50,
                                   pointer_uint_pair_sort_element_cmp,
                                   pointer_uint_pair_search_element_cmp,
                                   pointer_uint_pair_search_key_cmp);

    uint32_t i, count = 1;

    // Do a BFS search of the tree to make a table of memaddr to ID
    struct fifo_q *q = NULL;
    fifo_q_push(&q, root);
    while (fifo_q_peek(q) != NULL) {
        // Add the current node to the table
        struct bpt_node *curr = (struct bpt_node *)fifo_q_pop(&q);

        pu_p = (struct pointer_uint_pair *)
                malloc(sizeof(struct pointer_uint_pair));
        pu_p->pointer = curr;
        pu_p->uint = count;
        //fprintf(stderr, "%u %p\n", pu_p->uint, pu_p->pointer);
        pu_r = (struct pointer_uint_pair *)
                ordered_set_add(*addr_to_id, pu_p);

        count += 1;

        if (curr->is_leaf == 0) {
            // If the node is not a leaf, put all of its children on the q
            for (i = 0; i <= curr->num_keys; ++i) {
                if (curr->pointers[i] != NULL)
                    fifo_q_push(&q, curr->pointers[i]);
            }
        } else {
            // If it is not a leaf, put the data elements on the q
            for (i = 0; i < curr->num_keys; ++i) {
                if (curr->pointers[i] != NULL) {
                    pu_p = (struct pointer_uint_pair *)
                        malloc(sizeof(struct pointer_uint_pair));
                    pu_p->pointer = curr->pointers[i];
                    pu_p->uint = count;
                    //fprintf(stderr, "%u %p\n", pu_p->uint, pu_p->pointer);
                    pu_r = (struct pointer_uint_pair *)
                            ordered_set_add(*addr_to_id, pu_p);
                    count += 1;
                }
            }
            // If it is not a leaf, put the leading data on the que
            if (curr->leading != NULL) {
                pu_p = (struct pointer_uint_pair *)
                        malloc(sizeof(struct pointer_uint_pair));
                pu_p->pointer = curr->leading;
                pu_p->uint = count;
                //fprintf(stderr, "%u %p\n", pu_p->uint, pu_p->pointer);
                pu_r = (struct pointer_uint_pair *)
                        ordered_set_add(*addr_to_id, pu_p);
                count += 1;
            }
        }
    }

    long top_offset = ftell(f);

    //write ORDER
    if (fwrite(&(ORDER), sizeof(uint32_t), 1, f) != 1)
        err(EX_IOERR, "Error writing id to offset table size to file");

    //write number of elements in the table
    if (fwrite(&((*addr_to_id)->num), sizeof(uint32_t), 1, f) != 1)
        err(EX_IOERR, "Error writing id to offset table size to file");

    //make room in the file for the id -> offset table
    char *zero = (char *)calloc(
            (*addr_to_id)->num *(2*sizeof(uint32_t) + sizeof(long)),
            sizeof(char));
    *table_offset = ftell(f);
    if (fwrite(zero,
               sizeof(char),
               (*addr_to_id)->num *(2*sizeof(uint32_t) + sizeof(long)),
               f) != (*addr_to_id)->num *(2*sizeof(uint32_t) + sizeof(long)))
        err(EX_IOERR, "Error writing id to offset table to file");
    free(zero);

    // make table of id -> file offset
    struct uint_offset_size_pair *uo_p, *uo_r;

    /*
    *id_to_offset_size = ordered_set_init(50,
                                     uint_offset_size_pair_sort_element_cmp,
                                     uint_offset_size_pair_search_element_cmp,
                                     uint_offset_size_pair_search_key_cmp);
    */
    *id_to_offset_size = indexed_list_init(50, sizeof(struct offset_size_pair));

    struct byte_array *ba = byte_array_init((8+2*ORDER) * sizeof(uint32_t));

    // Use BFS to then write the nodes to disk, replacing pointers with ids
    // from the pointer/id table
    q = NULL;
    fifo_q_push(&q, root);
    while (fifo_q_peek(q) != NULL) {
        // "clear" out the byte_array
        ba->num = 0;
        struct bpt_node *curr = (struct bpt_node *)fifo_q_pop(&q);

        // get id from pointer/is table
        pu_r = (struct pointer_uint_pair *)ordered_set_get(*addr_to_id, curr);
        uint32_t curr_id = pu_r->uint;

        //serialize
        // is_leaf
        byte_array_append(ba, &(curr->is_leaf), sizeof(uint32_t));
        //fprintf(stderr, "is_leaf:%u\t%u\n", curr_id, ba->num);
   
        // converted parent
        // get parent id 
        if (curr->parent != NULL) {
            pu_r = (struct pointer_uint_pair *)
                ordered_set_get(*addr_to_id, curr->parent);
            uint32_t parent_id = pu_r->uint;
            byte_array_append(ba, &(parent_id), sizeof(uint32_t));
            //fprintf(stderr, "parent:%u\t%u\n", curr_id, ba->num);
        } else {
            byte_array_append_zeros(ba, sizeof(uint32_t));
            //fprintf(stderr, "parent:%u\t%u\n", curr_id, ba->num);
        }

        // num_keys
        byte_array_append(ba, &(curr->num_keys), sizeof(uint32_t));
        //fprintf(stderr, "num_keys:%u\t%u\n", curr_id, ba->num);
       
        // keys
        byte_array_append(ba,
                          curr->keys,
                          curr->num_keys*sizeof(uint32_t));
        //fprintf(stderr, "keys:%u\t%u\n", curr_id, ba->num);

        //fprintf(stderr, "curr->num_keys:%u\n", curr->num_keys);
        
        // converted pointers values
        for (i = 0;
             i <= (curr->is_leaf ? curr->num_keys - 1 : curr->num_keys);
             ++i) {

            if (curr->pointers[i] != NULL) {
                pu_r = (struct pointer_uint_pair *)
                        ordered_set_get(*addr_to_id, curr->pointers[i]);

                if (pu_r == NULL)
                    errx(1, "%u %p not in table", i, curr->pointers[i]);

                uint32_t pointer_id = pu_r->uint;
                byte_array_append(ba, &(pointer_id), sizeof(uint32_t));
                //fprintf(stderr, "pointers:%u\t%u\n", curr_id, ba->num);

            } else {
                byte_array_append_zeros(ba, sizeof(uint32_t));
                //fprintf(stderr, "pointers:%u\t%u\n", curr_id, ba->num);
            }
        }

        // converted next value
        if ( (curr->is_leaf == 1) && (curr->next != NULL)) {
            pu_r = (struct pointer_uint_pair *)
                    ordered_set_get(*addr_to_id, curr->next);
            uint32_t next_id = pu_r->uint;
            byte_array_append(ba, &(next_id), sizeof(uint32_t));
            //fprintf(stderr, "next:%u\t%u\n", curr_id, ba->num);
        } else {
                byte_array_append_zeros(ba, sizeof(uint32_t));
                //fprintf(stderr, "next:%u\t%u\n", curr_id, ba->num);
        }

        // converted leading
        if (curr->is_leaf == 0) {
            if (curr->leading != NULL) {
               pu_r = (struct pointer_uint_pair *)
                        ordered_set_get(*addr_to_id, curr->leading);

                if (pu_r == NULL)
                    errx(1, "%u %p not in table", i, curr->leading);

                uint32_t leading_id = pu_r->uint;
                byte_array_append(ba, &(leading_id), sizeof(uint32_t));
                //fprintf(stderr, "leading:%u\t%u\n", curr_id, ba->num);
            } else {
                byte_array_append_zeros(ba, sizeof(uint32_t));
                //fprintf(stderr, "leading:%u\t%u\n", curr_id, ba->num);
            }
        } 

        // put in id to offset table
        /*
        uo_p = (struct uint_offset_size_pair *)
                malloc(sizeof(struct uint_offset_size_pair));
        uo_p->uint = curr_id;
        uo_p->offset = ftell(f);
        uo_p->size = ba->num;
        uo_r = (struct uint_offset_size_pair *)
                ordered_set_add(*id_to_offset_size, uo_p);
        */
        struct offset_size_pair os_p;
        os_p.offset = ftell(f);
        os_p.size = ba->num;
        int os_p_r = indexed_list_add(*id_to_offset_size, curr_id, &os_p);

        //fprintf(stderr, "num:%u\n", ba->num);
        if (fwrite(ba->data, sizeof(char), ba->num, f) != ba->num)
            err(EX_IOERR, "Error writing node to file");

        if (curr->is_leaf == 0) {
            // If the node is not a leaf, put all of its children on the q
            for (i = 0; i <= curr->num_keys; ++i) {
                if (curr->pointers[i] != NULL)
                    fifo_q_push(&q, curr->pointers[i]);
            }
        }
    }
}
//}}}

#if 0
//{{{void store(struct bpt_node *root, char *file_name)
void store(struct bpt_node *root, char *file_name)
{
    FILE *f = fopne(file_name, "wb");

    if (!f)
        err(EX_IOERR, "Could not write to '%s'.", file_name);
}
//}}}
#endif
