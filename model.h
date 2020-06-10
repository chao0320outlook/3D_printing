#ifndef MODEL_H
#define MODEL_H

#include <QVector>
#include "mesh.h"

class Model
{
public:
    Model()=default;

    void push_back(const Vertex & vertex) {vertexs.push_back(vertex);}
    void push_back(Vertex && vertex) noexcept {vertexs.push_back(std::move(vertex));}

    int size()const {return meshs_numbers;}   //返回mesh数目
    int size_vertex()const {return vertexs.size();}  //返回顶点数目

    void set_size(QVector <float> vec)    //设置模型的三轴最大最小值以及中间点
    {
        x_max=vec[0];
        x_min=vec[1];
        y_max=vec[2];
        y_min=vec[3];
        z_max=vec[4];
        z_min=vec[5];

        y_mid=y_min+(y_max-y_min)/2.0f;
        x_mid=x_min+(x_max-x_min)/2.0f;
        z_mid=z_min+(z_max-z_min)/2.0f;

        meshs_numbers=vertexs.size()/3;
    }
    void set_meshs_size(int num){meshs_numbers=num;}
    void sample_restart_size()
    {
        float swap_mine;

        swap_mine=y_max;
        y_max=z_max;
        z_max=swap_mine;

        swap_mine=y_mid;
        y_mid=z_mid;
        z_mid=swap_mine;

        swap_mine=y_min;
        y_min=z_min;
        z_min=swap_mine;

    }

    float size_X()const{return x_max-x_min;}  //返回X轴上长度
    float size_Y()const{return y_max-y_min;}  //返回Y轴上长度
    float size_Z()const{return z_max-z_min;}  //返回Z轴上长度

    float samples_mid_x()const{return x_mid/20.0f;}
    float samples_mid_y()const{return y_mid/20.0f;}
    float samples_mid_z()const{return z_mid;}

    float mid_y()const {return y_mid/10.0f;}  //获得y中心坐标
    float mid_x()const {return x_mid/10.0f;}  //获得x中心坐标
    float mid_z()const {return z_mid/10.0f;}   //获得z中心坐标

    float show_x_max()const {return x_max;}
    float show_z_max()const {return z_max;}
    float show_x_min()const {return x_min;}
    float show_z_min()const {return z_min;}
    float show_y_min()const {return y_min;}

    float y_min_val()const {return y_min;}

    void set_color(int i)
    {
        vertexs[i].setColor(1.0,0.0,0.0);
        vertexs[i+1].setColor(1.0,0.0,0.0);
        vertexs[i+2].setColor(1.0,0.0,0.0);
    }
    void set_color_2(int i)
    {
        vertexs[i].setColor(0.0,1.0,0.0);
        vertexs[i+1].setColor(0.0,1.0,0.0);
        vertexs[i+2].setColor(0.0,1.0,0.0);
    }
    void restart_color()
    {
        for(auto & i:vertexs)
            i.setColor(0.3f,0.3f,0.3f);
    }

    QVector<Vertex>& vertexs_of_model(){return vertexs;}

    void clear(){vertexs.clear();meshs_numbers=0;}

private:
    QVector<Vertex> vertexs;      //顶点数组，包含坐标与法向量 绘制时使用
    float x_max,x_min,y_max,y_min,z_max,z_min; //三轴实际最大最小值
    float x_mid,y_mid,z_mid;      //中心位置坐标
    int meshs_numbers;
};

#endif // MODEL_H
