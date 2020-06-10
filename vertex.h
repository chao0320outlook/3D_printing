#ifndef VERTEX_H
#define VERTEX_H
#include "my_vector3d.h"
#include <QVector2D>
#include <utility>
#include <QList>

class Vertex
{
public:
    // Constructors
    Vertex()=default;

    Vertex(const float x,const float y,const float z,
                            const float a_1,const float a_2)
        :m_position(x,y,z),m_texture(a_1,a_2){}
    Vertex(const float& x,const float& y,const float& z ,
                            const float& x_n,const float& y_n,const float& z_n)
        :m_position(x,y,z),m_normal(x_n,y_n,z_n){}

    Vertex(const My_Vector3D &position,const QVector2D &texture):
        m_position(position),m_texture(texture){}

    Vertex(const My_Vector3D &position, const My_Vector3D &normal):
        m_position(position),m_normal(normal){}

    Vertex(const Vertex&)=default;
    Vertex& operator=(const Vertex&)=default;

    Vertex (Vertex&&)=default;
    Vertex& operator=(Vertex&&)=default;

  // Accessors / Mutators
    Q_DECL_CONSTEXPR const My_Vector3D& position() const;
    Q_DECL_CONSTEXPR const My_Vector3D& normal() const;
    Q_DECL_CONSTEXPR const QVector2D& texture() const;

    bool Is_first() const { return is_new; }
    bool is_old() const { return !is_new; }
    QMap<int,int>& show_pos_next() {return pos_next;}
    QVector<int>& show_pos_meshs(){return pos_meshs;}

    void setPosition(const My_Vector3D& position);
    void setColor (const float r,const float b,const float g);
    void setNormal(const My_Vector3D& normal);
    void setTexture(const QVector2D& texture);
    void set_old(){is_new=false;}

    void push_back_next(int i,int j)
    {
        if(j==3)
            j=0;
        pos_next[i]=j;
    }

    void push_back_set(int i)
    {
        pos_meshs.push_back(i);
    }

  // OpenGL Helpers
    static const int PositionTupleSize = 3;
    static const int NormalTupleSize = 3;
    static const int TextureTupleSize = 2;
    static const int ColorTupleSize = 3;

    static Q_DECL_CONSTEXPR int positionOffset();
    static Q_DECL_CONSTEXPR int normalOffset();
    static Q_DECL_CONSTEXPR int colorOffset();
    static Q_DECL_CONSTEXPR int textureOffset();
    static Q_DECL_CONSTEXPR int stride();


private:
    My_Vector3D m_position;  //位置
    My_Vector3D m_color{0.3f,0.3f,0.3f};    //颜色
    My_Vector3D m_normal;   //法向量
    QVector2D m_texture;   //纹理
    QMap<int,int> pos_next;          //key mesh号码  value 边标号，从1到3

    QVector<int> pos_meshs;
    bool is_new=true;
};


class X_Y_supperts
{
public:
    Q_DECL_CONSTEXPR X_Y_supperts(){}
    Q_DECL_CONSTEXPR X_Y_supperts(const QVector2D &x_z_1):x_z(x_z_1){}
    Q_DECL_CONSTEXPR X_Y_supperts(const QVector2D &x_z_1,const QVector2D &x_z_2):x_z(x_z_1),x_z_bottom(x_z_2){}

    X_Y_supperts(const X_Y_supperts&)=default;
    X_Y_supperts& operator=(const X_Y_supperts&)=default;

    X_Y_supperts (X_Y_supperts&&)=default;
    X_Y_supperts& operator=(X_Y_supperts&&)=default;

    void set_location(const float y_hight_1){y_position=y_hight_1;}
    void set_height(const float y_hight_1){y_hight=y_hight_1;}
    void set_needed(){need_bottom_height=true;}
    void set_oblique_needed(){need_oblique=true;}
    void set_mesh_num(int num){mesh_num=num;}

    Q_DECL_CONSTEXPR const bool& show_need() const {return need_bottom_height;}
    Q_DECL_CONSTEXPR const bool& show_oblique() const {return need_oblique;}
    Q_DECL_CONSTEXPR const int& mesh_mun() const {return mesh_num;}

    Q_DECL_CONSTEXPR float x() const {return x_z.x();}
    Q_DECL_CONSTEXPR float show_y_height() const {return y_hight;}
    Q_DECL_CONSTEXPR float show_y_position() const {return y_position;}
    Q_DECL_CONSTEXPR float z() const {return x_z.y();}
    Q_DECL_CONSTEXPR float x_bottom() const {return x_z_bottom.x();}
    Q_DECL_CONSTEXPR float z_bottom() const {return x_z_bottom.y();}

private:
    QVector2D x_z;
    QVector2D x_z_bottom;
    float y_position=0.0f;
    float y_hight=0.0f;
    bool need_bottom_height=false;
    bool need_oblique=false;

    int mesh_num=-1;
};


/*******************************************************************************
 * Inline Implementation
 ******************************************************************************/

// Note: Q_MOVABLE_TYPE means it can be memcpy'd.
Q_DECLARE_TYPEINFO(Vertex, Q_MOVABLE_TYPE);

// Constructors

// Accessors / Mutators
Q_DECL_CONSTEXPR inline const My_Vector3D& Vertex::position() const { return m_position; }
Q_DECL_CONSTEXPR inline const My_Vector3D& Vertex::normal() const { return m_normal; }
//Q_DECL_CONSTEXPR inline const QVector2D& Vertex::texture() const { return m_texture; }

void inline Vertex::setPosition(const My_Vector3D& position) { m_position = position; }
void inline Vertex::setNormal(const My_Vector3D& normal) { m_normal = normal; }
void inline Vertex::setTexture(const QVector2D& texture) { m_texture =texture; }
void inline Vertex::setColor (const float r,const float b,const float g){m_color=My_Vector3D(r,g,b);}


// OpenGL Helpers
Q_DECL_CONSTEXPR inline int Vertex::positionOffset() { return offsetof(Vertex, m_position); }
Q_DECL_CONSTEXPR inline int Vertex::normalOffset() { return offsetof(Vertex, m_normal); }
Q_DECL_CONSTEXPR inline int Vertex::textureOffset() { return offsetof(Vertex, m_texture); }
Q_DECL_CONSTEXPR inline int Vertex::colorOffset() { return offsetof(Vertex, m_color); }

Q_DECL_CONSTEXPR inline int Vertex::stride() { return sizeof(Vertex); }

#endif // VERTEX_H
