#include "U_YuvManager.h"
#include <QImage>
#include <QDebug>

YuvManager::YuvManager()
    : TargetWidth_n4(NULL)
    , TargetHeight_n4(NULL)
    , ShowRow_n4(1)
    , ShowColumn_n4(1)
    , IsRunning_b1(true)
    , IsRunningComplete_b1(false)
    , Fps_n4(25)
    , TargetYuv(new Yuv)
    , SplitColor_n4(0xFF3299CC)
    , SplitWidth_n4(4)
    , LogoYuv(NULL)
    , BuilderThreadNumber_n4(1)
    , Index_i(Yuvs.begin())
{
    QImage icon(":/logo.png");
    if(!icon.isNull()){
        LogoYuv = ImageToYuv(icon);
    }

    SetTargetSize(1920,1080); //默认窗口大小
    SetSplitLineWidth(SplitWidth_n4);
}

YuvManager::~YuvManager(){
    IsRunning_b1 = false;
    if(TargetYuv != NULL){
        TargetYuv->Mutex.lock();
        if(TargetYuv->YData_pc1 != NULL)
            delete TargetYuv->YData_pc1;
        if(TargetYuv->UData_pc1 != NULL)
            delete TargetYuv->UData_pc1;
        if(TargetYuv->VData_pc1 != NULL)
            delete TargetYuv->VData_pc1;
        TargetYuv->Mutex.unlock();
        delete TargetYuv;
    }
    while(!IsRunningComplete_b1); //等待执行完成
}

/** 设置目标显示大小 */
void YuvManager::SetTargetSize(int TargetWidth_n4i,int TargetHeight_n4i){
    //8k
//    TargetWidth_n4i = 4096 * 2;
//    TargetHeight_n4i = 2160 * 2;
    //4k
//    TargetWidth_n4i = 4096;
//    TargetHeight_n4i = 2160;
    //1080P
//    TargetWidth_n4i = 1920;
//    TargetHeight_n4i = 1080;
    Mutex.lock();
    //此步骤不能省略，为了规范yuv数据分辨率
    TargetWidth_n4 = TargetWidth_n4i / 8 * 8;
    TargetHeight_n4 = TargetHeight_n4i / 8 * 8;

    qDebug() << TargetWidth_n4 <<":"<<TargetHeight_n4;
    //end 此步骤不能省略，为了规范yuv数据分辨率

    if(TargetYuv != NULL){
        TargetYuv->Mutex.lock();
        TargetYuv->IsWaitDelete_b1 = true;
        unsigned char *YTempData_pc1 = TargetYuv->YData_pc1;
        unsigned char *UTempData_pc1 = TargetYuv->UData_pc1;
        unsigned char *VTempData_pc1 = TargetYuv->VData_pc1;

        TargetYuv->Width_n4 = TargetWidth_n4;
        TargetYuv->Height_n4 = TargetHeight_n4;
        TargetYuv->ShowX_n4 = (TargetWidth_n4i - TargetWidth_n4) >> 1;
        TargetYuv->ShowY_n4 = (TargetHeight_n4i - TargetHeight_n4) >> 1;
        TargetYuv->ShowWidth_n4 = TargetWidth_n4;
        TargetYuv->ShowHeight_n4 = TargetHeight_n4;

        TargetYuv->YSize_n4 = TargetWidth_n4 * TargetHeight_n4;
        TargetYuv->YData_pc1 = new unsigned char[TargetYuv->YSize_n4];

        TargetYuv->USize_n4 = TargetWidth_n4 * TargetHeight_n4 / 4;
        TargetYuv->UData_pc1 = new unsigned char[TargetYuv->USize_n4];

        TargetYuv->VSize_n4 = TargetWidth_n4 * TargetHeight_n4 / 4;
        TargetYuv->VData_pc1 = new unsigned char[TargetYuv->VSize_n4];


        if(YTempData_pc1 != NULL)
            delete YTempData_pc1;
        if(UTempData_pc1 != NULL)
            delete UTempData_pc1;
        if(VTempData_pc1 != NULL)
            delete VTempData_pc1;

        SetSplitLineColor(SplitColor_n4);
        CalcMatrix();
        TargetYuv->Mutex.unlock();
    }
    Mutex.unlock();
}

Yuv* YuvManager::ImageToYuv(const QImage &icon){
    if(!icon.isNull()){
        Yuv *yuv = new Yuv;
        yuv->Width_n4 = icon.width() / 2 * 2;
        yuv->Height_n4 = icon.height() / 2 * 2;
        yuv->YSize_n4 = yuv->Width_n4 * yuv->Height_n4;
        yuv->YData_pc1 = new unsigned char[yuv->YSize_n4];

        yuv->USize_n4 = yuv->Width_n4 * yuv->Height_n4 / 4;
        yuv->UData_pc1 = new unsigned char[yuv->USize_n4];

        yuv->VSize_n4 = yuv->Width_n4 * yuv->Height_n4 / 4;
        yuv->VData_pc1 = new unsigned char[yuv->VSize_n4];

        unsigned char *TempUData_pc1 = new unsigned char[yuv->YSize_n4];
        unsigned char *TempVData_pc1 = new unsigned char[yuv->YSize_n4];
        int Pos_n4 = 0;
        int UVPos_n4 = 0;
        for(int i = 0;i < yuv->Height_n4;++i){
            for(int j = 0;j < yuv->Width_n4;++j){
                QColor Color = icon.pixelColor(j,i);
                int r = Color.red();
                int g = Color.green();
                int b = Color.blue();
                int y = (unsigned char)( ( 66 * r + 129 * g +  25 * b + 128) >> 8) + 16  ;
                int u = (unsigned char)( ( -38 * r -  74 * g + 112 * b + 128) >> 8) + 128 ;
                int v = (unsigned char)( ( 112 * r -  94 * g -  18 * b + 128) >> 8) + 128 ;
                yuv->YData_pc1[Pos_n4] = max( 0, min(y, 255 ));
                TempUData_pc1[Pos_n4] = max( 0, min(u, 255 ));
                TempVData_pc1[Pos_n4] = max( 0, min(v, 255 ));
                Pos_n4++;
            }
        }

        unsigned char *cv;
        unsigned char *nv;
        unsigned char *cu;
        unsigned char *nu;
        for(int i = 0;i < yuv->Height_n4;i+=2){
            cv = &TempVData_pc1[i * yuv->Width_n4];
            nv = &TempVData_pc1[(i + 1) * yuv->Width_n4];

            cu = &TempUData_pc1[i * yuv->Width_n4];
            nu = &TempUData_pc1[(i + 1) * yuv->Width_n4];
            for(int j = 0;j < yuv->Width_n4;j+=2){
                yuv->UData_pc1[UVPos_n4] = (cu[j] + cu[j+1] + nu[j] + nu[j+1]) / 4;
                yuv->VData_pc1[UVPos_n4] = (cv[j] + cv[j+1] + nv[j] + nv[j+1]) / 4;
                UVPos_n4++;
            }
        }

        delete [] TempUData_pc1;
        delete [] TempVData_pc1;
        return yuv;
    }
    return NULL;
}

/** 设置分隔线颜色 */
void YuvManager::SetSplitLineColor(int Color_n4){
    if(TargetYuv != NULL){
        int Red_n4 = (Color_n4 >> 16) & 0xFF;
        int Green_n4 = (Color_n4 >> 8) & 0xFF;
        int Blue_n4 = (Color_n4) & 0xFF;

        // y分量
        int Y = (int)(( 66 * Red_n4 + 129 * Green_n4 +  25 * Blue_n4 + 128) >> 8) + 16;
        // u分量
        int U = (int)((-38 * Red_n4 -  74 * Green_n4 + 112 * Blue_n4 + 128) >> 8) + 128;
        // v分量
        int V = (int)((112 * Red_n4 -  94 * Green_n4 -  18 * Blue_n4 + 128) >> 8) + 128;

        bool locked = TargetYuv->Mutex.try_lock();
        memset(TargetYuv->YData_pc1 ,Y,TargetYuv->YSize_n4);
        memset(TargetYuv->UData_pc1 ,U,TargetYuv->USize_n4);
        memset(TargetYuv->VData_pc1 ,V,TargetYuv->VSize_n4);
        if(locked){
            TargetYuv->Mutex.unlock();
        }
    }
    SplitColor_n4 = Color_n4;
}

/** 设置分隔线粗细 */
void YuvManager::SetSplitLineWidth(int width){
    width = width / 2 * 2;//必须是2的倍数
    if(width < 0)
        SplitWidth_n4 = 0;
    SplitWidth_n4 = width;
}

/** 创建一个yuv对象 */
Yuv *YuvManager::CreateYuv(){
    Mutex.lock();
    static int Id_n4 = 1;
    Yuv *yuv = new Yuv;
    yuv->Id_n4 = Id_n4++;
    if(LogoYuv != NULL){
        yuv->Width_n4 = LogoYuv->Width_n4;
        yuv->Height_n4 = LogoYuv->Height_n4;
        yuv->YData_pc1 = LogoYuv->YData_pc1;
        yuv->UData_pc1 = LogoYuv->UData_pc1;
        yuv->VData_pc1 = LogoYuv->VData_pc1;
        yuv->YSize_n4 = LogoYuv->YSize_n4 / yuv->Height_n4;
        yuv->USize_n4 = LogoYuv->USize_n4 / yuv->Height_n4;
        yuv->VSize_n4 = LogoYuv->VSize_n4 / yuv->Height_n4;
        yuv->IsNeedUpdate_b1 = true;
    }
    Yuvs.push_back(yuv);
    CalcMatrix();
    Mutex.unlock();
    SetBuildThreadNumber(sqrt(Yuvs.size()));
    return yuv;
}

/** 更新yuv数据 */
void YuvManager::UpdateYuv(Yuv *yuv){
    yuv->IsNeedUpdate_b1 = true;//需要更新
}

/** 删除一个Yuv对象 */
void YuvManager::DeleteYuv(Yuv *yuv){
    Mutex.lock();
    list<Yuv *>::iterator Begin_i = Yuvs.begin();
    list<Yuv *>::iterator End_i = Yuvs.end();
    while(Begin_i != End_i){
        if(yuv == *Begin_i){
            yuv->IsWaitDelete_b1 = true;
            break;
        }
        ++Begin_i;
    }
    Mutex.unlock();
}

void YuvManager::ReallyRemoveYuv(Yuv* yuv){
    Mutex.lock();
    list<Yuv *>::iterator Begin_i = Yuvs.begin();
    list<Yuv *>::iterator End_i = Yuvs.end();
    while(Begin_i != End_i){
        if(yuv == *Begin_i){
            delete yuv;
            break;
        }
        ++Begin_i;
    }
    Mutex.unlock();
}

/** 获取拼接后的yuv数据 */
void YuvManager::CalcOutputYuv(){
//    Mutex.lock();
//    qint64 Begin_n8 = QDateTime::currentMSecsSinceEpoch();
//    list<Yuv *>::iterator Begin_i = Yuvs.begin();
//    list<Yuv *>::iterator End_i = Yuvs.end();
//    //每个块的宽度
//    int CellWidth_n4 = TargetWidth_n4 / ShowColumn_n4 / 2 * 2;
//    //每个块的高度
//    int CellHeight_n4 = TargetHeight_n4 / ShowRow_n4 / 2 * 2;
//    int CellWidthHalf_n4 = CellWidth_n4 >> 1;
//    int CellHeightHalf_n4 = CellHeight_n4 >> 1;
//    //一行数据的大小(以窗口为单位)
//    int LineDataCount_n4 = TargetWidth_n4 * CellHeight_n4;
//    int TargetWidthHalf_n4 = TargetWidth_n4 >> 1;

//    int Index_n4 = 0;
//    while(Begin_i != End_i){
//        Yuv *yuv = *Begin_i;
//        if(yuv != NULL){
////            if(yuv->UseNumber_n4 == 0 && yuv->IsWaitDelete_b1){//如果yuv没有被使用，并且等待删除，则删除当前yuv对象
////                if(yuv->Mutex.try_lock()){
////                    if(yuv->UseNumber_n4 == 0){//删除此对象
////                        Begin_i = Yuvs.erase(Begin_i);
////                        delete yuv;
////                        yuv->Mutex.unlock();
////                        continue;
////                    }
////                    yuv->Mutex.unlock();
////                }
////            }


////            int CurrentColumn = Index_n4 % ShowColumn_n4; //当前列数
////            int CurrentRow = Index_n4 / ShowColumn_n4; //当前行数

////            int RealityCellWidth_n4 = CellWidth_n4 - SplitWidth_n4;
////            int RealityCellHeight_n4 = CellHeight_n4 - SplitWidth_n4;

////            int RealityCellWidthHalf_n4 = CellWidthHalf_n4 - (SplitWidth_n4 >> 1);
////            int RealityCellHiehgtHalf_n4 = CellHeightHalf_n4 - (SplitWidth_n4 >> 1);

////            if(CurrentColumn == ShowColumn_n4 - 1){
////                RealityCellWidth_n4 = CellWidth_n4;
////                RealityCellWidthHalf_n4 = CellWidthHalf_n4;
////            }

////            if(CurrentRow == ShowRow_n4 - 1){
////                RealityCellHeight_n4 = CellHeight_n4;
////                RealityCellHiehgtHalf_n4 = CellHeightHalf_n4;
////            }

////            int TargetColumnCount_n4 = CurrentColumn * CellWidth_n4; //当前显示当窗口的数据起始位置
////            yuv->ShowWidth_n4 = RealityCellWidth_n4;
////            yuv->ShowHeight_n4 = RealityCellHeight_n4;
////            if(CurrentColumn > 0)
////                yuv->ShowX_n4 = TargetColumnCount_n4 + (SplitWidth_n4 >> 1);
////            else{
////                yuv->ShowX_n4 = 0;
////            }
////            if(CurrentRow > 0){
////                yuv->ShowY_n4 = CurrentRow * CellHeight_n4;
////            }else{
////                yuv->ShowY_n4 = 0;
////            }

////            if(BuilderThrad_v.size() >  0){
////                YuvBuilderThread *BuilderThread = BuilderThrad_v.front();
////                BuilderThread->AddTask(yuv,TargetYuv,CurrentRow,LineDataCount_n4);
////                BuilderThrad_v.pop_front();
////                BuilderThrad_v.push_back(BuilderThread);
////            }

//                /*
//            if(yuv->IsNeedUpdate_b1 && 0){ //尝试锁定，如果锁定失败,则有可能是其他线程在处理，则不处理
//                int CurrentColumn = Index_n4 % ShowColumn_n4; //当前列数
//                int CurrentRow = Index_n4 / ShowColumn_n4; //当前行数

//                int RealityCellWidth_n4 = CellWidth_n4 - SplitWidth_n4;
//                int RealityCellHeight_n4 = CellHeight_n4 - SplitWidth_n4;

//                int RealityCellWidthHalf_n4 = CellWidthHalf_n4 - (SplitWidth_n4 >> 1);
//                int RealityCellHiehgtHalf_n4 = CellHeightHalf_n4 - (SplitWidth_n4 >> 1);

//                if(CurrentColumn == ShowColumn_n4 - 1){
//                    RealityCellWidth_n4 = CellWidth_n4;
//                    RealityCellWidthHalf_n4 = CellWidthHalf_n4;
//                }

//                if(CurrentRow == ShowRow_n4 - 1){
//                    RealityCellHeight_n4 = CellHeight_n4;
//                    RealityCellHiehgtHalf_n4 = CellHeightHalf_n4;
//                }

//                //乘以1000000，转换整型是为了提高运算效率，保留浮点数6位精度，运算完后再除以1000000
//                //1048576相当于<<20
//                //做除法的时候>>20，以提高效率
//                int WidthRate_f4 = yuv->Width_n4 * 1.0 / RealityCellWidth_n4 * 1048576;
//                int HeightRate_f4 = yuv->Height_n4 * 1.0 / RealityCellHeight_n4 * 1048576;

//                int TargetColumnCount_n4 = CurrentColumn * CellWidth_n4; //当前显示当窗口的数据起始位置
//                int TargetColumnCountHalf_n4 = TargetColumnCount_n4 >> 1; //当前显示当窗口的数据起始位置
//                int CurrentRowLineDataCount_n4 = CurrentRow * LineDataCount_n4;//己处理的行数据大小
//                int CurrentRowLineDataCountQuarter_n4 = CurrentRow * LineDataCount_n4 >> 2;//己处理的行数据大小

//                int YCurrentDataCount_n4 = TargetColumnCount_n4 + CurrentRowLineDataCount_n4;
//                int UVCurrentDataCount_n4 = TargetColumnCountHalf_n4 + CurrentRowLineDataCountQuarter_n4;
//                int YRowPos_n4 = YCurrentDataCount_n4 - TargetWidth_n4;
//                int UVRowPos_n4 = UVCurrentDataCount_n4 - TargetWidthHalf_n4;
//                int YStartPos_n4 = -HeightRate_f4;
//                int UVTargetPos_n4 = 0;
//                int UVSourcePos_n4 = 0;
//                int YBasePos_n4 = 0;
//                int UVBasePos_n4 = 0;
//                bool IsUpdateUVData_b1 = false;
//                int YStart_n4 = 0;
//                int UVStart_n4 = 0;
//                int i = 0,j = 0;
//                int XStartPos_n4 = 0;


//                //为了减少计算，事先生成一个下标索引列表
//                int Indexs [RealityCellWidth_n4];
//                XStartPos_n4 = -WidthRate_f4;
//                for(i = 0;i < RealityCellWidth_n4;i++){
//                    XStartPos_n4 += WidthRate_f4;
//                    UVStart_n4 = XStartPos_n4 >> 20;
//                    Indexs[i] = UVStart_n4;
//                }
//                //end 为了减少计算，事先生成一个下标索引列表

//                for(i = 0;i < RealityCellHeight_n4;i++){ //为了减少循环次数，特将yuv放在一个循环里面处理
//                    YRowPos_n4 += TargetWidth_n4;
//                    YStartPos_n4 += HeightRate_f4;
//                    YStart_n4 = YStartPos_n4 >> 20;
//                    YBasePos_n4 = YStart_n4 * yuv->YSize_n4;
//                    IsUpdateUVData_b1 = i < RealityCellHiehgtHalf_n4;
//                    if(IsUpdateUVData_b1){ //为了减少循环次数，特将yuv放在一个循环里面处理,固多了此判断
//                        UVRowPos_n4 += TargetWidthHalf_n4;
//                        UVBasePos_n4 = YStart_n4 * yuv->USize_n4;
//                    }

//                    for(j = 0;j < RealityCellWidth_n4;j++){
//                        UVStart_n4 = Indexs[j];
//                        TargetYuv->YData_pc1[YRowPos_n4 + j] = yuv->YData_pc1[YBasePos_n4 + UVStart_n4];
//                        if(IsUpdateUVData_b1 && j < RealityCellWidthHalf_n4){ //为了减少循环次数，特将yuv放在一个循环里面处理,固多了此判断
//                            UVTargetPos_n4 = UVRowPos_n4 + j;
//                            UVSourcePos_n4 = UVBasePos_n4 + UVStart_n4;
//                            TargetYuv->UData_pc1[UVTargetPos_n4] = yuv->UData_pc1[UVSourcePos_n4];
//                            TargetYuv->VData_pc1[UVTargetPos_n4] = yuv->VData_pc1[UVSourcePos_n4];
//                        }
//                    }
//                }
//                yuv->IsNeedUpdate_b1 = false;
////                yuv->Mutex.unlock();
//            }*/
//        }
//        ++Begin_i;
//        ++Index_n4;
//    }
//    qint64 End_n8 = QDateTime::currentMSecsSinceEpoch();
//    int ExecuteTime_n4 = End_n8 - Begin_n8;
////    qDebug() << "ExecuteTime_n4 : " << ExecuteTime_n4;
//    Mutex.unlock();
}

/** 计算矩阵排列 */
void YuvManager::CalcMatrix(){
    bool Locked = Mutex.try_lock();
    int Size_n4 = Yuvs.size();
    if(Size_n4 == 0)
        return;

    int Base = sqrt(Size_n4);
    int Base2 = Size_n4 / Base;
    int More = Size_n4 % Base;
    ShowColumn_n4 = Base > Base2 ? Base2 : Base;
    ShowRow_n4 = Base > Base2 ? Base : Base2;
    if(More == 0){//表示可以整除，显示整行整列，没有多余

    }else{
        ShowRow_n4++;
    }

    list<Yuv *>::iterator Begin_i = Yuvs.begin();
    list<Yuv *>::iterator End_i = Yuvs.end();
    //每个块的宽度
    int CellWidth_n4 = TargetWidth_n4 / ShowColumn_n4 / 4 * 4;
    //每个块的高度
    int CellHeight_n4 = TargetHeight_n4 / ShowRow_n4 / 4 * 4;
    int CellWidthHalf_n4 = CellWidth_n4 >> 1;
    int CellHeightHalf_n4 = CellHeight_n4 >> 1;
    //一行数据的大小(以窗口为单位)
    int LineDataCount_n4 = TargetWidth_n4 * CellHeight_n4;
    int TargetWidthHalf_n4 = TargetWidth_n4 >> 1;

    int Index_n4 = 0;

    while(Begin_i != End_i){
        Yuv *yuv = *Begin_i;
        if(yuv != NULL){
            int CurrentColumn = Index_n4 % ShowColumn_n4; //当前列数
            int CurrentRow = Index_n4 / ShowColumn_n4; //当前行数

            int RealityCellWidth_n4 = CellWidth_n4 - SplitWidth_n4;
            int RealityCellHeight_n4 = CellHeight_n4 - SplitWidth_n4;

            int RealityCellWidthHalf_n4 = CellWidthHalf_n4 - (SplitWidth_n4 >> 1);
            int RealityCellHiehgtHalf_n4 = CellHeightHalf_n4 - (SplitWidth_n4 >> 1);

            if(CurrentColumn == ShowColumn_n4 - 1){
                RealityCellWidth_n4 = CellWidth_n4;
                RealityCellWidthHalf_n4 = CellWidthHalf_n4;
            }

            if(CurrentRow == ShowRow_n4 - 1){
                RealityCellHeight_n4 = CellHeight_n4;
                RealityCellHiehgtHalf_n4 = CellHeightHalf_n4;
            }

            int TargetColumnCount_n4 = CurrentColumn * CellWidth_n4; //当前显示当窗口的数据起始位置
            yuv->ShowWidth_n4 = RealityCellWidth_n4;
            yuv->ShowHeight_n4 = RealityCellHeight_n4;
            if(CurrentColumn > 0)
                yuv->ShowX_n4 = TargetColumnCount_n4 + (SplitWidth_n4 >> 1);
            else{
                yuv->ShowX_n4 = 0;
            }
            if(CurrentRow > 0){
                yuv->ShowY_n4 = CurrentRow * CellHeight_n4;
            }else{
                yuv->ShowY_n4 = 0;
            }
            yuv->CurrentRow_n4 = CurrentRow;
            yuv->CurrentColumn_n4 = CurrentColumn;
            yuv->LineDataCount_n4 = LineDataCount_n4;
            ++Index_n4;
        }
        ++Begin_i;
    }

    if(Locked){
        Mutex.unlock();
    }
}

/** 设置组装数据的fps */
void YuvManager::SetFps(int Fps){
    if(Fps > 0){
        Fps_n4 = Fps;
    }
}

/** 获取显示的yuv对象 */
Yuv* YuvManager::GetShowYuv(){
    return TargetYuv;
}

/** 通过x,y坐标查找对应的yuv对象 */
Yuv* YuvManager::FindByPoint(int x,int y){
    Mutex.lock();
    list<Yuv *>::iterator Begin_i = Yuvs.begin();
    list<Yuv *>::iterator End_i = Yuvs.end();
    while(Begin_i != End_i){
        Yuv *yuv = *Begin_i;
        if (x > yuv->ShowX_n4 && y > yuv->ShowY_n4 && x < (yuv->ShowX_n4 + yuv->ShowWidth_n4) && y > yuv->ShowY_n4 && x < (yuv->ShowX_n4 + yuv->ShowWidth_n4) && y < (yuv->ShowY_n4 + yuv->ShowHeight_n4) && x > yuv->ShowX_n4 && y < (yuv->ShowY_n4 + yuv->ShowHeight_n4)){
            Mutex.unlock();
            return yuv;
        }
        ++Begin_i;
    }
    Mutex.unlock();
    return NULL;
}

/** 设置构建线程数量,默认为1 */
void YuvManager::SetBuildThreadNumber(int BuilderThreadNumber_n4i){
    BuilderThreadNumber_n4i = 16;
    if(BuilderThreadNumber_n4i > 0){
        for(int i = BuilderThreadNumber_n4;i < BuilderThreadNumber_n4i;i++){
            BuilderThrad_v.push_back(new YuvBuilderThread()); //新增线程
        }
        BuilderThreadNumber_n4 = BuilderThreadNumber_n4i;
    }else if(BuilderThreadNumber_n4i < BuilderThreadNumber_n4 && BuilderThreadNumber_n4 > 0){
        for(int i = BuilderThreadNumber_n4i; i < BuilderThreadNumber_n4;i++){
            YuvBuilderThread *Thread = BuilderThrad_v.back();
            delete Thread;
            BuilderThrad_v.pop_back();
        }
    }
}

/** 取到下一个窗口对象 */
Yuv *YuvManager::NextWindow(){
    Yuv *yuv = NULL;
    Mutex.lock();
    if(Yuvs.size() > 0){
        if(Index_i == Yuvs.end()){
            Index_i = Yuvs.begin();
        }
        while(Index_i != Yuvs.end()){
            Yuv *TempYuv = *Index_i;
            if(TempYuv != NULL && TempYuv->IsNeedUpdate_b1){
                yuv = TempYuv;
                yuv->TargetYuv = TargetYuv;
                yuv->IsNeedUpdate_b1 = false;
                break;
            }
            ++Index_i;
        }
    }
    Mutex.unlock();
    return yuv;
}

void YuvManager::run(){
    while(IsRunning_b1){
        IsRunningComplete_b1 = false;
        int Interval_n4 = 1000 / Fps_n4 ;

        qint64 Begin_n8 = QDateTime::currentMSecsSinceEpoch();
        CalcOutputYuv();
        qint64 End_n8 = QDateTime::currentMSecsSinceEpoch();
        int ExecuteTime_n4 = End_n8 - Begin_n8;
//        qDebug() << "ExecuteTime_n4 : " << ExecuteTime_n4;
        if(Interval_n4 > ExecuteTime_n4){
            QThread::msleep(Interval_n4 - ExecuteTime_n4);
        }else{
            QThread::msleep(1);
        }
    }
    IsRunningComplete_b1 = true;
}

void YuvBuilderThread::run(){
    while(IsRunning_b1){
        Yuv *yuv = YuvManager::GetInstance()->NextWindow();
        if(yuv == NULL || yuv->TargetYuv == NULL){
            QThread::msleep(20);
            continue;
        }

        int Interval_n4 = 1000 / 25;
        qint64 Begin_n8 = QDateTime::currentMSecsSinceEpoch();

        Yuv *TargetYuv = yuv->TargetYuv;
        yuv->Mutex.lock();
        ++yuv->UseNumber_n4;

        int RealityCellWidth_n4 = yuv->ShowWidth_n4;
        int RealityCellHeight_n4 = yuv->ShowHeight_n4;

        int RealityCellWidthHalf_n4 = RealityCellWidth_n4 >> 1;
        int RealityCellHiehgtHalf_n4 = RealityCellHeight_n4 >> 1;


        //乘以1000000，转换整型是为了提高运算效率，保留浮点数6位精度，运算完后再除以1000000
        //1048576相当于<<20
        //做除法的时候>>20，以提高效率
        int WidthRate_f4 = yuv->Width_n4 * 1.0 / RealityCellWidth_n4 * 1048576;
        int HeightRate_f4 = yuv->Height_n4 * 1.0 / RealityCellHeight_n4 * 1048576;

        int TargetColumnCount_n4 = yuv->ShowX_n4; //当前显示当窗口的数据起始位置
        int TargetColumnCountHalf_n4 = TargetColumnCount_n4 >> 1; //当前显示当窗口的数据起始位置
        int CurrentRowLineDataCount_n4 = yuv->CurrentRow_n4 * yuv->LineDataCount_n4;//己处理的行数据大小
        int CurrentRowLineDataCountQuarter_n4 = yuv->CurrentRow_n4 * yuv->LineDataCount_n4 >> 2;//己处理的行数据大小

        int YCurrentDataCount_n4 = TargetColumnCount_n4 + CurrentRowLineDataCount_n4;
        int UVCurrentDataCount_n4 = TargetColumnCountHalf_n4 + CurrentRowLineDataCountQuarter_n4;
        int YRowPos_n4 = YCurrentDataCount_n4 - TargetYuv->ShowWidth_n4;
        int TargetWidthHalf_n4 = TargetYuv->ShowWidth_n4 >> 1;
        int UVRowPos_n4 = UVCurrentDataCount_n4 - TargetWidthHalf_n4;

        int YStartPos_n4 = -HeightRate_f4;
        int UVTargetPos_n4 = 0;
        int UVSourcePos_n4 = 0;
        int YBasePos_n4 = 0;
        int UVBasePos_n4 = 0;
        bool IsUpdateUVData_b1 = false;
        int YStart_n4 = 0;
        int UVStart_n4 = 0;
        int i = 0,j = 0;
        int XStartPos_n4 = 0;

        //为了减少计算，事先生成一个下标索引列表
        int Indexs [RealityCellWidth_n4];
        XStartPos_n4 = -WidthRate_f4;
        for(i = 0;i < RealityCellWidth_n4;i++){
            XStartPos_n4 += WidthRate_f4;
            UVStart_n4 = XStartPos_n4 >> 20;
            Indexs[i] = UVStart_n4;
        }
        //end 为了减少计算，事先生成一个下标索引列表

        for(i = 0;i < RealityCellHeight_n4;i++){ //为了减少循环次数，特将yuv放在一个循环里面处理
            YRowPos_n4 += TargetYuv->ShowWidth_n4;
            YStartPos_n4 += HeightRate_f4;
            YStart_n4 = YStartPos_n4 >> 20;
            YBasePos_n4 = YStart_n4 * yuv->YSize_n4;
            IsUpdateUVData_b1 = i < RealityCellHiehgtHalf_n4;
            if(IsUpdateUVData_b1){ //为了减少循环次数，特将yuv放在一个循环里面处理,固多了此判断
                UVRowPos_n4 += TargetWidthHalf_n4;
                UVBasePos_n4 = YStart_n4 * yuv->USize_n4;
            }

            for(j = 0;j < RealityCellWidth_n4;j++){
                UVStart_n4 = Indexs[j];
                TargetYuv->YData_pc1[YRowPos_n4 + j] = yuv->YData_pc1[YBasePos_n4 + UVStart_n4];
                if(IsUpdateUVData_b1 && j < RealityCellWidthHalf_n4){ //为了减少循环次数，特将yuv放在一个循环里面处理,固多了此判断
                    UVTargetPos_n4 = UVRowPos_n4 + j;
                    UVSourcePos_n4 = UVBasePos_n4 + UVStart_n4;
//                    TargetYuv->UData_pc1[UVTargetPos_n4] = 0;
//                    TargetYuv->VData_pc1[UVTargetPos_n4] = 0;
                    TargetYuv->UData_pc1[UVTargetPos_n4] = yuv->UData_pc1[UVSourcePos_n4];
                    TargetYuv->VData_pc1[UVTargetPos_n4] = yuv->VData_pc1[UVSourcePos_n4];
                }
            }
        }
        yuv->IsNeedUpdate_b1 = false;
        --yuv->UseNumber_n4;
        if(yuv->UseNumber_n4 == 0 && yuv->IsWaitDelete_b1){
            YuvManager::GetInstance()->ReallyRemoveYuv(yuv);
        }else{
            yuv->Mutex.unlock();
        }

        qint64 End_n8 = QDateTime::currentMSecsSinceEpoch();
        int ExecuteTime_n4 = End_n8 - Begin_n8;
//        qDebug() << "ExecuteTime_n4 : " << ExecuteTime_n4;
        if(Interval_n4 > ExecuteTime_n4){
            QThread::msleep(Interval_n4 - ExecuteTime_n4);
        }else{
            QThread::msleep(1);
        }
    }
}
