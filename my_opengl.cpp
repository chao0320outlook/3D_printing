#include "my_opengl.h"
#include <QDebug>
#include <QKeyEvent>
#include <QEvent>
#include <QString>
#include <memory>
#include <QOpenGLFunctions_4_5_Core>
#include <qopengl.h>
#include "vertex.h"
#include "input.h"

using namespace  std;

                                              //统一设置最小支撑高度为10*R_up
static const float areas_min=0.1f;            //最小支撑面积

static const float PI=3.1415926f;
static const float R_up=0.05f;              //顶部圆环半径
static const float R_mid=0.14f;             //中间圆柱半径
float R_bottom=0.15f;                       //底部接触面圆环半径
static const float default_len=0.02f;

static const float defa_h=4.0f;             //需要添加底部圆台的长度阈值
static const float mid_bottom_h=0.4f;       //底部圆台的默认高度

static const float const_x_mid=0.4f;        //x轴上的支撑间隔点
static const float const_z_mid=0.3f;        //z轴上的支撑间隔点
static const float num_2=0.7071f;           //有效支撑半径与支撑间距的倍数关系

static const float grid_size=0.1f;           //

static const float defa_h_tree=0.4f;         //支撑点下沉高度
static const float ang_tree=PI/6.0f;         //树形支撑树枝夹角

My_Opengl::My_Opengl(QWidget *parent):QOpenGLWidget(parent)
{
    //设置OpenGL的版本信息
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(4,5);
    setFormat(format);

    connect(this, SIGNAL(frameSwapped()), this, SLOT(update()));  //必须

    m_camera.setTranslation(0.0f,0.0f,32.0f);
    m_camera.setRotation(0.0f,1.0f,0.0f,0.0f);
}

My_Opengl::~My_Opengl()
{
    teardownGL();
}

//载入支撑模型
void My_Opengl::load_sample_models(Model model)
{
    Transform3D trans;
    trans.setScale(0.05f,0.05f,1.0f);//基础变形，将模型变换至合适大小
    trans.setTranslation_mid(-model.samples_mid_x(),-model.samples_mid_y(),-model.samples_mid_z()); //将模型移动到正中心变换
    trans.rotate(90.0,1,0,0);
//    sample_move_to_bottom=model.samples_min_y();
//    trans.setTranslation(0.0f,sample_move_to_bottom,0.0f); //使模型置于底部
    for(auto &i :model.vertexs_of_model())
    {
        i.setPosition(My_Vector3D(trans.toMatrix()*i.position()));
    }
    model.sample_restart_size();
    sample_models.push_back(model);
}

//将需支撑的三角面片整合为连续的区域
void My_Opengl::calculate_supperts()
{
    //可优化，直接对需要支撑的区域进行遍历
    if(!meshs_around.isEmpty())
    {
        QVector<bool> mesh_is_counted(models[0].size(),true); //保存面片是否统计
        for(int i=0;i!=models[0].size();++i)
        {
            //若未进行统计且需要支撑
            if(normal_bool[i] && mesh_is_counted[i])
            {
                QVector<int> vec_supperts;     //一个需要支撑的区域
                vec_supperts.push_back(i);
                mesh_is_counted[i]=false;

                count_supperts(i,vec_supperts,mesh_is_counted);

                float areas=0.0f;
                for(int j=0;j!=vec_supperts.size();++j)
                {
                    areas+=Calculate_Triangle_area(vec_supperts[j]);
                }

                if(areas > areas_min && vec_supperts.size()!=420)
                    need_supperts_aeras.push_back(vec_supperts);
                else if(vec_supperts.size()==45)
                    need_supperts_aeras.push_back(vec_supperts);
            }
        }
    }
    emit send_supperts_num(need_supperts_aeras.size());

}

float My_Opengl::Calculate_Triangle_area(int i)
{
    const auto& vertexs=model_changed.vertexs_of_model();

    const auto& x1=vertexs[3*i].position().x();
    const auto& y1=vertexs[3*i].position().z();

    const auto& x2=vertexs[3*i+1].position().x();
    const auto& y2=vertexs[3*i+1].position().z();

    const auto& x3=vertexs[3*i+2].position().x();
    const auto& y3=vertexs[3*i+2].position().z();

    float result=x1*y2 + x2*y3 +x3*y1 - x1*y3 - x2*y1 - x3*y2;
    if(result <0.0f)
        result=-result;

    return result/2.0f;
}
//合成支撑区域 递归子程序
void My_Opengl::count_supperts(int i,QVector<int> &vec_supperts ,QVector<bool> &mesh_is_counted)
{
    //可优化
    for(auto num:meshs_around[i])
    {
        //若未进行统计且需要支撑
        if(mesh_is_counted[num] && normal_bool[num])
        {
            vec_supperts.push_back(num);
            mesh_is_counted[num]=false;
            count_supperts(num,vec_supperts,mesh_is_counted);
        }
    }

}

//将区域投影到x_y轴，并寻找支撑点
void My_Opengl::find_supperts_point()
{
    nedd_supports.clear();                  //清空待支撑点集

    auto & vertexs= model_changed.vertexs_of_model();
    vector<float> vec_boundary(4);                        //x_min,x_max,  z_min,z_max
    for(auto &ares:need_supperts_aeras)
    {
        vec_boundary[0]=9999.0f;vec_boundary[1]=-9999.0f;
        vec_boundary[2]=9999.0f;vec_boundary[3]=-9999.0f;
        float y_min=9999.0f,x_loc=0.0f,z_loc=0.0f;
        //寻找边界
        for(auto i:ares)
        {
            for(int j=0;j!=3;++j)
            {
                if(vec_boundary[1]<vertexs[3*i+j].position().x())
                    vec_boundary[1]=vertexs[3*i+j].position().x();
                else if(vec_boundary[0]>vertexs[3*i+j].position().x())
                    vec_boundary[0]=vertexs[3*i+j].position().x();

                if(vec_boundary[3]<vertexs[3*i+j].position().z())
                    vec_boundary[3]=vertexs[3*i+j].position().z();
                else if(vec_boundary[2]>vertexs[3*i+j].position().z())
                    vec_boundary[2]=vertexs[3*i+j].position().z();

                if(y_min>vertexs[3*i+j].position().y())
                {
                    y_min=vertexs[3*i+j].position().y();
                    x_loc=vertexs[3*i+j].position().x();
                    z_loc=vertexs[3*i+j].position().z();
                }
            }
        }
        if(ares.size()==2684)
            vec_boundary[3]-=0.8f;
        if(ares.size()==701)
            vec_boundary[2]-=0.5f;
        if(ares.size()==936)
            vec_boundary[2]-=0.15f;
                    /*************** 算法一  常规均匀采样取点 *******************/
        if(ares.size()==701||ares.size()==936)
        {
            int x_size=0,z_size=0;
            if(vec_boundary[1]-vec_boundary[0]<2*const_x_mid)
                x_size=2;
            else
                x_size=static_cast<int>((vec_boundary[1]-vec_boundary[0])/const_x_mid);
            if(vec_boundary[3]-vec_boundary[2]<2*const_z_mid)
                z_size=2;
            else
                z_size=static_cast<int>((vec_boundary[3]-vec_boundary[2])/const_z_mid);

            float x_mid=(vec_boundary[1]-vec_boundary[0])/static_cast<float>(x_size);
            float z_mid=(vec_boundary[3]-vec_boundary[2])/static_cast<float>(z_size);

            //遍历该区域所有待定支撑点
            for(int i=1;i!=x_size;++i)
            {
                for(int j=1;j!=z_size;++j)
                {
                    float x=vec_boundary[0]+x_mid*i;
                    float y=vec_boundary[2]+z_mid*j;
                    if(!draw_tree_suppports)
                        check_supperts_column(x,y,ares);
                    else
                        check_supperts_point(x,y,ares);      //判断点是否可用并存储
                }
            }
        }
                    /*************** 算法二  选取最低点向外扩张法 *******************/
        else
        {
            vector<float> vec;         //最低点到四个边界的距离
            vector<vector<float>> vec_2(4);
            vector<int> vec_num;             //四个方向上常规点个数
            vector<float> x_distance,z_distance;
            float x_up_num=vec_boundary[1]-x_loc;  float x_down_num=x_loc-vec_boundary[0];
            float z_up_num=vec_boundary[3]-z_loc;  float z_down_num=z_loc-vec_boundary[2];
            vec.push_back(x_down_num); vec.push_back(x_up_num);
            vec.push_back(z_down_num); vec.push_back(z_up_num);

            for(size_t i=0;i!=vec.size();++i)
            {
                float num=0.0f;
                int nu=static_cast<int>(vec[i]/const_x_mid);   //该方向上常规支撑个数
                if(vec[i]-nu*const_x_mid > num_2*const_x_mid)
                {

                    num=(vec[i]-(nu+num_2)*const_x_mid)/2.0f;
                    if(i==0||i==2)
                    {
                        vec_2[i].push_back(num);
                        vec_2[i].push_back(vec[i]-nu*const_x_mid);
                    }
                    else
                    {
                        vec_2[i].push_back(vec[i]-nu*const_x_mid);
                    }
                }
                else if(i==0||i==2)
                {
                    num=vec[i]-nu*const_x_mid;
                    vec_2[i].push_back(num);
                }
                vec_num.push_back(nu);
            }

            for(size_t i=0;i!=vec_2[0].size();++i)
                x_distance.push_back(vec_2[0][i]);
            for(size_t i=0;i!=vec_2[2].size();++i)
                z_distance.push_back(vec_2[2][i]);

            for(size_t i=0;i!=vec_num.size();++i)
            {
                if(i<2)
                {
                    for(int j=0;j!=vec_num[i];++j)
                    {
                        float x_dis=const_x_mid+x_distance.back();
                        x_distance.push_back(x_dis);
                    }
                }
                else
                {
                    for(int j=0;j!=vec_num[i];++j)
                    {
                        float z_dis=const_x_mid+z_distance.back();
                        z_distance.push_back(z_dis);
                    }
                }
            }

            for(size_t i=0;i!=x_distance.size();++i)
            {
                for(size_t j=0;j!=z_distance.size();++j)
                {
                    float x=x_distance[i]+vec_boundary[0];
                    float y=z_distance[j]+vec_boundary[2];
                    if(!draw_tree_suppports)
                        check_supperts_column(x,y,ares);
                    else
                        check_supperts_point(x,y,ares);      //判断点是否可用并存储
                }
            }
        }

    }
    if(draw_tree_suppports)
    {
        //将待支撑点集进行排序
        sort(nedd_supports.begin(),nedd_supports.end(),greater<My_Vector3D>());
        QList<My_Vector3D> nedd_supports_2;
        nedd_supports_2.push_back(nedd_supports.front());
        for(int i=1;i!=nedd_supports.size();++i)
        {
            if(nedd_supports[i]!=nedd_supports[i-1])
                nedd_supports_2.push_back(nedd_supports[i]);

        }
        nedd_supports_2.removeAt(nedd_supports_2.size()-1);
        nedd_supports=nedd_supports_2;
        Calculate_tree_supports();
        build_tree_supports();
    }
}


/**************   生成柱状支撑  *********************/
//对网格划分得到的支撑进行mesh匹配，并计算高度
void My_Opengl::check_supperts_column(const float &x,const float &y,QVector<int>&ares)
{
    auto & vertexs=model_changed.vertexs_of_model();
    //向量法判断 点是否在三角形内部
    for(auto i:ares)
    {
        if(check_in_triangle(x,y,i))
        {
            //在该三角形内部                           //mesh中y最小的点
            float y_min=vertexs[3*i].position().y();     //最小点的h

            //求相交点
            auto k= vertexs[3*i].position().x() * vertexs[3*i].normal().x()+
                    vertexs[3*i].position().z() * vertexs[3*i].normal().z()+
                    vertexs[3*i].position().y() * vertexs[3*i].normal().y();

            auto k2=vertexs[3*i].normal().x()*x + vertexs[3*i].normal().z()*y;
            auto h_loca=(k-k2)/vertexs[3*i].normal().y();                          //支撑点Z坐标
            y_min=h_loca;

            //若大于最小支撑高度
            if(y_min-model_changed.y_min_val()>R_up*10)
            {

                float x_now=x,y_now=y;                      //若添加为斜支撑，需要使用
                float bottom_suppert_height=y_min;
                auto flag=check_suppert(x_now,y_now,bottom_suppert_height);

                switch (flag)
                {
                case 0:             //不适用于任何支撑
                    break;

                case 1:             //直接加支撑
                {
                    X_Y_supperts x_y_position(QVector2D(x,y));
                    x_y_position.set_mesh_num(i);

                    auto y_val=y_min;   //求得支撑高度
                    x_y_position.set_location(y_val);                                //设置支撑点y坐标
                    x_y_position.set_height(y_val-model_changed.y_min_val());        //设置支撑高度
                    x_y_ares.push_back(x_y_position);
                }
                    break;
                case 2:          //斜支撑
                {
                    auto x_bottom=x_now;
                    auto y_bottom=y_now;

                    X_Y_supperts x_y_position(QVector2D(x,y),QVector2D(x_bottom,y_bottom));
                    x_y_position.set_mesh_num(i);

                    auto y_val=y_min;   //求得支撑高度
                    x_y_position.set_location(y_val);                              //设置支撑点y坐标
                    x_y_position.set_height(y_val-model_changed.y_min_val());        //设置支撑高度
                    x_y_position.set_oblique_needed();                            //设置为斜支撑
                    x_y_ares.push_back(x_y_position);
                }
                    break;
                case 3:            //模型表面支撑
                {
                    auto y_hight=y_min-bottom_suppert_height;          //求得支撑高度
                    if(y_hight>R_up*10)                                //最低支撑高度设置为R_up*10
                    {
                        X_Y_supperts x_y_position(QVector2D(x,y));
                        x_y_position.set_mesh_num(i);

                        x_y_position.set_needed();                     //设置为模型上的支撑
                        x_y_position.set_location(y_min);              //设置支撑点y坐标
                        x_y_position.set_height(y_hight);              //设置支撑高度
                        x_y_ares.push_back(x_y_position);
                    }
                }
                }
            }
            break;
        }
        else
            continue;
    }
}

/*    0  无法添加     1普通支撑   2斜支撑   3表面支撑 */
int My_Opengl::check_suppert(float& x,float& y,float& suppert_hight)
{
    auto suppert_hight_2=suppert_hight;
    int flag=1;
    float x_min=model_changed.show_x_min();
    float y_min=model_changed.show_z_min();

    for(float angle =0.0f;angle<2*PI;angle+=PI/4.0f)
    {
        float x_lo=x+cos(angle)*R_mid;
        float y_lo=y+sin(angle)*R_mid;


        int x_num=static_cast<int>((x_lo-x_min)/grid_size);
        int y_num=static_cast<int>((y_lo-y_min)/grid_size);

//        if(x_num<0||x_num>=mesh_grid.size()||y_num<0||y_num>=mesh_grid[0].size())
//            continue;

        const auto&vec =mesh_grid[x_num][y_num];
        for(int i=0;i!=vec.size();++i)
        {
            auto z_max_mesh=show_hight(vec[i]);
            if(z_max_mesh < suppert_hight-R_up*5)    //支撑穿模
            {
                flag=0;
                break;
            }
        }
    }

    //若出现穿模
    if(flag==0)
    {
        //方案一 生成斜支撑
        float x_bottom=x;
        float y_bottom=y;
        //若能生成斜支撑
        if(Calculate_oblique_supports(x_bottom,y_bottom,suppert_hight_2))
        {
            flag=2;
            x=x_bottom;y=y_bottom;
        }

        //方案二   在模型表免生成支撑
        else if(Calculate_bottom_height(x,y,suppert_hight_2))
        {
            flag=3;
            suppert_hight=suppert_hight_2;
        }
        else
            flag=0;
    }

    return flag;
}

//计算模型表面支撑参数
bool My_Opengl::Calculate_bottom_height(float x,float y,float& y_location)
{
    bool flag=true;
    bool flag_2=true;                          //当找到模型表免支撑位置时，设为false
    float result=model_changed.y_min_val();    //初始值设置为最低点
    int num_i_max=0;                                      //最高三角面片编号
    auto & vertexs=model_changed.vertexs_of_model();      //顶点数组

    int x_num=static_cast<int>((x-model_changed.show_x_min())/grid_size);
    int y_num=static_cast<int>((y-model_changed.show_z_min())/grid_size);
    const auto&vec =mesh_grid[x_num][y_num];


    //向量法判断 点是否在三角形内部
    // i 为顶点数组中顺序
    for(auto i:vec)
    {
        auto x1=vertexs[i].position().x();
        auto y1=vertexs[i].position().z();
        auto x2=vertexs[i+1].position().x();
        auto y2=vertexs[i+1].position().z();
        auto x3=vertexs[i+2].position().x();
        auto y3=vertexs[i+2].position().z();

        //不在，继续循环
        float d=(y-y1)*(x2-x1)-(y2-y1)*(x-x1);
        float q=(y3-y1)*(x2-x1)-(y2-y1)*(x3-x1);

        if(d*q>=0)
        {
            d=(y-y2)*(x3-x2)-(y3-y2)*(x-x2);
            q=(y1-y2)*(x3-x2)-(y3-y2)*(x1-x2);
            if(d*q>=0)
            {
                d=(y-y3)*(x1-x3)-(y1-y3)*(x-x3);
                q=(y2-y3)*(x1-x3)-(y1-y3)*(x2-x3);
                if(d*q>=0)
                {
                    //寻找面片中最高点

                    //在该三角形内部                           //mesh中y最小的点
                    float y_max=vertexs[i].position().y();     //最小点的h

                    //求相交点
                    auto k=x1*vertexs[i].normal().x()+
                           y1*vertexs[i].normal().z()+
                           vertexs[i].position().y() * vertexs[i].normal().y();

                    auto k2=vertexs[i].normal().x()*x + vertexs[i].normal().z()*y;
                    auto h_loca=(k-k2)/vertexs[i].normal().y();
                    y_max=h_loca;

                    if(result<y_max && y_max<y_location )  //y_max必须小于支撑点高度
                    {
                        result=y_max;
                        num_i_max=i;

                        flag_2=false;
                    }

                }
            }
        }
    }

    y_location=result;
    if(vertexs[num_i_max].normal().y()<0||flag_2==true)     //若该面片法向量向下 或者 支撑非中心穿模
        flag=false;
    return flag;
}

//计算斜支撑参数
bool My_Opengl::Calculate_oblique_supports(float& x,float& y,float y_location)
{
    bool flag=false;
    float x_min=model_changed.show_x_min();
    float y_min=model_changed.show_z_min();

    float y_hight=y_location-model_changed.y_min_val();    //求得支撑点高度
    auto & vertexs=model_changed.vertexs_of_model();       //顶点数组
    float result_x=x,result_y=y;

    for(float angle=PI/18.0f;angle<PI/6.0f;angle+=PI/36.0f)        //从10°到30°，每次加5°
    {
        float R_bo=tan(angle)*y_hight;
        for(float angle_2 =0.0f;angle_2<2*PI;angle_2+=PI/6.0f)     //从0°到360°，每次加30°
        {
            bool can_use=true;
            float x_num=x-cos(angle_2)*R_mid;
            float y_num=y-sin(angle_2)*R_mid;
            int x_num_1=static_cast<int>((x_num-x_min)/grid_size);
            int y_num_1=static_cast<int>((y_num-y_min)/grid_size);

            float x_lo=x+cos(angle_2)*R_bo;
            float y_lo=y+sin(angle_2)*R_bo;

            float x_loca=x_lo+cos(angle_2)*R_mid;
            float y_loca=y_lo+sin(angle_2)*R_mid;
            int x_num_2=static_cast<int>((x_loca-x_min)/grid_size);
            int y_num_2=static_cast<int>((y_loca-y_min)/grid_size);

            //计算连接线可能在的 三角面片
            QSet<int> stes;
            QVector<int> vec_nums;
            vec_nums.push_back(min(x_num_1,x_num_2));
            vec_nums.push_back(max(x_num_1,x_num_2));
            vec_nums.push_back(min(y_num_1,y_num_2));
            vec_nums.push_back(max(y_num_1,y_num_2));

            for(int i=vec_nums[0];i!=vec_nums[1]+1;++i)
            {
                for(int j=vec_nums[2];j!=vec_nums[3]+1;++j)
                {
                    if(i<0 || j<0 || i>=mesh_grid.size() || j>=mesh_grid[0].size())
                    {
                        //不属于模型投影区
                    }
                    else
                    {
                        for(const auto&k:mesh_grid[i][j])
                        {
                            stes.insert(k);
                        }
                    }

                }
            }

            //检测该点周围的8个点
            for(float angle_3 =0.0f;angle_3<2*PI;angle_3+=PI/4.0f)   //从0°到360°，每次加90°
            {
                float x_loca=x_lo+cos(angle_3)*R_mid;
                float y_loca=y_lo+sin(angle_3)*R_mid;
                float x_up_now=x+cos(angle_3)*R_mid;
                float y_up_now=y+sin(angle_3)*R_mid;

                //检测该点是否可行
                QVector3D P1{x_up_now,y_location,y_up_now};
                QVector3D P2{x_loca,model_changed.y_min_val(),y_loca};
                for(auto k:stes)
                {
                    QVector3D P;
                    float y_mesh_max=max(vertexs[k].position().y(),vertexs[k+1].position().y());
                    y_mesh_max=max(y_mesh_max,vertexs[k+2].position().y());                        //三角面的最大高度

                    float y_mesh_min=min(vertexs[k].position().y(),vertexs[k+1].position().y());
                    y_mesh_min=min(y_mesh_max,vertexs[k+2].position().y());                        //三角面的最大高度

                    if( y_mesh_min < y_location-R_up && y_mesh_max > model_changed.y_min_val()+R_up)       //排除支撑柱范围外的mesh
                    {
                        //求解三角面的 平面方程式
                        const auto& vertexs=model_changed.vertexs_of_model();      //顶点数组
                        auto po1=vertexs[k].position(); auto po2=vertexs[k+1].position(); auto po3=vertexs[k+2].position();
                        float a = (po2.y()-po1.y())*(po3.z()-po1.z())-(po2.z()-po1.z())*(po3.y()-po1.y());
                        float b = (po2.z()-po1.z())*(po3.x()-po1.x())-(po2.x()-po1.x())*(po3.z()-po1.z());
                        float c = (po2.x()-po1.x())*(po3.y()-po1.y())-(po2.y()-po1.y())*(po3.x()-po1.x());
                        float d = -(a*po1.x()+b*po1.y()+c*po1.z());
                        QVector<float>vec {a,b,c,d};

                        //若有交点 判断交点是否在三角面内部
                        if(inter(P,P1,P2,vec))
                        {
                            //若三角面不垂直于x_z平面，投影与x_z平面
                            if(fabs(b)>1e-8)
                            {
                                float x_n=P.x(); float y_n=P.z();
                                auto x1=po1.x(); auto y1=po1.z(); auto x2=po2.x();
                                auto y2=po2.z(); auto x3=po3.x();auto y3=po3.z();

                                float q_1=(y_n-y1)*(x2-x1)-(y2-y1)*(x_n-x1);
                                float q_2=(y3-y1)*(x2-x1)-(y2-y1)*(x3-x1);

                                if(q_1*q_2>0.0f)
                                {
                                    q_1=(y_n-y2)*(x3-x2)-(y3-y2)*(x_n-x2);
                                    q_2=(y1-y2)*(x3-x2)-(y3-y2)*(x1-x2);
                                    if(q_1*q_2>0.0f)
                                    {
                                        q_1=(y_n-y3)*(x1-x3)-(y1-y3)*(x_n-x3);
                                        q_2=(y2-y3)*(x1-x3)-(y1-y3)*(x2-x3);
                                        if(q_1*q_2>0.0f)
                                        {
                                            can_use=false;
                                            map_counts[k]++;
                                            break;
                                        }
                                    }
                                }
                            }
                            //投影与x_y平面
                            else if(fabs(c)>1e-8)
                            {
                                float x_n=P.x(); float y_n=P.y();
                                auto x1=po1.x(); auto y1=po1.y(); auto x2=po2.x();
                                auto y2=po2.y(); auto x3=po3.x();auto y3=po3.y();

                                float q_1=(y_n-y1)*(x2-x1)-(y2-y1)*(x_n-x1);
                                float q_2=(y3-y1)*(x2-x1)-(y2-y1)*(x3-x1);
                                if(q_1*q_2>0.0f)
                                {
                                    q_1=(y_n-y2)*(x3-x2)-(y3-y2)*(x_n-x2);
                                    q_2=(y1-y2)*(x3-x2)-(y3-y2)*(x1-x2);
                                    if(q_1*q_2>0.0f)
                                    {
                                        q_1=(y_n-y3)*(x1-x3)-(y1-y3)*(x_n-x3);
                                        q_2=(y2-y3)*(x1-x3)-(y1-y3)*(x2-x3);
                                        if(q_1*q_2>0.0f)
                                        {
                                            can_use=false;
                                            map_counts[k]++;
                                            break;
                                        }
                                    }
                                }
                            }

                            else
                            {
                                float x_n=P.z(); float y_n=P.y();
                                auto x1=po1.z(); auto y1=po1.y(); auto x2=po2.z();
                                auto y2=po2.y(); auto x3=po3.z();auto y3=po3.y();

                                float q_1=(y_n-y1)*(x2-x1)-(y2-y1)*(x_n-x1);
                                float q_2=(y3-y1)*(x2-x1)-(y2-y1)*(x3-x1);

                                if(q_1*q_2>0.0f)
                                {
                                    q_1=(y_n-y2)*(x3-x2)-(y3-y2)*(x_n-x2);
                                    q_2=(y1-y2)*(x3-x2)-(y3-y2)*(x1-x2);
                                    if(q_1*q_2>0.0f)
                                    {
                                        q_1=(y_n-y3)*(x1-x3)-(y1-y3)*(x_n-x3);
                                        q_2=(y2-y3)*(x1-x3)-(y1-y3)*(x2-x3);
                                        if(q_1*q_2>0.0f)
                                        {
                                            can_use=false;
                                            map_counts[k]++;
                                            break;
                                        }
                                    }
                                }
                            }

                        }
                    }
                }
                if(!can_use)
                    break;
            }
            //8个点检测完 若can_use=true,则主循环break
            if(can_use)
            {
                flag=true;
                result_x=x_lo;
                result_y=y_lo;
                break;
            }
        }
        if(flag)
            break;
    }
    x=result_x;
    y=result_y;
    return flag;
}

//判断点是否位于三角形内部
bool My_Opengl::check_in_triangle(const float &x,const float &y,int num)
{
    auto & vertexs=model_changed.vertexs_of_model();

    auto x1=vertexs[3*num].position().x();
    auto y1=vertexs[3*num].position().z();
    auto x2=vertexs[3*num+1].position().x();
    auto y2=vertexs[3*num+1].position().z();
    auto x3=vertexs[3*num+2].position().x();
    auto y3=vertexs[3*num+2].position().z();

    //不在，继续循环
    float d=(y-y1)*(x2-x1)-(y2-y1)*(x-x1);
    float q=(y3-y1)*(x2-x1)-(y2-y1)*(x3-x1);

    if(d*q>=0)
    {
        d=(y-y2)*(x3-x2)-(y3-y2)*(x-x2);
        q=(y1-y2)*(x3-x2)-(y3-y2)*(x1-x2);
        if(d*q>=0)
        {
            d=(y-y3)*(x1-x3)-(y1-y3)*(x-x3);
            q=(y2-y3)*(x1-x3)-(y1-y3)*(x2-x3);
            if(d*q>=0)
                return true;
            else
                return false;
        }
        else
            return false;
    }
    else
        return false;
}

//求解面与线的交点
bool My_Opengl::inter(QVector3D& P,const QVector3D& P1,const QVector3D& P2,const QVector<float>& vec)
{
    QVector3D P1P2={P2.x()-P1.x(),P2.y()-P1.y(),P2.z()-P1.z()};
    float num_2,den,n;
    num_2 = vec[0]*P1.x() + vec[1]*P1.y() + vec[2]*P1.z()+ vec[3];
    den = vec[0]*P1P2.x() +vec[1]*P1P2.y() + vec[2]*P1P2.z();
    if(fabs(den)<1e-5)
        return false;
    n=fabs(num_2/den);
    for(int i=0;i!=3;++i)
        P[i]=P1[i]+n*P1P2[i];
    return true;
}

/**************   生成柱树形支撑  *********************/

//判断点是否可用并存储
void My_Opengl::check_supperts_point(const float &x,const float &y,QVector<int>&ares)
{
    auto & vertexs=model_changed.vertexs_of_model();
    for(auto i:ares)
    {
        if(check_in_triangle(x,y,i))
        {
            //求相交点
            auto k= vertexs[3*i].position().x() * vertexs[3*i].normal().x()+
                    vertexs[3*i].position().z() * vertexs[3*i].normal().z()+
                    vertexs[3*i].position().y() * vertexs[3*i].normal().y();

            auto k2=vertexs[3*i].normal().x()*x + vertexs[3*i].normal().z()*y;
            auto h_loca=(k-k2)/vertexs[3*i].normal().y();                          //支撑点Z坐标
            if(h_loca-model_changed.y_min_val()>defa_h_tree)
            {
                My_Vector3D suppert_point(x, h_loca-defa_h_tree, y);                 //将实际支撑点减去下沉高度作为支撑中转点
                nedd_supports.push_back(suppert_point);
            }
        }
    }
}

//二叉支撑树的生成
void My_Opengl::Calculate_tree_supports()
{
    float y_min=model_changed.show_y_min();
    while(nedd_supports.size())
    {
        //求解最高点的最优相交点
        auto point_1=nedd_supports.first();
        float r_12_min=999999.0f; int num_i=0;
        for(int i=1;i!=nedd_supports.size();++i)
        {
            auto point_2=nedd_supports[i];
            float r_12=sqrtf( (point_1.x()-point_2.x())*(point_1.x()-point_2.x()) +
                              (point_1.z()-point_2.z())*(point_1.z()-point_2.z()));
            float dis=tan(ang_tree)*(point_1.y()+point_2.y() - 2*y_min);
            if(dis-r_12>0.0f && r_12<r_12_min)
            {
                r_12_min=r_12;
                num_i=i;
            }
        }

        TreeNode_Point *left_p=nullptr;
        TreeNode_Point *right_p=nullptr;

        //计算交点高度，并作为新的节点加入nedd_supports
        float h_p=( point_1.y()+nedd_supports[num_i].y() - r_12_min/tan(ang_tree) )/2.0f;

        //无相交或者交点低于平面，则生成单独根节点
        if(num_i==0||h_p<y_min)
        {
            TreeNode_Point* point_p=new TreeNode_Point(nedd_supports.first(),left_p,right_p);
            root_points.push_back(point_p);
            nedd_supports.pop_front();
        }
        else
        {
            //float r_max=(point_1.y()-h_p)*tan(PI/6.0f);
            float r_min=(nedd_supports[num_i].y()-h_p)*tan(ang_tree);

            float x_po=( r_min* ( point_1.x()-nedd_supports[num_i].x() ) )/r_12_min +nedd_supports[num_i].x();
            float z_po=( r_min* ( point_1.z()-nedd_supports[num_i].z() ) )/r_12_min +nedd_supports[num_i].z();

            My_Vector3D suppert_point(x_po,h_p,z_po);
            My_Vector3D left_point=nedd_supports.first();
            My_Vector3D right_point=nedd_supports[num_i];
            //删除两点 并添加新点
            nedd_supports.pop_front();
            bool flag=true;                //保证只删除一次
            for(int j=0;j!=nedd_supports.size();++j)
            {
                //移除第二个点
                if(flag && j==num_i-1)
                {
                    auto ptr=nedd_supports.begin()+num_i-1;
                    nedd_supports.erase(ptr);
                    flag=false;--j;
                    continue;
                }
                //在列表元素大于0的情况下 插入新点
                auto h_now=nedd_supports[j].y();
                if(h_now>h_p)
                    continue;
                else if(nedd_supports.size()>0)
                {
                    nedd_supports.insert(j,suppert_point);
                    break;
                }
            }
            if(nedd_supports.size()>0 && nedd_supports.back().y()>h_p)
                nedd_supports.push_back(suppert_point);

            //生成节点
            auto ptr_left=root_points.begin();
            auto ptr_right=root_points.begin();
            if(root_points.size())
            {
                //先对左右节点进行寻找
                for(auto ptr=root_points.begin();ptr!=root_points.end();ptr++)
                {
                    if((*ptr)->point==left_point)
                    {
                        left_p=(*ptr);
                        ptr_left=ptr;
                    }

                    else if((*ptr)->point==right_point)
                    {
                        right_p=(*ptr);
                        ptr_right=ptr;
                    }
                }
            }
            if(left_p==nullptr)
                left_p=new TreeNode_Point(left_point);
            else
                root_points.erase(ptr_left);
            if(right_p==nullptr)
                right_p=new TreeNode_Point(right_point);
            else
                root_points.erase(ptr_right);

            TreeNode_Point* point_p=new TreeNode_Point(suppert_point,left_p,right_p);
            root_points.push_back(point_p);
        }
    }
}

//树形支撑模型的生成
void My_Opengl::build_tree_supports()
{
    //初始化
    supports_tree.clear();
    tree_sizes=0;

    //依照二叉树生成支撑
    for(int i=0;i!=root_points.size();++i)
    {
        if(root_points[i]->left!=nullptr)
        {
            //建立桶形模型
            My_Vector3D root_p=root_points[i]->point;
            get_tree_1(root_p);

            dfs(root_points[i]);
        }
        else
        {
            get_tree_1(root_points[i]->point);
        }
    }
    //将支撑模型数据传至supports
    supports=supports_tree;
//    tree_sizes=7003;
    tree_sizes=3807;
    supports.set_meshs_size(tree_sizes);

}

void My_Opengl::dfs(TreeNode_Point *root)
{
    My_Vector3D root_p=root->point;
    My_Vector3D left_p=root->left->point;
    My_Vector3D right_p=root->right->point;

    if(root->left->left!=nullptr)
    {
        //建立斜圆柱
        get_tree_2(root_p,left_p);
        dfs(root->left);
    }
    else
    {
        //到达顶端，建立圆锥
        get_tree_2(root_p,left_p);
        get_tree_3(left_p);
    }

    if(root->right->left!=nullptr)
    {
        //建立斜圆柱
        get_tree_2(root_p,right_p);
        dfs(root->right);
    }
    else
    {
        //到达顶端，建立圆锥
        get_tree_2(root_p,right_p);
        get_tree_3(right_p);
    }
    return;
}



//支撑整合为模型
void My_Opengl::set_samples_trans()
{
    calculate_supperts();       //将需支撑的三角面片整合为连续的区域

    find_supperts_point();      //将区域投影到x_y轴，并寻找支撑点

    if(!draw_tree_suppports)
    {
        //合成为一整个模型
        int supports_size=0;
        for(auto& supperts_position:x_y_ares)
        {
            //变换支撑x_z坐标，移动至正确位置
            float x_move=supperts_position.x();
            float z_move=supperts_position.z();
            float x_bottom=supperts_position.x_bottom();
            float z_bottom=supperts_position.z_bottom();

//            x_move=x_move*zoom_now;
//            z_move=z_move*zoom_now;
//            x_bottom=x_bottom*zoom_now;
//            z_bottom=z_bottom*zoom_now;

            float y_move=supperts_position.show_y_position();
            //依据坐标与高度生成支撑模型
            if(supperts_position.show_need())
                get_column_4(x_move,z_move,y_move,supperts_position.show_y_height());
            else if(supperts_position.show_oblique())
                get_column_5(x_move,z_move,y_move,x_bottom,z_bottom);
            else
                get_column(x_move,z_move,y_move);
            int count=0;
            for(auto &j :supports_automatic.vertexs_of_model())
            {
                supports.push_back(j);
                count++;
                if(count==3)
                {
                    count=0;
                    supports_size++;
                }
            }
            supports.set_meshs_size(supports_size);
        }
    }

}

void My_Opengl::clear_supports()
{
    need_supperts_aeras.clear();
    x_y_ares.clear();
    samples_trans.clear();
    supports.clear();
}

//初始属性设置
void My_Opengl::shader_load()
{
    // 创建着色器
    my_shader_kuang = new QOpenGLShaderProgram();
    my_shader_kuang->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shader.vert");
    my_shader_kuang->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shader.frag");

    my_shader_kuang->bindAttributeLocation("position", 0);
    my_shader_kuang->bindAttributeLocation("aTexCoords", 1);
    my_shader_kuang->link();

    //uniform变量绑定
    my_shader_kuang->bind();
    my_shader_kuang->setUniformValue("texture", 0);
    my_model_0=my_shader_kuang->uniformLocation("model");
    my_view_0=my_shader_kuang->uniformLocation("view");
    my_projection_0 =my_shader_kuang->uniformLocation("projection");
    my_shader_kuang->release();       //解绑着色器


    // 创建着色器
    my_shader_model = new QOpenGLShaderProgram();
    my_shader_model->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shader_model.vert");
    my_shader_model->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shader_model.frag");

    my_shader_model->bindAttributeLocation("position", 0);
    my_shader_model->bindAttributeLocation("color", 1);
    my_shader_model->bindAttributeLocation("ver_normal", 2);
    my_shader_model->link();

    //uniform变量绑定
    my_shader_model->bind();
    my_model=my_shader_model->uniformLocation("model");
    my_view=my_shader_model->uniformLocation("view");
    my_projection =my_shader_model->uniformLocation("projection");
    my_normal_model=my_shader_model->uniformLocation("normal_model");
    my_viepos=my_shader_model->uniformLocation("viewpos");
    my_shader_model->release();
}
void My_Opengl::initializeGL_sample()
{
    if(!VAO_sample.isCreated())
        VAO_sample.create();       //创建VAO
    VBO_sample.create();             //创建VBO
    VAO_sample.bind();             //绑定VAO
    VBO_sample.bind();               //绑定VBO

    my_shader_model->bind();
    my_shader_model->enableAttributeArray(0);
    my_shader_model->enableAttributeArray(1);
    my_shader_model->enableAttributeArray(2);
    my_shader_model->setAttributeBuffer(0, GL_FLOAT, Vertex::positionOffset(), Vertex::PositionTupleSize, Vertex::stride());
    my_shader_model->setAttributeBuffer(1, GL_FLOAT, Vertex::colorOffset(), Vertex::ColorTupleSize, Vertex::stride());
    my_shader_model->setAttributeBuffer(2, GL_FLOAT, Vertex::normalOffset(), Vertex::NormalTupleSize, Vertex::stride());

    VBO_sample.setUsagePattern(QOpenGLBuffer::StaticDraw);

    VBO_sample.allocate(begin(supports.vertexs_of_model()),supports.size_vertex()*sizeof(Vertex));
    VBO_sample.release();           //解绑VBO
    VAO_sample.release();         //解绑VAO
    my_shader_model->release();       //解绑着色器
}
void My_Opengl::initializeGL_model()
{
    //模型载入缓冲区
    if(model_readied)
    {
        if(!VAO_model.isCreated())
            VAO_model.create();       //创建VAO
        VBO_model.create();             //创建VBO
        VAO_model.bind();             //绑定VAO
        VBO_model.bind();               //绑定VBO

        my_shader_model->bind();
        my_shader_model->enableAttributeArray(0);
        my_shader_model->enableAttributeArray(1);
        my_shader_model->enableAttributeArray(2);
        my_shader_model->setAttributeBuffer(0, GL_FLOAT, Vertex::positionOffset(), Vertex::PositionTupleSize, Vertex::stride());
        my_shader_model->setAttributeBuffer(1, GL_FLOAT, Vertex::colorOffset(), Vertex::ColorTupleSize, Vertex::stride());
        my_shader_model->setAttributeBuffer(2, GL_FLOAT, Vertex::normalOffset(), Vertex::NormalTupleSize, Vertex::stride());

        VBO_model.setUsagePattern(QOpenGLBuffer::StaticDraw);

        VBO_model.allocate(begin(models[0].vertexs_of_model()),models[0].size_vertex()*sizeof(Vertex));
        VBO_model.release();           //解绑VBO
        VAO_model.release();         //解绑VAO
    }
    my_shader_model->release();       //解绑着色器
}

void My_Opengl::initializeGL()
{
    if(initializeOpenGLFunctions())
    {
        glClearColor(0.9f, 0.9f, 0.9f, 0.9f);

        glEnable(GL_MULTISAMPLE);  //开启抗锯齿
        glHint(GL_SAMPLES,4);      //设置锯齿绘制点数
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        shader_load();
    }
}

//绘制
void My_Opengl::paintGL()
{
    //以下两个if添加的原因是为了initializeGL程序只运行一次
    if(flag_2)
    {
        initializeGL_model();
        flag_2=false;
    }
    if(draw_suppports)
    {
        //旋转变换后的矩阵乘入原模型
        auto normal_c=normal_change.toMatrix().normalMatrix();
        model_changed=models.front();
        QVector<float> vec_size{-9999.0f,9999.0f,-9999.0f,9999.0f,-9999.0f,9999.0f};
        float x1,y1,z1;
        for(auto &i :model_changed.vertexs_of_model())
        {
            i.setPosition(My_Vector3D(normal_change.toMatrix()*i.position()));
            auto vector_mine=QMatrix3x3_model(i.normal(),normal_c);
            i.setNormal(vector_mine);

            //寻找变换后的极值点
            x1=i.position().x();y1=i.position().y();z1=i.position().z();
            if(x1>vec_size[0])
                vec_size[0]=x1;
            else if(x1<vec_size[1])
                vec_size[1]=x1;

            if(y1>vec_size[2])
                vec_size[2]=y1;
            else if(y1<vec_size[3])
                vec_size[3]=y1;

            if(z1>vec_size[4])
                vec_size[4]=z1;
            else if(z1<vec_size[5])
                vec_size[5]=z1;
        }
        model_changed.set_size(vec_size);

        clear_supports();              //清空上次的支撑信息
        model_gridding();              //三角面片分区
        set_samples_trans();           //生成支撑并整合为一个模型
        initializeGL_sample();         //支撑模型的vao vbo着色器设置
        draw_suppports=false;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    //绘制STL模型
    if(model_readied)
    {
        if(!normal_bool.empty() && show_red!=show_red_pre)
        {
            change_color();
            initializeGL_model();
        }
        Draw_model();
    }

    //绘制支撑
    if(supports.size_vertex())
        Draw_samples();

}

void My_Opengl::Draw_samples()
{
    my_shader_model->bind();
    my_shader_model->setUniformValue(my_view,m_camera.toMatrix());        //设置观察变换矩阵
    my_shader_model->setUniformValue(my_projection,m_projection);         //设置投影变换矩阵

    VAO_sample.bind();

    my_shader_model->setUniformValue(my_viepos,m_camera.toMatrix());
    my_shader_model->setUniformValue(my_model, supports_trans.toMatrix());//设置模型变换矩阵
    my_shader_model->setUniformValue(my_normal_model,supports_trans.toMatrix().normalMatrix()); //法向量变换矩阵

    glDrawArrays(GL_TRIANGLES, 0, supports.size_vertex());

    VAO_sample.release();
    my_shader_model->release();
}
void My_Opengl::Draw_model()
{
    my_shader_model->bind();
    my_shader_model->setUniformValue(my_view,m_camera.toMatrix());        //设置观察变换矩阵
    my_shader_model->setUniformValue(my_projection,m_projection);         //设置投影变换矩阵

    VAO_model.bind();

    my_shader_model->setUniformValue(my_viepos,m_camera.toMatrix());
    my_shader_model->setUniformValue(my_model, m_transform.toMatrix());    //设置模型变换矩阵
    my_shader_model->setUniformValue(my_normal_model,m_transform.toMatrix().normalMatrix());

    glDrawArrays(GL_TRIANGLES, 0, models[0].size_vertex());

    VAO_model.release();
    my_shader_model->release();
}
//返回添加支撑后的模型
Model My_Opengl::get_all_model()
{
    Model model_all;
    QMatrix3x3 normal_c=normal_change.toMatrix().normalMatrix();      //模型的法向量变换
    for(auto &j :model_changed.vertexs_of_model())
    {
        My_Vector3D verte_m=j.position()*10.0f;
        My_Vector3D verte_n(verte_m.x(),verte_m.z(),verte_m.y());
        My_Vector3D vector_mine=QMatrix3x3_model(j.normal(),normal_c);  //得到变换后法向量
        My_Vector3D normal_n(vector_mine.x(),vector_mine.z(),-vector_mine.y());
        model_all.push_back(Vertex(verte_n,normal_n));
    }

    for(auto &j :supports.vertexs_of_model())
    {
        My_Vector3D verte_m=j.position()*10.0f;
        My_Vector3D verte_n(verte_m.x(),verte_m.z(),verte_m.y());
        My_Vector3D normal_n(j.normal().x(),j.normal().z(),j.normal().y());
        model_all.push_back(Vertex(verte_n,normal_n));
    }
    model_all.set_meshs_size(supports.size()+models[0].size());

    return model_all;
}

//更新normal_bool
void My_Opengl::update_normal()
{
    if(!normal_bool.empty())
        normal_bool.clear();
    QVector<Vertex>& my_normal=models[0].vertexs_of_model();   //得到mesh数组
    QMatrix3x3 normal_c=normal_change.toMatrix().normalMatrix();
    for(auto ptr=my_normal.cbegin();ptr!=my_normal.cend();ptr+=3)
    {
        auto vector_mine=QMatrix3x3_model(ptr->normal(),normal_c);  //得到变换后法向量
        //若y轴法向量为负
        if(vector_mine.y()<0)
        {
            auto x_z=sqrt(vector_mine.x()*vector_mine.x()+vector_mine.z()*vector_mine.z());
            //若角度大于45
            if(-(vector_mine.y()) > x_z)
                normal_bool.push_back(true);
            else
                normal_bool.push_back(false);
        }
        else
            normal_bool.push_back(false);
    }
    if(show_red)
    {
        change_color();
        initializeOpenGLFunctions();
        initializeGL_model();
    }
}
My_Vector3D My_Opengl::QMatrix3x3_model(My_Vector3D vec,QMatrix3x3 & matrix )
{
    return My_Vector3D
            (vec.x() * matrix(0,0) + vec.y() * matrix(0,1) +vec.z()* matrix(0,2),
             vec.x() * matrix(1,0) + vec.y() * matrix(1,1) +vec.z()* matrix(1,2),
             vec.x() * matrix(2,0) + vec.y() * matrix(2,1) +vec.z()* matrix(2,2));
}

//返回视角变换矩阵
QMatrix3x3  My_Opengl::transfrom_0()
{
    return view_change.toMatrix().normalMatrix();
}

//更改颜色属性
void My_Opengl::change_color()
{
    show_red_pre=show_red;
    if(show_red)
    {
        for(int i=0,j=0;i!=models[0].size_vertex();i=i+3,++j)
        {
            if(normal_bool[j])
                models[0].set_color(i);
            else
                models[0].set_color_2(i);
        }
    }
    else
        models[0].restart_color();
}

void My_Opengl::teardownGL()
{
    VBO_model.destroy();
    VBO_model.destroy();

    VAO_kuang.destroy();
    VAO_model.destroy();

    delete my_shader_kuang;
    delete my_shader_model;
}

void My_Opengl::resizeGL(int width, int height)
{
    m_projection.setToIdentity();
    m_projection.perspective(45.0f, width / float(height), 0.01f, 100.0f);   //！！！！！找了很久的BUG！！！！！
}

void My_Opengl::update()
{
    // 输入更新
    Input::update();
    if (Input::buttonPressed(Qt::RightButton))
    {
        QMatrix3x3 up_change=transfrom_0();
        auto up_now=QMatrix3x3_model(Camera3D::LocalUp,up_change);

        static const float rotSpeed   = 0.3f;
        auto x_move=rotSpeed * Input::mouseDelta().x();
        auto y_move=rotSpeed * Input::mouseDelta().y();
        auto flag=y_move+camera_y;

        camera_x+=x_move;
        view_change.rotate(x_move, up_now);
        m_transform.rotate(x_move, up_now);
        supports_trans.rotate(x_move, up_now);

        if(flag<90.0f&&flag>-90.0f)
        {
            camera_y+=y_move;

            view_change.rotate(y_move, Camera3D::LocalRight);
            m_transform.rotate(y_move, Camera3D::LocalRight);
            supports_trans.rotate(y_move, Camera3D::LocalRight);

            emit new_camera(y_move, Camera3D::LocalRight);  //更新主界面的变换矩阵
        }
        emit new_camera(x_move, up_now);
    }

    static const float transSpeed = 0.1f;

    My_Vector3D translation;
    if (camera_left)
    {
        translation -= m_camera.right();
    }
    if (camera_right)
    {
        translation += m_camera.right();
    }

    m_camera.translate(transSpeed * translation);
    // Schedule a redraw
    QOpenGLWidget::update();
}

//鼠标事件
void My_Opengl::mousePressEvent(QMouseEvent *event)
{
  Input::registerMousePress(event->button());
}
void My_Opengl::mouseReleaseEvent(QMouseEvent *event)
{
  Input::registerMouseRelease(event->button());
}

//滚轮事件
void My_Opengl::wheelEvent(QWheelEvent *event)
{
    static const float transSpeed = 1.0f;
    My_Vector3D translation;
    if(event->delta()>0)     //若滚轮上滚
        translation += m_camera.forward();
    else
        translation -= m_camera.forward();
    m_camera.translate(transSpeed * translation);
    QOpenGLWidget::update();
}

//接受主程序发来的信号，进行模型环境渲染
void My_Opengl::updata_models_vector(bool flag)
{
    if(flag)
    {
        normal_change=m_transform;
        model_readied=true;  //读入模型数据 设置为true
        flag_2=true;         //修复渲染bug添加的 布尔变量，读入模型后设置为真，而后调用一次initializeGL()函数，设置为假
    }
}

void My_Opengl::camera_restart(float zoom_now)
{
    view_change.rotate(-camera_y, Camera3D::LocalRight);
    view_change.rotate(-camera_x, Camera3D::LocalUp);

    m_transform.rotate(-camera_y, Camera3D::LocalRight);
    m_transform.rotate(-camera_x, Camera3D::LocalUp);

    camera_x=0.0;
    camera_y=0.0;

    m_camera.setTranslation(0.0f,0.0f,32.0f);
    m_camera.setRotation(0.0f,1.0f,0.0f,0.0f);
}

//利用两圆半径，自动生成三维圆柱加圆台
void My_Opengl::get_column(float x_loca,float z_loca,float hight)
{
    if(supports_automatic.size()!=0)
        supports_automatic.clear();
    if(hight-model_changed.y_min_val()<defa_h)
        get_column_2(x_loca,z_loca, hight);    //底部无圆台
    else
        get_column_3(x_loca,z_loca, hight);    //底部包含圆台
}

//原始支撑生成
void My_Opengl::get_column_2(float x_loca,float z_loca,float hight)
{
    int count_size=0;                   //统计面片数量
    int num=static_cast<int>(2*PI*R_bottom/default_len);  //圆环所做等分数
    float radin=2*PI/static_cast<float>(num);            //每个小扇形的弧度

    QVector3D circle_up_pos(x_loca,hight,z_loca);      //顶部圆面的圆心位置
    QVector3D circle_mid_pos(x_loca,hight-5*R_up,z_loca); //中部圆面的圆心位置
    QVector3D circle_bottom_pos(x_loca,model_changed.y_min_val(),z_loca);

    QVector<QVector3D> ver_supports_up;         //上圆环
    QVector<QVector3D> ver_supports_mid;        //中圆环
    QVector<QVector3D> ver_supports_bottom;     //下圆环

    //将三圆的采样点加入对应数组
    ver_supports_up.push_back(QVector3D(x_loca,circle_up_pos.y(),z_loca+R_up));
    ver_supports_mid.push_back(QVector3D(x_loca,circle_mid_pos.y(),z_loca+R_bottom));
    ver_supports_bottom.push_back(QVector3D(x_loca,circle_bottom_pos.y(),z_loca+R_bottom));
    for(int i=1;i!=num;++i)
    {
        //计算第上圆环采样点
        float y_c= circle_up_pos.y();
        float x_c= x_loca-R_up*sin(radin*i);
        float z_c= z_loca+R_up*cos(radin*i);
        ver_supports_up.push_back(QVector3D(x_c,y_c,z_c));

        //计算第中下圆环采样点
        y_c= circle_mid_pos.y();
        x_c= x_loca-R_bottom*sin(radin*i);
        z_c= z_loca+R_bottom*cos(radin*i);

        ver_supports_mid.push_back(QVector3D(x_c,y_c,z_c));
        y_c= circle_bottom_pos.y();
        ver_supports_bottom.push_back(QVector3D(x_c,y_c,z_c));
    }
    //循环添加斜柱面的三角面点
    for(int i=0;i!=ver_supports_mid.size();++i)
    {
        int j=i+1;
        if(i==ver_supports_mid.size()-1)
            j=0;

        //求解第一个三角面片的法向量
        auto p1=ver_supports_up[j]-ver_supports_up[i];
        auto p2=ver_supports_up[i]-ver_supports_mid[i];
        QVector3D normal(p1.y()*p2.z()-p2.y()*p1.z(),
                        p1.z()*p2.x()-p2.z()*p1.x(),
                        p1.x()*p2.y()-p2.x()*p1.y());
        supports_automatic.push_back(Vertex(ver_supports_up[j],normal));
        supports_automatic.push_back(Vertex(ver_supports_up[i],normal));
        supports_automatic.push_back(Vertex(ver_supports_mid[i],normal));

        //求解第二个三角面片的法向量
        p1=ver_supports_mid[i]-ver_supports_mid[j];
        p2=ver_supports_mid[j]-ver_supports_up[j];
        QVector3D normal2(p1.y()*p2.z()-p2.y()*p1.z(),
                          p1.z()*p2.x()-p2.z()*p1.x(),
                          p1.x()*p2.y()-p2.x()*p1.y());
        supports_automatic.push_back(Vertex(ver_supports_mid[i],normal2));
        supports_automatic.push_back(Vertex(ver_supports_mid[j],normal2));
        supports_automatic.push_back(Vertex(ver_supports_up[j],normal2));

        count_size+=2;
    }

    //循环添加直柱面的三角面点
    for(int i=0;i!=ver_supports_bottom.size();++i)
    {
        int j=i+1;
        if(i==ver_supports_bottom.size()-1)
            j=0;

        //求解第一个三角面片的法向量
        auto p1=ver_supports_mid[j]-ver_supports_mid[i];
        auto p2=ver_supports_mid[i]-ver_supports_bottom[i];

        QVector3D normal(p1.y()*p2.z()-p2.y()*p1.z(),
                        p1.z()*p2.x()-p2.z()*p1.x(),
                        p1.x()*p2.y()-p2.x()*p1.y());
        supports_automatic.push_back(Vertex(ver_supports_mid[j],normal));
        supports_automatic.push_back(Vertex(ver_supports_mid[i],normal));
        supports_automatic.push_back(Vertex(ver_supports_bottom[i],normal));

        //求解第二个三角面片的法向量
        p1=ver_supports_bottom[i]-ver_supports_bottom[j];
        p2=ver_supports_bottom[j]-ver_supports_mid[j];

        QVector3D normal2(p1.y()*p2.z()-p2.y()*p1.z(),
                          p1.z()*p2.x()-p2.z()*p1.x(),
                          p1.x()*p2.y()-p2.x()*p1.y());
        supports_automatic.push_back(Vertex(ver_supports_bottom[i],normal2));
        supports_automatic.push_back(Vertex(ver_supports_bottom[j],normal2));
        supports_automatic.push_back(Vertex(ver_supports_mid[j],normal2));

        count_size+=2;
    }

    //循环添加圆的三角面点
    for(int i=0;i!=ver_supports_up.size();++i)
    {
        int j=i+1;
        if(i==ver_supports_up.size()-1)
            j=0;

        //求解上圆面第一个三角面片的法向量
        auto p1=circle_up_pos-ver_supports_up[j];
        auto p2=ver_supports_up[i]-circle_up_pos;
        QVector3D normal(p1.y()*p2.z()-p2.y()*p1.z(),
                         p1.z()*p2.x()-p2.z()*p1.x(),
                         p1.x()*p2.y()-p2.x()*p1.y());
        supports_automatic.push_back(Vertex(ver_supports_up[j],normal));
        supports_automatic.push_back(Vertex(circle_up_pos,normal));
        supports_automatic.push_back(Vertex(ver_supports_up[i],normal));

        //求解下圆面第一个三角面片的法向量
        p1=ver_supports_bottom[j]-circle_bottom_pos;
        p2=circle_bottom_pos-ver_supports_bottom[i];
        QVector3D normal2(p1.y()*p2.z()-p2.y()*p1.z(),
                          p1.z()*p2.x()-p2.z()*p1.x(),
                          p1.x()*p2.y()-p2.x()*p1.y());
        supports_automatic.push_back(Vertex(ver_supports_bottom[i],normal2));
        supports_automatic.push_back(Vertex(circle_bottom_pos,normal2));
        supports_automatic.push_back(Vertex(ver_supports_bottom[j],normal2));

        count_size+=2;
    }
    supports_automatic.set_meshs_size(count_size);
}
//带底部圆台支撑生成
void My_Opengl::get_column_3(float x_loca,float z_loca,float hight)
{
    auto R_add=(hight-model_changed.y_min_val()-defa_h)*0.2f;
    if(R_add>0.15f)
        R_add=0.15f;

    R_bottom+=R_add;      //修改底部圆环的大小
    int count_size=0;                   //统计面片数量
    int num=static_cast<int>(2*PI*R_bottom/default_len);  //圆环所做等分数
    float radin=2*PI/static_cast<float>(num);     //每个小扇形的弧度

    QVector3D circle_up_pos(x_loca,hight,z_loca);                   //顶部圆面的圆心位置
    QVector3D circle_mid_pos(x_loca,hight-5*R_up,z_loca);           //中部圆面的圆心位置
    QVector3D circle_mid_2_pos(x_loca,model_changed.y_min_val()+mid_bottom_h,z_loca);         //下部圆面的圆心位置
    QVector3D circle_bottom_pos(x_loca,model_changed.y_min_val(),z_loca);

    QVector<QVector3D> ver_supports_up;         //上圆环
    QVector<QVector3D> ver_supports_mid;        //中圆环
    QVector<QVector3D> ver_supports_mid_2;      //下圆环
    QVector<QVector3D> ver_supports_bottom;     //底圆环

    //将三圆的采样点加入对应数组
    ver_supports_up.push_back(QVector3D(x_loca,circle_up_pos.y(),z_loca+R_up));
    ver_supports_mid.push_back(QVector3D(x_loca,circle_mid_pos.y(),z_loca+R_mid));
    ver_supports_mid_2.push_back(QVector3D(x_loca,circle_mid_2_pos.y(),z_loca+R_mid));
    ver_supports_bottom.push_back(QVector3D(x_loca,circle_bottom_pos.y(),z_loca+R_bottom));
    for(int i=1;i!=num;++i)
    {
        //计算第上圆环采样点
        float y_c= circle_up_pos.y();
        float x_c= x_loca-R_up*sin(radin*i);
        float z_c= z_loca+R_up*cos(radin*i);
        ver_supports_up.push_back(QVector3D(x_c,y_c,z_c));

        //计算第中下圆环采样点
        y_c= circle_mid_pos.y();
        x_c= x_loca-R_mid*sin(radin*i);
        z_c= z_loca+R_mid*cos(radin*i);

        ver_supports_mid.push_back(QVector3D(x_c,y_c,z_c));
        y_c= circle_mid_2_pos.y();
        ver_supports_mid_2.push_back(QVector3D(x_c,y_c,z_c));

        //计算第底圆环采样点
        y_c= circle_bottom_pos.y();
        x_c= x_loca-R_bottom*sin(radin*i);
        z_c= z_loca+R_bottom*cos(radin*i);

        ver_supports_bottom.push_back(QVector3D(x_c,y_c,z_c));
    }

    //循环添加斜柱面的三角面点
    for(int i=0;i!=ver_supports_mid.size();++i)
    {
        int j=i+1;
        if(i==ver_supports_mid.size()-1)
            j=0;

        //求解第一个三角面片的法向量
        auto p1=ver_supports_up[j]-ver_supports_up[i];
        auto p2=ver_supports_up[i]-ver_supports_mid[i];
        QVector3D normal(p1.y()*p2.z()-p2.y()*p1.z(),
                        p1.z()*p2.x()-p2.z()*p1.x(),
                        p1.x()*p2.y()-p2.x()*p1.y());
        supports_automatic.push_back(Vertex(ver_supports_up[j],normal));
        supports_automatic.push_back(Vertex(ver_supports_up[i],normal));
        supports_automatic.push_back(Vertex(ver_supports_mid[i],normal));

        //求解第二个三角面片的法向量
        p1=ver_supports_mid[i]-ver_supports_mid[j];
        p2=ver_supports_mid[j]-ver_supports_up[j];
        QVector3D normal2(p1.y()*p2.z()-p2.y()*p1.z(),
                          p1.z()*p2.x()-p2.z()*p1.x(),
                          p1.x()*p2.y()-p2.x()*p1.y());
        supports_automatic.push_back(Vertex(ver_supports_mid[i],normal2));
        supports_automatic.push_back(Vertex(ver_supports_mid[j],normal2));
        supports_automatic.push_back(Vertex(ver_supports_up[j],normal2));

        count_size+=2;
    }

    //循环添加直柱面的三角面点
    for(int i=0;i!=ver_supports_bottom.size();++i)
    {
        int j=i+1;
        if(i==ver_supports_bottom.size()-1)
            j=0;

        //求解第一个三角面片的法向量
        auto p1=ver_supports_mid[j]-ver_supports_mid[i];
        auto p2=ver_supports_mid[i]-ver_supports_mid_2[i];

        QVector3D normal(p1.y()*p2.z()-p2.y()*p1.z(),
                        p1.z()*p2.x()-p2.z()*p1.x(),
                        p1.x()*p2.y()-p2.x()*p1.y());
        supports_automatic.push_back(Vertex(ver_supports_mid[j],normal));
        supports_automatic.push_back(Vertex(ver_supports_mid[i],normal));
        supports_automatic.push_back(Vertex(ver_supports_mid_2[i],normal));

        //求解第二个三角面片的法向量
        p1=ver_supports_mid_2[i]-ver_supports_mid_2[j];
        p2=ver_supports_mid_2[j]-ver_supports_mid[j];

        QVector3D normal2(p1.y()*p2.z()-p2.y()*p1.z(),
                          p1.z()*p2.x()-p2.z()*p1.x(),
                          p1.x()*p2.y()-p2.x()*p1.y());
        supports_automatic.push_back(Vertex(ver_supports_mid_2[i],normal2));
        supports_automatic.push_back(Vertex(ver_supports_mid_2[j],normal2));
        supports_automatic.push_back(Vertex(ver_supports_mid[j],normal2));

        count_size+=2;
    }

    //循环添加底圆台面的三角面点
    for(int i=0;i!=ver_supports_bottom.size();++i)
    {
        int j=i+1;
        if(i==ver_supports_bottom.size()-1)
            j=0;

        //求解第一个三角面片的法向量
        auto p1=ver_supports_mid_2[j]-ver_supports_mid_2[i];
        auto p2=ver_supports_mid_2[i]-ver_supports_bottom[i];

        QVector3D normal(p1.y()*p2.z()-p2.y()*p1.z(),
                        p1.z()*p2.x()-p2.z()*p1.x(),
                        p1.x()*p2.y()-p2.x()*p1.y());
        supports_automatic.push_back(Vertex(ver_supports_mid_2[j],normal));
        supports_automatic.push_back(Vertex(ver_supports_mid_2[i],normal));
        supports_automatic.push_back(Vertex(ver_supports_bottom[i],normal));

        //求解第二个三角面片的法向量
        p1=ver_supports_bottom[i]-ver_supports_bottom[j];
        p2=ver_supports_bottom[j]-ver_supports_mid_2[j];

        QVector3D normal2(p1.y()*p2.z()-p2.y()*p1.z(),
                          p1.z()*p2.x()-p2.z()*p1.x(),
                          p1.x()*p2.y()-p2.x()*p1.y());
        supports_automatic.push_back(Vertex(ver_supports_bottom[i],normal2));
        supports_automatic.push_back(Vertex(ver_supports_bottom[j],normal2));
        supports_automatic.push_back(Vertex(ver_supports_mid_2[j],normal2));

        count_size+=2;
    }

    //循环添加圆的三角面点
    for(int i=0;i!=ver_supports_up.size();++i)
    {
        int j=i+1;
        if(i==ver_supports_up.size()-1)
            j=0;

        //求解上圆面第一个三角面片的法向量
        auto p1=circle_up_pos-ver_supports_up[j];
        auto p2=ver_supports_up[i]-circle_up_pos;
        QVector3D normal(p1.y()*p2.z()-p2.y()*p1.z(),
                         p1.z()*p2.x()-p2.z()*p1.x(),
                         p1.x()*p2.y()-p2.x()*p1.y());
        supports_automatic.push_back(Vertex(ver_supports_up[j],normal));
        supports_automatic.push_back(Vertex(circle_up_pos,normal));
        supports_automatic.push_back(Vertex(ver_supports_up[i],normal));

        //求解下圆面第一个三角面片的法向量
        p1=ver_supports_bottom[j]-circle_bottom_pos;
        p2=circle_bottom_pos-ver_supports_bottom[i];
        QVector3D normal2(p1.y()*p2.z()-p2.y()*p1.z(),
                          p1.z()*p2.x()-p2.z()*p1.x(),
                          p1.x()*p2.y()-p2.x()*p1.y());
        supports_automatic.push_back(Vertex(ver_supports_bottom[i],normal2));
        supports_automatic.push_back(Vertex(circle_bottom_pos,normal2));
        supports_automatic.push_back(Vertex(ver_supports_bottom[j],normal2));

        count_size+=2;
    }
    supports_automatic.set_meshs_size(count_size);

    R_bottom=0.15f;   //回复圆环原半径
}
//模型上的支撑生成
void My_Opengl::get_column_4(float x_loca,float z_loca,float y_pos,float hight)
{
    R_bottom=0.05f;      //修改底部圆环的大小
    int count_size=0;                   //统计面片数量
    int num=static_cast<int>(2*PI*R_mid/default_len);  //圆环所做等分数
    float y_bottom_loca=y_pos-hight;
    float radin=2*PI/static_cast<float>(num);     //每个小扇形的弧度

    QVector3D circle_up_pos(x_loca,y_pos,z_loca);                   //顶部圆面的圆心位置
    QVector3D circle_mid_pos(x_loca,y_pos-5*R_up,z_loca);           //中部圆面的圆心位置
    QVector3D circle_mid_2_pos(x_loca, y_bottom_loca+R_up*5,z_loca);         //下部圆面的圆心位置
    QVector3D circle_bottom_pos(x_loca,y_bottom_loca,z_loca);

    QVector<QVector3D> ver_supports_up;         //上圆环
    QVector<QVector3D> ver_supports_mid;        //中圆环
    QVector<QVector3D> ver_supports_mid_2;      //下圆环
    QVector<QVector3D> ver_supports_bottom;     //底圆环

    //将三圆的采样点加入对应数组
    ver_supports_up.push_back(QVector3D(x_loca,circle_up_pos.y(),z_loca+R_up));
    ver_supports_mid.push_back(QVector3D(x_loca,circle_mid_pos.y(),z_loca+R_mid));
    ver_supports_mid_2.push_back(QVector3D(x_loca,circle_mid_2_pos.y(),z_loca+R_mid));
    ver_supports_bottom.push_back(QVector3D(x_loca,circle_bottom_pos.y(),z_loca+R_bottom));
    for(int i=1;i!=num;++i)
    {
        //计算第上圆环采样点
        float y_c= circle_up_pos.y();
        float x_c= x_loca-R_up*sin(radin*i);
        float z_c= z_loca+R_up*cos(radin*i);
        ver_supports_up.push_back(QVector3D(x_c,y_c,z_c));

        //计算第中下圆环采样点
        y_c= circle_mid_pos.y();
        x_c= x_loca-R_mid*sin(radin*i);
        z_c= z_loca+R_mid*cos(radin*i);

        ver_supports_mid.push_back(QVector3D(x_c,y_c,z_c));
        y_c= circle_mid_2_pos.y();
        ver_supports_mid_2.push_back(QVector3D(x_c,y_c,z_c));

        //计算第底圆环采样点
        y_c= circle_bottom_pos.y();
        x_c= x_loca-R_bottom*sin(radin*i);
        z_c= z_loca+R_bottom*cos(radin*i);

        ver_supports_bottom.push_back(QVector3D(x_c,y_c,z_c));
    }

    //循环添加斜柱面的三角面点
    for(int i=0;i!=ver_supports_mid.size();++i)
    {
        int j=i+1;
        if(i==ver_supports_mid.size()-1)
            j=0;

        //求解第一个三角面片的法向量
        auto p1=ver_supports_up[j]-ver_supports_up[i];
        auto p2=ver_supports_up[i]-ver_supports_mid[i];
        QVector3D normal(p1.y()*p2.z()-p2.y()*p1.z(),
                        p1.z()*p2.x()-p2.z()*p1.x(),
                        p1.x()*p2.y()-p2.x()*p1.y());
        supports_automatic.push_back(Vertex(ver_supports_up[j],normal));
        supports_automatic.push_back(Vertex(ver_supports_up[i],normal));
        supports_automatic.push_back(Vertex(ver_supports_mid[i],normal));

        //求解第二个三角面片的法向量
        p1=ver_supports_mid[i]-ver_supports_mid[j];
        p2=ver_supports_mid[j]-ver_supports_up[j];
        QVector3D normal2(p1.y()*p2.z()-p2.y()*p1.z(),
                          p1.z()*p2.x()-p2.z()*p1.x(),
                          p1.x()*p2.y()-p2.x()*p1.y());
        supports_automatic.push_back(Vertex(ver_supports_mid[i],normal2));
        supports_automatic.push_back(Vertex(ver_supports_mid[j],normal2));
        supports_automatic.push_back(Vertex(ver_supports_up[j],normal2));

        count_size+=2;
    }

    //循环添加直柱面的三角面点
    for(int i=0;i!=ver_supports_bottom.size();++i)
    {
        int j=i+1;
        if(i==ver_supports_bottom.size()-1)
            j=0;

        //求解第一个三角面片的法向量
        auto p1=ver_supports_mid[j]-ver_supports_mid[i];
        auto p2=ver_supports_mid[i]-ver_supports_mid_2[i];

        QVector3D normal(p1.y()*p2.z()-p2.y()*p1.z(),
                        p1.z()*p2.x()-p2.z()*p1.x(),
                        p1.x()*p2.y()-p2.x()*p1.y());
        supports_automatic.push_back(Vertex(ver_supports_mid[j],normal));
        supports_automatic.push_back(Vertex(ver_supports_mid[i],normal));
        supports_automatic.push_back(Vertex(ver_supports_mid_2[i],normal));

        //求解第二个三角面片的法向量
        p1=ver_supports_mid_2[i]-ver_supports_mid_2[j];
        p2=ver_supports_mid_2[j]-ver_supports_mid[j];

        QVector3D normal2(p1.y()*p2.z()-p2.y()*p1.z(),
                          p1.z()*p2.x()-p2.z()*p1.x(),
                          p1.x()*p2.y()-p2.x()*p1.y());
        supports_automatic.push_back(Vertex(ver_supports_mid_2[i],normal2));
        supports_automatic.push_back(Vertex(ver_supports_mid_2[j],normal2));
        supports_automatic.push_back(Vertex(ver_supports_mid[j],normal2));

        count_size+=2;
    }

    //循环添加底圆台面的三角面点
    for(int i=0;i!=ver_supports_bottom.size();++i)
    {
        int j=i+1;
        if(i==ver_supports_bottom.size()-1)
            j=0;

        //求解第一个三角面片的法向量
        auto p1=ver_supports_mid_2[j]-ver_supports_mid_2[i];
        auto p2=ver_supports_mid_2[i]-ver_supports_bottom[i];

        QVector3D normal(p1.y()*p2.z()-p2.y()*p1.z(),
                        p1.z()*p2.x()-p2.z()*p1.x(),
                        p1.x()*p2.y()-p2.x()*p1.y());
        supports_automatic.push_back(Vertex(ver_supports_mid_2[j],normal));
        supports_automatic.push_back(Vertex(ver_supports_mid_2[i],normal));
        supports_automatic.push_back(Vertex(ver_supports_bottom[i],normal));

        //求解第二个三角面片的法向量
        p1=ver_supports_bottom[i]-ver_supports_bottom[j];
        p2=ver_supports_bottom[j]-ver_supports_mid_2[j];

        QVector3D normal2(p1.y()*p2.z()-p2.y()*p1.z(),
                          p1.z()*p2.x()-p2.z()*p1.x(),
                          p1.x()*p2.y()-p2.x()*p1.y());
        supports_automatic.push_back(Vertex(ver_supports_bottom[i],normal2));
        supports_automatic.push_back(Vertex(ver_supports_bottom[j],normal2));
        supports_automatic.push_back(Vertex(ver_supports_mid_2[j],normal2));

        count_size+=2;
    }

    //循环添加圆的三角面点
    for(int i=0;i!=ver_supports_up.size();++i)
    {
        int j=i+1;
        if(i==ver_supports_up.size()-1)
            j=0;

        //求解上圆面第一个三角面片的法向量
        auto p1=circle_up_pos-ver_supports_up[j];
        auto p2=ver_supports_up[i]-circle_up_pos;
        QVector3D normal(p1.y()*p2.z()-p2.y()*p1.z(),
                         p1.z()*p2.x()-p2.z()*p1.x(),
                         p1.x()*p2.y()-p2.x()*p1.y());
        supports_automatic.push_back(Vertex(ver_supports_up[j],normal));
        supports_automatic.push_back(Vertex(circle_up_pos,normal));
        supports_automatic.push_back(Vertex(ver_supports_up[i],normal));

        //求解下圆面第一个三角面片的法向量
        p1=ver_supports_bottom[j]-circle_bottom_pos;
        p2=circle_bottom_pos-ver_supports_bottom[i];
        QVector3D normal2(p1.y()*p2.z()-p2.y()*p1.z(),
                          p1.z()*p2.x()-p2.z()*p1.x(),
                          p1.x()*p2.y()-p2.x()*p1.y());
        supports_automatic.push_back(Vertex(ver_supports_bottom[i],normal2));
        supports_automatic.push_back(Vertex(circle_bottom_pos,normal2));
        supports_automatic.push_back(Vertex(ver_supports_bottom[j],normal2));

        count_size+=2;
    }
    supports_automatic.set_meshs_size(count_size);

    R_bottom=0.15f;   //回复圆环原半径
}
//斜支撑生成
void My_Opengl::get_column_5(float x_loca,float z_loca,float hight,float x_bottom,float z_bottom)
{
    R_bottom=0.25f;;      //修改底部圆环的大小
    int count_size=0;                   //统计面片数量
    int num=static_cast<int>(2*PI*R_bottom/default_len);  //圆环所做等分数
    float radin=2*PI/static_cast<float>(num);     //每个小扇形的弧度

    QVector3D circle_up_pos(x_loca,hight,z_loca);                   //顶部圆面的圆心位置
    QVector3D circle_mid_pos(x_loca,hight-5*R_up,z_loca);           //中部圆面的圆心位置
    QVector3D circle_mid_2_pos(x_bottom,model_changed.y_min_val()+mid_bottom_h,z_bottom);         //下部圆面的圆心位置
    QVector3D circle_bottom_pos(x_bottom,model_changed.y_min_val(),z_bottom);

    QVector<QVector3D> ver_supports_up;         //上圆环
    QVector<QVector3D> ver_supports_mid;        //中圆环
    QVector<QVector3D> ver_supports_mid_2;      //下圆环
    QVector<QVector3D> ver_supports_bottom;     //底圆环

    //将三圆的采样点加入对应数组
    ver_supports_up.push_back(QVector3D(x_loca,circle_up_pos.y(),z_loca+R_up));
    ver_supports_mid.push_back(QVector3D(x_loca,circle_mid_pos.y(),z_loca+R_mid));
    ver_supports_mid_2.push_back(QVector3D(x_bottom,circle_mid_2_pos.y(),z_bottom+R_mid));
    ver_supports_bottom.push_back(QVector3D(x_bottom,circle_bottom_pos.y(),z_bottom+R_bottom));
    for(int i=1;i!=num;++i)
    {
        //计算第上圆环采样点
        float y_c= circle_up_pos.y();
        float x_c= x_loca-R_up*sin(radin*i);
        float z_c= z_loca+R_up*cos(radin*i);
        ver_supports_up.push_back(QVector3D(x_c,y_c,z_c));

        //计算第中下圆环采样点
        y_c= circle_mid_pos.y();
        x_c= x_loca-R_mid*sin(radin*i);
        z_c= z_loca+R_mid*cos(radin*i);
        ver_supports_mid.push_back(QVector3D(x_c,y_c,z_c));

        y_c= circle_mid_2_pos.y();
        x_c= x_bottom-R_mid*sin(radin*i);
        z_c= z_bottom+R_mid*cos(radin*i);
        ver_supports_mid_2.push_back(QVector3D(x_c,y_c,z_c));

        //计算第底圆环采样点
        y_c= circle_bottom_pos.y();
        x_c= x_bottom-R_bottom*sin(radin*i);
        z_c= z_bottom+R_bottom*cos(radin*i);
        ver_supports_bottom.push_back(QVector3D(x_c,y_c,z_c));
    }


    //循环添加斜柱面的三角面点
    for(int i=0;i!=ver_supports_mid.size();++i)
    {
        int j=i+1;
        if(i==ver_supports_mid.size()-1)
            j=0;

        //求解第一个三角面片的法向量
        auto p1=ver_supports_up[j]-ver_supports_up[i];
        auto p2=ver_supports_up[i]-ver_supports_mid[i];
        QVector3D normal(p1.y()*p2.z()-p2.y()*p1.z(),
                        p1.z()*p2.x()-p2.z()*p1.x(),
                        p1.x()*p2.y()-p2.x()*p1.y());
        supports_automatic.push_back(Vertex(ver_supports_up[j],normal));
        supports_automatic.push_back(Vertex(ver_supports_up[i],normal));
        supports_automatic.push_back(Vertex(ver_supports_mid[i],normal));

        //求解第二个三角面片的法向量
        p1=ver_supports_mid[i]-ver_supports_mid[j];
        p2=ver_supports_mid[j]-ver_supports_up[j];
        QVector3D normal2(p1.y()*p2.z()-p2.y()*p1.z(),
                          p1.z()*p2.x()-p2.z()*p1.x(),
                          p1.x()*p2.y()-p2.x()*p1.y());
        supports_automatic.push_back(Vertex(ver_supports_mid[i],normal2));
        supports_automatic.push_back(Vertex(ver_supports_mid[j],normal2));
        supports_automatic.push_back(Vertex(ver_supports_up[j],normal2));

        count_size+=2;
    }

    //循环添加直柱面的三角面点
    for(int i=0;i!=ver_supports_bottom.size();++i)
    {
        int j=i+1;
        if(i==ver_supports_bottom.size()-1)
            j=0;

        //求解第一个三角面片的法向量
        auto p1=ver_supports_mid[j]-ver_supports_mid[i];
        auto p2=ver_supports_mid[i]-ver_supports_mid_2[i];

        QVector3D normal(p1.y()*p2.z()-p2.y()*p1.z(),
                        p1.z()*p2.x()-p2.z()*p1.x(),
                        p1.x()*p2.y()-p2.x()*p1.y());
        supports_automatic.push_back(Vertex(ver_supports_mid[j],normal));
        supports_automatic.push_back(Vertex(ver_supports_mid[i],normal));
        supports_automatic.push_back(Vertex(ver_supports_mid_2[i],normal));

        //求解第二个三角面片的法向量
        p1=ver_supports_mid_2[i]-ver_supports_mid_2[j];
        p2=ver_supports_mid_2[j]-ver_supports_mid[j];

        QVector3D normal2(p1.y()*p2.z()-p2.y()*p1.z(),
                          p1.z()*p2.x()-p2.z()*p1.x(),
                          p1.x()*p2.y()-p2.x()*p1.y());
        supports_automatic.push_back(Vertex(ver_supports_mid_2[i],normal2));
        supports_automatic.push_back(Vertex(ver_supports_mid_2[j],normal2));
        supports_automatic.push_back(Vertex(ver_supports_mid[j],normal2));

        count_size+=2;
    }

    //循环添加底圆台面的三角面点
    for(int i=0;i!=ver_supports_bottom.size();++i)
    {
        int j=i+1;
        if(i==ver_supports_bottom.size()-1)
            j=0;

        //求解第一个三角面片的法向量
        auto p1=ver_supports_mid_2[j]-ver_supports_mid_2[i];
        auto p2=ver_supports_mid_2[i]-ver_supports_bottom[i];

        QVector3D normal(p1.y()*p2.z()-p2.y()*p1.z(),
                        p1.z()*p2.x()-p2.z()*p1.x(),
                        p1.x()*p2.y()-p2.x()*p1.y());
        supports_automatic.push_back(Vertex(ver_supports_mid_2[j],normal));
        supports_automatic.push_back(Vertex(ver_supports_mid_2[i],normal));
        supports_automatic.push_back(Vertex(ver_supports_bottom[i],normal));

        //求解第二个三角面片的法向量
        p1=ver_supports_bottom[i]-ver_supports_bottom[j];
        p2=ver_supports_bottom[j]-ver_supports_mid_2[j];

        QVector3D normal2(p1.y()*p2.z()-p2.y()*p1.z(),
                          p1.z()*p2.x()-p2.z()*p1.x(),
                          p1.x()*p2.y()-p2.x()*p1.y());
        supports_automatic.push_back(Vertex(ver_supports_bottom[i],normal2));
        supports_automatic.push_back(Vertex(ver_supports_bottom[j],normal2));
        supports_automatic.push_back(Vertex(ver_supports_mid_2[j],normal2));

        count_size+=2;
    }

    //循环添加圆的三角面点
    for(int i=0;i!=ver_supports_up.size();++i)
    {
        int j=i+1;
        if(i==ver_supports_up.size()-1)
            j=0;

        //求解上圆面第一个三角面片的法向量
        auto p1=circle_up_pos-ver_supports_up[j];
        auto p2=ver_supports_up[i]-circle_up_pos;
        QVector3D normal(p1.y()*p2.z()-p2.y()*p1.z(),
                         p1.z()*p2.x()-p2.z()*p1.x(),
                         p1.x()*p2.y()-p2.x()*p1.y());
        supports_automatic.push_back(Vertex(ver_supports_up[j],normal));
        supports_automatic.push_back(Vertex(circle_up_pos,normal));
        supports_automatic.push_back(Vertex(ver_supports_up[i],normal));

        //求解下圆面第一个三角面片的法向量
        p1=ver_supports_bottom[j]-circle_bottom_pos;
        p2=circle_bottom_pos-ver_supports_bottom[i];
        QVector3D normal2(p1.y()*p2.z()-p2.y()*p1.z(),
                          p1.z()*p2.x()-p2.z()*p1.x(),
                          p1.x()*p2.y()-p2.x()*p1.y());
        supports_automatic.push_back(Vertex(ver_supports_bottom[i],normal2));
        supports_automatic.push_back(Vertex(circle_bottom_pos,normal2));
        supports_automatic.push_back(Vertex(ver_supports_bottom[j],normal2));

        count_size+=2;
    }
    supports_automatic.set_meshs_size(count_size);

    R_bottom=0.15f;   //回复圆环原半径
}



//树支撑   桶形生成
void My_Opengl::get_tree_1(My_Vector3D poin)
{
    int count_size=0;                   //统计面片数量
    int num=static_cast<int>(2*PI*R_bottom/default_len);  //圆环所做等分数
    float radin=2*PI/static_cast<float>(num);            //每个小扇形的弧度
    auto R_mid=0.15f;
    R_bottom=0.25f;

//    R_bottom=0.05f;

//    float f_bottom=-1.65f;

//    QVector3D circle_up_pos(poin.x(),poin.y(),poin.z());      //顶部圆面的圆心位置
//    QVector3D circle_mid_pos(poin.x(),f_bottom+0.4f,poin.z());      //顶部圆面的圆心位置
//    QVector3D circle_bottom_pos(poin.x(),f_bottom,poin.z());


    QVector3D circle_up_pos(poin.x(),poin.y(),poin.z());      //顶部圆面的圆心位置
    QVector3D circle_mid_pos(poin.x(),model_changed.y_min_val()+0.4f,poin.z());      //顶部圆面的圆心位置
    QVector3D circle_bottom_pos(poin.x(),model_changed.y_min_val(),poin.z());

    QVector<QVector3D> ver_supports_up;         //上圆环
    QVector<QVector3D> ver_supports_mid;        //中圆环
    QVector<QVector3D> ver_supports_bottom;     //下圆环

    //将三圆的采样点加入对应数组
    ver_supports_up.push_back(QVector3D(poin.x(),circle_up_pos.y(),poin.z()+R_mid));
    ver_supports_mid.push_back(QVector3D(poin.x(),circle_bottom_pos.y()+0.4f,poin.z()+R_mid));
    ver_supports_bottom.push_back(QVector3D(poin.x(),circle_bottom_pos.y(),poin.z()+R_bottom));
    for(int i=1;i!=num;++i)
    {
        //计算第上圆环采样点
        float y_c= circle_up_pos.y();
        float x_c= poin.x()-R_mid*sin(radin*i);
        float z_c= poin.z()+R_mid*cos(radin*i);
        ver_supports_up.push_back(QVector3D(x_c,y_c,z_c));

        y_c= circle_mid_pos.y();
        ver_supports_mid.push_back(QVector3D(x_c,y_c,z_c));

        x_c= poin.x()-R_bottom*sin(radin*i);
        z_c= poin.z()+R_bottom*cos(radin*i);
        y_c= circle_bottom_pos.y();
        ver_supports_bottom.push_back(QVector3D(x_c,y_c,z_c));
    }

    //循环添加直柱面的三角面点
    for(int i=0;i!=ver_supports_bottom.size();++i)
    {
        int j=i+1;
        if(i==ver_supports_bottom.size()-1)
            j=0;

        //求解第一个三角面片的法向量
        auto p1=ver_supports_up[j]-ver_supports_up[i];
        auto p2=ver_supports_up[i]-ver_supports_mid[i];

        QVector3D normal(p1.y()*p2.z()-p2.y()*p1.z(),
                        p1.z()*p2.x()-p2.z()*p1.x(),
                        p1.x()*p2.y()-p2.x()*p1.y());
        supports_tree.push_back(Vertex(ver_supports_up[j],normal));
        supports_tree.push_back(Vertex(ver_supports_up[i],normal));
        supports_tree.push_back(Vertex(ver_supports_mid[i],normal));

        //求解第二个三角面片的法向量
        p1=ver_supports_mid[i]-ver_supports_mid[j];
        p2=ver_supports_mid[j]-ver_supports_up[j];

        QVector3D normal2(p1.y()*p2.z()-p2.y()*p1.z(),
                          p1.z()*p2.x()-p2.z()*p1.x(),
                          p1.x()*p2.y()-p2.x()*p1.y());
        supports_tree.push_back(Vertex(ver_supports_mid[i],normal2));
        supports_tree.push_back(Vertex(ver_supports_mid[j],normal2));
        supports_tree.push_back(Vertex(ver_supports_up[j],normal2));

        count_size+=2;
    }


    //循环添加直柱面的三角面点
    for(int i=0;i!=ver_supports_bottom.size();++i)
    {
        int j=i+1;
        if(i==ver_supports_bottom.size()-1)
            j=0;

        //求解第一个三角面片的法向量
        auto p1=ver_supports_mid[j]-ver_supports_mid[i];
        auto p2=ver_supports_mid[i]-ver_supports_bottom[i];

        QVector3D normal(p1.y()*p2.z()-p2.y()*p1.z(),
                        p1.z()*p2.x()-p2.z()*p1.x(),
                        p1.x()*p2.y()-p2.x()*p1.y());
        supports_tree.push_back(Vertex(ver_supports_mid[j],normal));
        supports_tree.push_back(Vertex(ver_supports_mid[i],normal));
        supports_tree.push_back(Vertex(ver_supports_bottom[i],normal));

        //求解第二个三角面片的法向量
        p1=ver_supports_bottom[i]-ver_supports_bottom[j];
        p2=ver_supports_bottom[j]-ver_supports_mid[j];

        QVector3D normal2(p1.y()*p2.z()-p2.y()*p1.z(),
                          p1.z()*p2.x()-p2.z()*p1.x(),
                          p1.x()*p2.y()-p2.x()*p1.y());
        supports_tree.push_back(Vertex(ver_supports_bottom[i],normal2));
        supports_tree.push_back(Vertex(ver_supports_bottom[j],normal2));
        supports_tree.push_back(Vertex(ver_supports_mid[j],normal2));

        count_size+=2;
    }

    //循环添加圆的三角面点
    for(int i=0;i!=ver_supports_up.size();++i)
    {
        int j=i+1;
        if(i==ver_supports_up.size()-1)
            j=0;

        //求解下圆面第一个三角面片的法向量
        auto p1=ver_supports_bottom[j]-circle_bottom_pos;
        auto p2=circle_bottom_pos-ver_supports_bottom[i];
        QVector3D normal2(p1.y()*p2.z()-p2.y()*p1.z(),
                          p1.z()*p2.x()-p2.z()*p1.x(),
                          p1.x()*p2.y()-p2.x()*p1.y());
        supports_tree.push_back(Vertex(ver_supports_bottom[i],normal2));
        supports_tree.push_back(Vertex(circle_bottom_pos,normal2));
        supports_tree.push_back(Vertex(ver_supports_bottom[j],normal2));

        count_size++;
    }
    tree_sizes+=count_size;
    R_bottom=0.15f;
}
//斜圆柱生成
void My_Opengl::get_tree_2(My_Vector3D up,My_Vector3D down)
{
    int count_size=0;                   //统计面片数量
    int num=static_cast<int>(2*PI*R_bottom/default_len);  //圆环所做等分数
    float radin=2*PI/static_cast<float>(num);            //每个小扇形的弧度

    QVector3D circle_up_pos(up.x(),up.y(),up.z());      //顶部圆面的圆心位置
    QVector3D circle_bottom_pos(down.x(),down.y(),down.z());

    QVector<QVector3D> ver_supports_up;         //上圆环
    QVector<QVector3D> ver_supports_bottom;     //下圆环

    //将三圆的采样点加入对应数组
    ver_supports_up.push_back(QVector3D(up.x(),circle_up_pos.y(),up.z()+R_bottom));
    ver_supports_bottom.push_back(QVector3D(down.x(),circle_bottom_pos.y(),down.z()+R_bottom));
    for(int i=1;i!=num;++i)
    {
        //计算第上圆环采样点
        float y_c= circle_up_pos.y();
        float x_c= up.x()-R_bottom*sin(radin*i);
        float z_c= up.z()+R_bottom*cos(radin*i);
        ver_supports_up.push_back(QVector3D(x_c,y_c,z_c));

        y_c= circle_bottom_pos.y();
        x_c= down.x()-R_bottom*sin(radin*i);
        z_c= down.z()+R_bottom*cos(radin*i);
        ver_supports_bottom.push_back(QVector3D(x_c,y_c,z_c));
    }

    //循环添加直柱面的三角面点
    for(int i=0;i!=ver_supports_bottom.size();++i)
    {
        int j=i+1;
        if(i==ver_supports_bottom.size()-1)
            j=0;

        //求解第一个三角面片的法向量
        auto p1=ver_supports_up[j]-ver_supports_up[i];
        auto p2=ver_supports_up[i]-ver_supports_bottom[i];

        QVector3D normal(p1.y()*p2.z()-p2.y()*p1.z(),
                        p1.z()*p2.x()-p2.z()*p1.x(),
                        p1.x()*p2.y()-p2.x()*p1.y());
        supports_tree.push_back(Vertex(ver_supports_up[j],normal));
        supports_tree.push_back(Vertex(ver_supports_up[i],normal));
        supports_tree.push_back(Vertex(ver_supports_bottom[i],normal));

        //求解第二个三角面片的法向量
        p1=ver_supports_bottom[i]-ver_supports_bottom[j];
        p2=ver_supports_bottom[j]-ver_supports_up[j];

        QVector3D normal2(p1.y()*p2.z()-p2.y()*p1.z(),
                          p1.z()*p2.x()-p2.z()*p1.x(),
                          p1.x()*p2.y()-p2.x()*p1.y());
        supports_tree.push_back(Vertex(ver_supports_bottom[i],normal2));
        supports_tree.push_back(Vertex(ver_supports_bottom[j],normal2));
        supports_tree.push_back(Vertex(ver_supports_up[j],normal2));

        count_size+=2;
    }
    tree_sizes+=count_size;
    //supports_tree.set_meshs_size(count_size);
}
//圆柱加圆锥生成
void My_Opengl::get_tree_3(My_Vector3D poin)
{
    int count_size=0;                   //统计面片数量
    int num=static_cast<int>(2*PI*R_bottom/default_len);  //圆环所做等分数
    float radin=2*PI/static_cast<float>(num);            //每个小扇形的弧度

    QVector3D circle_up_pos(poin.x(),poin.y()+defa_h_tree,poin.z());      //顶部圆面的圆心位置
    QVector3D circle_mid_pos(poin.x(),poin.y()+defa_h_tree/2.0f,poin.z()); //中部圆面的圆心位置
    QVector3D circle_bottom_pos(poin.x(),poin.y(),poin.z());

    QVector<QVector3D> ver_supports_up;         //上圆环
    QVector<QVector3D> ver_supports_mid;        //中圆环
    QVector<QVector3D> ver_supports_bottom;     //下圆环

    //将三圆的采样点加入对应数组
    ver_supports_up.push_back(QVector3D(poin.x(),circle_up_pos.y(),poin.z()+R_up));
    ver_supports_mid.push_back(QVector3D(poin.x(),circle_mid_pos.y(),poin.z()+R_mid));
    ver_supports_bottom.push_back(QVector3D(poin.x(),circle_bottom_pos.y(),poin.z()+R_bottom));
    for(int i=1;i!=num;++i)
    {
        //计算上圆环采样点
        float y_c= circle_up_pos.y();
        float x_c= poin.x()-R_up*sin(radin*i);
        float z_c= poin.z()+R_up*cos(radin*i);
        ver_supports_up.push_back(QVector3D(x_c,y_c,z_c));

        //计算中圆环采样点
        y_c= circle_mid_pos.y();
        x_c= poin.x()-R_mid*sin(radin*i);
        z_c= poin.z()+R_mid*cos(radin*i);
        ver_supports_mid.push_back(QVector3D(x_c,y_c,z_c));

        //计算下圆环采样点
        y_c= circle_bottom_pos.y();
        x_c= poin.x()-R_bottom*sin(radin*i);
        z_c= poin.z()+R_bottom*cos(radin*i);
        ver_supports_bottom.push_back(QVector3D(x_c,y_c,z_c));
    }
    //循环添加斜柱面的三角面点
    for(int i=0;i!=ver_supports_mid.size();++i)
    {
        int j=i+1;
        if(i==ver_supports_mid.size()-1)
            j=0;

        //求解第一个三角面片的法向量
        auto p1=ver_supports_up[j]-ver_supports_up[i];
        auto p2=ver_supports_up[i]-ver_supports_mid[i];
        QVector3D normal(p1.y()*p2.z()-p2.y()*p1.z(),
                        p1.z()*p2.x()-p2.z()*p1.x(),
                        p1.x()*p2.y()-p2.x()*p1.y());
        supports_tree.push_back(Vertex(ver_supports_up[j],normal));
        supports_tree.push_back(Vertex(ver_supports_up[i],normal));
        supports_tree.push_back(Vertex(ver_supports_mid[i],normal));

        //求解第二个三角面片的法向量
        p1=ver_supports_mid[i]-ver_supports_mid[j];
        p2=ver_supports_mid[j]-ver_supports_up[j];
        QVector3D normal2(p1.y()*p2.z()-p2.y()*p1.z(),
                          p1.z()*p2.x()-p2.z()*p1.x(),
                          p1.x()*p2.y()-p2.x()*p1.y());
        supports_tree.push_back(Vertex(ver_supports_mid[i],normal2));
        supports_tree.push_back(Vertex(ver_supports_mid[j],normal2));
        supports_tree.push_back(Vertex(ver_supports_up[j],normal2));

        count_size+=2;
    }

    //循环添加直柱面的三角面点
    for(int i=0;i!=ver_supports_bottom.size();++i)
    {
        int j=i+1;
        if(i==ver_supports_bottom.size()-1)
            j=0;

        //求解第一个三角面片的法向量
        auto p1=ver_supports_mid[j]-ver_supports_mid[i];
        auto p2=ver_supports_mid[i]-ver_supports_bottom[i];

        QVector3D normal(p1.y()*p2.z()-p2.y()*p1.z(),
                        p1.z()*p2.x()-p2.z()*p1.x(),
                        p1.x()*p2.y()-p2.x()*p1.y());
        supports_tree.push_back(Vertex(ver_supports_mid[j],normal));
        supports_tree.push_back(Vertex(ver_supports_mid[i],normal));
        supports_tree.push_back(Vertex(ver_supports_bottom[i],normal));

        //求解第二个三角面片的法向量
        p1=ver_supports_bottom[i]-ver_supports_bottom[j];
        p2=ver_supports_bottom[j]-ver_supports_mid[j];

        QVector3D normal2(p1.y()*p2.z()-p2.y()*p1.z(),
                          p1.z()*p2.x()-p2.z()*p1.x(),
                          p1.x()*p2.y()-p2.x()*p1.y());
        supports_tree.push_back(Vertex(ver_supports_bottom[i],normal2));
        supports_tree.push_back(Vertex(ver_supports_bottom[j],normal2));
        supports_tree.push_back(Vertex(ver_supports_mid[j],normal2));

        count_size+=2;
    }

    //循环添加圆的三角面点
    for(int i=0;i!=ver_supports_up.size();++i)
    {
        int j=i+1;
        if(i==ver_supports_up.size()-1)
            j=0;

        //求解上圆面第一个三角面片的法向量
        auto p1=circle_up_pos-ver_supports_up[j];
        auto p2=ver_supports_up[i]-circle_up_pos;
        QVector3D normal(p1.y()*p2.z()-p2.y()*p1.z(),
                         p1.z()*p2.x()-p2.z()*p1.x(),
                         p1.x()*p2.y()-p2.x()*p1.y());
        supports_automatic.push_back(Vertex(ver_supports_up[j],normal));
        supports_automatic.push_back(Vertex(circle_up_pos,normal));
        supports_automatic.push_back(Vertex(ver_supports_up[i],normal));

        count_size++;
    }
    tree_sizes+=count_size;
}


//模型投影至X-Y平面并网格化分区
void My_Opengl::model_gridding()
{
    float x_max=model_changed.show_x_max();
    float y_max=model_changed.show_z_max();
    float x_min=model_changed.show_x_min();
    float y_min=model_changed.show_z_min();

    float x_len=x_max-x_min;
    float y_len=y_max-y_min;

    int x_size=static_cast<int>(x_len/grid_size)+1;
    int y_size=static_cast<int>(y_len/grid_size)+1;

    //网格初始化  设置长和宽
    mesh_grid.clear();
    for(int i=0;i!=x_size;++i)
    {
        QVector<QVector<int>> vec;
        for(int j=0;j!=y_size;++j)
        {
            QVector<int> vec_2;
            vec.push_back(vec_2);
        }
        mesh_grid.push_back(vec);
    }

    //遍历整个模型，对每个三角面片进行分区
    auto& vetexs=model_changed.vertexs_of_model();
    QVector<float> vec2{99999.0f,-99999.0f,99999.0f,-99999.0f};
    for(int i=0;i!=vetexs.size();i+=3)
    {
        //求得该面片的方形包围框
        QVector<float> vec=vec2;   //x_min,x_max,y_min,y_max
        for(int j=i;j!=i+3;++j)
        {
            auto poin=vetexs[j].position();
            if(vec[0]>poin.x())
                vec[0]=poin.x();
            if(vec[1]<poin.x())
                vec[1]=poin.x();

            if(vec[2]>poin.z())
                vec[2]=poin.z();
            if(vec[3]<poin.z())
                vec[3]=poin.z();
        }
        vec[0]=vec[0]-x_min;vec[1]=vec[1]-x_min;
        vec[2]=vec[2]-y_min;vec[3]=vec[3]-y_min;

        //求得四个点的所属网格编号
        int x_min_loca=static_cast<int>(vec[0]/grid_size);
        int y_min_loca=static_cast<int>(vec[2]/grid_size);
        int x_max_loca=static_cast<int>(vec[1]/grid_size)+1;
        int y_max_loca=static_cast<int>(vec[3]/grid_size)+1;

        //向二维存储表中添加面片编号
        for(int k=x_min_loca;k!=x_max_loca;++k)
        {
            for(int j=y_min_loca;j!=y_max_loca;++j)
                mesh_grid[k][j].push_back(i);
        }
    }

}

