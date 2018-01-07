#ifndef U_YUVMANAGER_H
#define U_YUVMANAGER_H

#include <QMutex>
#include <list>
#include <QDateTime>
#include <QImage>


using namespace std;

//实现单例
#define IMPL_SINGLE(ClassName) \
    ClassName *ClassName::Instance = NULL; \
    QMutex ClassName::SingleMutex_t; \
    ClassName *ClassName::GetInstance(){ \
        if (ClassName::Instance == NULL){ \
            ClassName::SingleMutex_t.lock(); \
            if (ClassName::Instance == NULL) { \
                ClassName::Instance = new ClassName(); \
            } \
            ClassName::SingleMutex_t.unlock(); \
        } \
        return ClassName::Instance; \
    } \
    void ClassName::Destroy(){} \
//        if (ClassName::Instance != NULL){ \
//            ClassName::SingleMutex_t.lock(); \
//            if (ClassName::Instance != NULL) { \
//                delete ClassName::Instance; \
//                ClassName::Instance = NULL; \
//            } \
//            ClassName::SingleMutex_t.unlock(); \
//        } \
//    }

//创建单例
#define CREATE_SINGLE(ClassName) \
    private : \
    ClassName(const ClassName&){} \
    ClassName& operator=(const ClassName&){} \
    static ClassName *Instance; \
    static QMutex SingleMutex_t; \
    public : \
    static ClassName *GetInstance(); \
    static void Destroy();

class Yuv
{
public:
    Yuv()
        : YData_pc1(NULL)
        , UData_pc1(NULL)
        , VData_pc1(NULL)
        , YSize_n4(-1)
        , USize_n4(-1)
        , VSize_n4(-1)
        , Width_n4(-1)
        , Height_n4(-1)
        , Id_n4(-1)
        , IsNeedUpdate_b1(false)
        , ShowWidth_n4(-1)
        , ShowHeight_n4(-1)
        , ShowX_n4(0)
        , ShowY_n4(0)
        , FixedX_n4(-1)
        , FixedY_n4(-1)
        , FixedWidth_n4(-1)
        , FixedHeight_n4(-1)
        , IsProcessing_b1(false)
        , UseNumber_n4(1)
        , IsWaitDelete_b1(false)
        , TargetYuv(NULL)
        , CurrentRow_n4(0)
        , CurrentColumn_n4(0)
        , LineDataCount_n4(0)
    {
    }

public:
    unsigned char *YData_pc1;
    int YSize_n4;
    unsigned char *UData_pc1;
    int USize_n4;
    unsigned char *VData_pc1;
    int VSize_n4;

    int Width_n4; //源数据宽度
    int Height_n4; //源数据高度
    //用户设置的x,y坐标,默认为-1（ 自动排列）
    int FixedX_n4;
    int FixedY_n4;
    //用户设置的宽高度，默认为-1(根据自动排列，自动计算)
    int FixedWidth_n4;
    int FixedHeight_n4;


    int ShowWidth_n4; //在屏幕中显示的宽度
    int ShowHeight_n4; //在屏幕中显示的宽度
    int ShowX_n4; //在屏幕中显示的X坐标
    int ShowY_n4; //在屏幕中显示的Y坐标

    //当前窗口的id
    int Id_n4;

    //是否需要更新YUV数据
    bool IsNeedUpdate_b1;

    /** 是否正在处理中 */
    bool IsProcessing_b1;
    /** 使用计数器，当为0时，则删除此对象 */
    int UseNumber_n4;
    /** 是否等待删除 */
    bool IsWaitDelete_b1;
    /** 目录yuv指针 */
    Yuv *TargetYuv;
    /** 当前窗口所在的行数 */
    int CurrentRow_n4;
    /** 当前窗口所在的行数 */
    int CurrentColumn_n4;
    /** 每一大行窗口数据的总量 */
    int LineDataCount_n4;

    QMutex Mutex;
};

/** 显示排列方式 */
enum Arrangement{
    //顺序排列， 根据创建顺序依次排列,根据总数量计算最优排列方式，如4个，排列为2*2,5个排列为2*2(其中有一个小屏幕显示两个)
    //6个排列为 2*3或3*2，具体根据视屏分辨率来计算
    SEQUENCE
};

#include <QThread>

/** yuv构建线程 */

class YuvBuilderThread : public QThread{
public :
    YuvBuilderThread()
        : IsRunning_b1(true)
    {
        start();
    }
    ~YuvBuilderThread(){
        IsRunning_b1 = false;
    }
protected:
    void run();
private :
    bool IsRunning_b1;
};

class YuvManager : public QThread
{
public:
    static YuvManager *GetInstance(){
        static YuvManager Instance;
        return &Instance;
    }
public:
    /** 设置目标显示大小 */
    void SetTargetSize(int TargetWidth_n4i,int TargetHeight_n4i);
    /** 设置构建线程数量，默认为自动 */
    void SetBuilderThread(int);
    /** 设置分隔线粗细,argb */
    void SetSplitLineWidth(int);
    /** 设置分隔线颜色,//必须是2的倍数 */
    void SetSplitLineColor(int);
    /** 创建一个yuv对象 */
    Yuv *CreateYuv();
    /** 更新yuv数据 */
    void UpdateYuv(Yuv *);
    /** 删除一个Yuv对象 */
    void DeleteYuv(Yuv *);
    /** 设置组装数据的fps */
    void SetFps(int);
    /** 获取显示的yuv对象 */
    Yuv* GetShowYuv();
    /** 通过x,y坐标查找对应的yuv对象 */
    Yuv* FindByPoint(int,int);
    /** 设置构建线程数量,默认为1 */
    void SetBuildThreadNumber(int BuilderThreadNumber_n4i = 1);
protected :
    void run();

    /** 取到下一个窗口对象 */
    friend class YuvBuilderThread;
    Yuv *NextWindow();
    void ReallyRemoveYuv(Yuv*);
private :
    /** 计算输出的yuv数据 */
    void CalcOutputYuv();
    Yuv* ImageToYuv(const QImage &);
private :
    YuvManager();
    ~YuvManager();
    YuvManager(const YuvManager&){}
    YuvManager& operator=(const YuvManager&){}

    /** 计算矩阵排列 */
    void CalcMatrix();
private :
    QMutex Mutex;
    list<Yuv *> Yuvs;
    /** 当前处理列表的下标 */
    list<Yuv *>::iterator Index_i;
    Yuv *LogoYuv;

    //输入的宽度
    int TargetWidth_n4;
    //输入的高度
    int TargetHeight_n4;

    Yuv *TargetYuv;
    /** 显示的行数 */
    int ShowRow_n4;
    /** 显示示的列数 */
    int ShowColumn_n4;
    /** 线程是否运行中 */
    bool IsRunning_b1;
    bool IsRunningComplete_b1;
    int Fps_n4;

    /** yuv构建线程 */
    list<YuvBuilderThread *> BuilderThrad_v;
    /** 分隔线颜色 */
    int SplitColor_n4;
    /** 分隔线粗细 */
    int SplitWidth_n4;
    /** 构建线程数量 */
    int BuilderThreadNumber_n4;
//CREATE_SINGLE(YuvManager) //创建单例
};
//IMPL_SINGLE(YuvManager) //实现单例

#endif // U_YUVMANAGER_H
