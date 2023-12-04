#include <iostream>
#include "gdal_priv.h"

int DEM_average_correct()
{
    // 注册所有的GDAL驱动
    GDALAllRegister();

    // 打开第一幅TIFF影像
    GDALDataset* poDataset1 = (GDALDataset*)GDALOpen("F://DEM//Copernicus_DSM_COG_10_N21_00_W158_00_DEM-Error.tif", GA_ReadOnly);
    if (poDataset1 == NULL)
    {
        std::cout << "打开第一幅影像失败！" << std::endl;
        return 1;
    }

    // 打开第二幅TIFF影像
    GDALDataset* poDataset2 = (GDALDataset*)GDALOpen("F://DEM//ICESat_data.tif", GA_ReadOnly);
    if (poDataset2 == NULL)
    {
        std::cout << "打开第二幅影像失败！" << std::endl;
        return 1;
    }

    // 确保两幅影像有相同的大小和投影
    if (poDataset1->GetRasterXSize() != poDataset2->GetRasterXSize() ||
        poDataset1->GetRasterYSize() != poDataset2->GetRasterYSize() ||
        strcmp(poDataset1->GetProjectionRef(), poDataset2->GetProjectionRef()) != 0)
    {
        std::cout << "两幅影像大小或投影不一致！" << std::endl;
        GDALClose(poDataset1);
        GDALClose(poDataset2);
        return 1;
    }

    // 获取影像的宽度和高度
    int width = poDataset1->GetRasterXSize();
    int height = poDataset1->GetRasterYSize();

    // 创建输出影像文件
    GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
    GDALDataset* poOutputDataset = poDriver->Create("F://DEM//dsm_output_correct.tif", width, height, 1, GDT_Float32, NULL);
    if (poOutputDataset == NULL)
    {
        std::cout << "创建输出影像文件失败！" << std::endl;
        GDALClose(poDataset1);
        GDALClose(poDataset2);
        return 1;
    }

    // 设置输出影像的地理坐标转换信息和投影信息
    double adfGeoTransform[6];
    poDataset1->GetGeoTransform(adfGeoTransform);
    poOutputDataset->SetGeoTransform(adfGeoTransform);
    const char* projection = poDataset1->GetProjectionRef();
    poOutputDataset->SetProjection(projection);

    // 读取两幅影像的像元值
    float* image1Data = new float[width * height];
    float* image2Data = new float[width * height];
    poDataset1->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, width, height, image1Data, width, height, GDT_Float32, 0, 0);
    poDataset2->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, width, height, image2Data, width, height, GDT_Float32, 0, 0);

    // 计算两幅影像对应区域数值之差的平均值
    float* outputData = new float[width * height];
    int validCount = 0;
    float sum = 0.0;
    //定义平均误差
    float avgError = 0.0;
    for (int i = 0; i < width * height; i++)
    {
        if ( image2Data[i] != 0&& image1Data[i] != 0)//不为0的地方相减求平均
        {
            outputData[i] = static_cast<float>(image1Data[i] - image2Data[i]);
            sum += outputData[i];
            validCount++;
            avgError = float(float(sum) / validCount);
        }
        
    }
    // 添加整体高程误差
    float* correctedElevation = new float[width * height];
    for (int j = 0; j < width * height; j++)
    {
        correctedElevation[j] = image1Data[j] - avgError;
    }
    /*for (int j = 0; j < width * height; j++) {
        if (image1Data[j] != 0 && image2Data[j] != 0) {
            correctedElevation[j] = image2Data[j];
        }
        else {
            correctedElevation[j] = image1Data[j] - avgError;
        }
   }*/
     

   
    // 写入结果到输出影像文件
    poOutputDataset->GetRasterBand(1)->RasterIO(GF_Write, 0, 0, width, height, correctedElevation, width, height, GDT_Float32, 0, 0);
    
    // 释放资源
    delete[] image1Data;
    delete[] image2Data;
    delete[] outputData;
    delete[] correctedElevation;
    GDALClose(poDataset1);
    GDALClose(poDataset2);
    GDALClose(poOutputDataset);
    std::cout << "Success!" << std::endl;

    return 0;
}
