/**
二叉树
普通树可以通过 孩子兄弟表示法 进行顺利转化为二叉树，利用二叉树的高性能进行运算
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

typedef int TElemType;

typedef struct BiTreeNode {
    TElemType data;
    struct BiTreeNode *lchild, *rchild;
} BiTree;

TElemType putin[] = {1, 2, 3, 4, 5, 6, -1, -1, 7, 8, -1, -1, -1};
TElemType *putinptr = (int *)&putin;
TElemType build(BiTree **node, char *method)
{
    if (*putinptr == 0) {
        *node = NULL;
        return -1;
    }
    TElemType num = *putinptr;
    putinptr ++;
    if (num < 0) {
        *node = NULL;
        return -1;
    } else {
        printf("node: %s, num: %d \n", method, num);
        *node = calloc(1, sizeof(BiTree));
        if (node == NULL) {
            printf("node is null \n");
            return -1;
        }
        (*node)->data = num;
        build(&(*node)->lchild, "left");
        build(&(*node)->rchild, "right");
    }
    return 0;
}

TElemType main ()
{
    BiTree *btree;
    build(&btree, "parent");

    return 0;
}
