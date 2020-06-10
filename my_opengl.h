#ifndef MY_OPENGL_H
#define MY_OPENGL_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLTexture>
#include <QOpenGLShaderProgram>

#include <QVector>
#include <QVector3D>
#include <QMap>
#include <QSet>
#include <QMatrix4x4>
#include <cmath>

#include <QKeyEvent>
#include <QEvent>

#include "transform_3d.h"
#include "camera.h"
#include "model.h"
#include "treenode_point.h"


class My_Opengl :public QOpenGLWidget, protected QOpenGLFunctions_4_5_Core
{
     Q_OBJECT
public:
    My_Opengl(QWidget *parent = nullptr);
    ~My_Opengl();

    Camera3D m_camera;                    //相机成员

    My_Vector3D vec1[3]=
    {
        {1.0f,0.0f,0.0f},
        {0.0f,0.0f,1.0f},
        {1.0f,0.0f,1.0f},
    };

    void shader_load();
    void initializeGL_model();
    void initializeGL_sample();

    void teardownGL();
    void set_samples_trans();
    void calculate_supperts();
    void count_supperts(int i,QVector<int> &vec_supperts);
    float Calculate_Triangle_area(int i);
    void find_supperts_point();

    //矩阵与向量相乘  3维
    My_Vector3D QMatrix3x3_model(My_Vector3D  vec,QMatrix3x3 & matrix );

    QMatrix3x3 transfrom_0();

    void Draw_samples();        //绘制样本
    void Draw_model();          //绘制模型函数

    void update_normal();        //需支撑位置判断
    void change_color();         //改变物体颜色
    void set_show_red(bool flag) {show_red=flag;}

    //设置相机移动
    void set_camera_left_true (){camera_left=true;}
    void set_camera_left_false (){camera_left=false;}
    void set_camera_right_true (){camera_right=true;}
    void set_camera_right_false (){camera_right=false;}


    void camera_restart(float zoom_now);
    void m_transform_restart()
    {
        //清空上个模型的变换矩阵
        view_change.restart();
        m_transform.restart();
        supports_trans.restart();
        normal_change.restart();

        samples_trans.clear();

        //清空上个模型 支撑点信息
        normal_bool.clear();
        meshs_around.clear();
        clear_supports();

        //重置相机位置
        camera_x=0.0;
        camera_y=0.0;
    }
    QQuaternion get_rotation()
    {return view_change.rotation();}

    void normal_trans(float angle, My_Vector3D axis)
    {
        normal_change.rotate(angle,axis);
    }
    void set_normal_trans(float angle, My_Vector3D axis)
    {
        normal_change.setRotation(angle,axis);
    }
    void normal_x_y_z_trans(float x,float y,float z)
    {
        normal_change.setTranslation_mid(x,y,z);
    }

    //读入模型
    void input_model(Model model)
    {
        if(models.empty())
            models.push_back(model);
        else
        {
            models.pop_back();
            models.push_back(model);
        }
    }
    //读入支撑样本
    void load_sample_models(Model model);
    void set_draw_suppports_true(float zoom)
    {
        draw_suppports=true;
        zoom_now=zoom;
    }

    //自动支撑模板生成
    void get_column(float x_loca,float z_loca,float hight);

    void get_column_2(float x_loca,float z_loca,float hight);
    void get_column_3(float x_loca,float z_loca,float hight);
    void get_column_4(float x_loca,float z_loca,float y_pos,float hight);
    void get_column_5(float x_loca,float z_loca,float hight,float x_bottom,float z_bottom);

    void get_tree_1(My_Vector3D poin);
    void get_tree_2(My_Vector3D up,My_Vector3D down);
    void get_tree_3(My_Vector3D poin);

    void model_gridding();

    Model& get_sample(){return supports_automatic;}
    Model get_all_model();


    //设置变换
    void set_transform(Transform3D tran){m_transform=tran;}
    //设置拓补数据
    void set_mesh_around(QVector<QVector<int>> meshs_around_z)
    {meshs_around= meshs_around_z; }

    void count_supperts(int i,QVector<int> &vec_supperts ,QVector<bool> &mesh_is_counted);
    void check_supperts_column(const float &x,const float &y,QVector<int>&ares);
    int check_suppert(float& x,float& y,float& suppert_hight);
    bool inter(QVector3D& P,const QVector3D& P1,const QVector3D& P2,const QVector<float>& vec);
    //寻找指定三角面的最高点
    float show_hight(int i)
    {
        float y_max=-9999.0f;
        auto& ver=model_changed.vertexs_of_model();
        QVector<float> vec;
        for(int j=0;j!=3;++j)
            vec.push_back(ver[i+j].position().y());
        for(int j=0;j!=vec.size();++j)
        {
            if(y_max<vec[j])
                y_max=vec[j];
        }
        return y_max;
    }
    bool Calculate_oblique_supports(float& x,float& y, float y_locatio);
    bool Calculate_bottom_height(float x,float y,float& y_location);
    void clear_supports();

    bool check_in_triangle(const float &x,const float &y,int num);
    void check_supperts_point(const float &x,const float &y,QVector<int>&ares);

    void Calculate_tree_supports();
    void build_tree_supports();
    void dfs(TreeNode_Point *root);

signals:

    void new_camera(float move,My_Vector3D now);
    void send_supperts_num(int num);

protected slots:
    void update();

private slots:
    void updata_models_vector(bool flag);

protected:
    void initializeGL() override;
    void resizeGL(int width, int height) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent *event)override;
    void mouseReleaseEvent(QMouseEvent *event)override;
    //滚轮事件
    void wheelEvent(QWheelEvent *event)override;
private:
    bool model_readied=false;
    bool flag_2=false;
    bool show_red=false;                   //是否显示需支撑位置
    bool show_red_pre=false;               //若为真，则说明戴志诚区域改变
    bool draw_suppports=false;             //绘制支撑环开启
    bool draw_tree_suppports=true;

    QVector<bool> normal_bool;             //是否需要支撑点集
    QVector<Model> sample_models;          //支撑样本
    Model supports_automatic;              //自生成支撑模型

    Model supports_tree;                   //树形支撑模型
    int tree_sizes=0;                      //树形支撑面片数

    Model supports;                        //支撑模型

    QVector<QVector< QVector<int> >> mesh_grid;        //存储面片投影到x_y平面对应的方格

    QVector<Model> models;                 //读入模型数据
    Model model_changed;                   //生成支撑前，将变换后的模型存入该变量
    QOpenGLTexture *texture[2];            //纹理

    QOpenGLVertexArrayObject VAO_kuang;          //VAO顶点数组对象
    QOpenGLVertexArrayObject VAO_model;
    QOpenGLVertexArrayObject VAO_sample;

    QOpenGLBuffer  VBO_kuang;                   //VBO顶点缓冲对象
    QOpenGLBuffer  VBO_model;
    QOpenGLBuffer  VBO_sample;

    QOpenGLShaderProgram *my_shader_kuang;          //绘制框
    QOpenGLShaderProgram *my_shader_model;          //着色器 绘制模型

    //shader_0 uniform值
    int my_model_0;    //模型矩阵
    int my_view_0;     //观察矩阵
    int my_projection_0;   //投影矩阵

    //shader_1 uniform值
    int my_model;    //模型矩阵
    int my_view;     //观察矩阵
    int my_projection;   //投影矩阵
    int my_normal_model;  //法向量变换矩阵

    int my_viepos;       //相机位置

    //相机移动状态
    bool camera_left=false;
    bool camera_right=false;

    //视角变换
    float camera_x=0.0;
    float camera_y=0.0;


    float zoom_now=1.0;                    //缩放倍数

    QMatrix4x4 m_projection;             //投影变换矩阵

    Transform3D view_change;             //仅统计 视角变换 的矩阵
    Transform3D m_transform;             //变换 模型
    Transform3D normal_change;           //仅统计模型旋转的矩阵                计算是否需要支撑时使用的变换矩阵,用于计算变换后的法向量

    Transform3D supports_trans;          //总支撑模型变换
    QVector<Transform3D> samples_trans;  //各支撑的变换矩阵

    QVector<QVector<int>> meshs_around;         //每个mesh周围的mesh
    QVector<QVector<int>> need_supperts_aeras;  //需要支撑的区域
    QVector<X_Y_supperts> x_y_ares;             //需要支撑的平面点

    QList<My_Vector3D> nedd_supports;           //需要支撑的空间点集
    QList<TreeNode_Point *> root_points;        //二叉树根节点集合

    QMap<int,int> map_counts;

};

#endif // MY_OPENGL_H
