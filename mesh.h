#ifndef MESH_H
#define MESH_H
#include "vertex.h"
#include <QVector>

class Mesh
{
public:
    Mesh()=default;
    Mesh(Vertex *verte_x,Vertex *verte_y, Vertex *verte_z)
    {
        m_pos[0]=verte_x;
        m_pos[1]=verte_y;
        m_pos[2]=verte_z;
    }
    Vertex* show_vertex(int i) {return m_pos[i];}

    Vertex* get_next_pos(int i)
    {
        return m_pos[i];
    }

    Vertex* m_pos[3] ;
    QVector<int> mesh_around;
};

#endif // MESH_H
