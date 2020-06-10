#ifndef TREENODE_POINT_H
#define TREENODE_POINT_H

#include <my_vector3d.h>

struct TreeNode_Point
{
    My_Vector3D point;
    TreeNode_Point* left;
    TreeNode_Point* right;
    TreeNode_Point(My_Vector3D point_p,TreeNode_Point* left_p=nullptr,
                   TreeNode_Point* right_p=nullptr):point(point_p),left(left_p),right(right_p){}
};

#endif // TREENODE_POINT_H
