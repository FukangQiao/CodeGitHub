#include <iostream>
#include "gdal_priv.h"

int DEM_average_correct()
{
    // ע�����е�GDAL����
    GDALAllRegister();

    // �򿪵�һ��TIFFӰ��
    GDALDataset* poDataset1 = (GDALDataset*)GDALOpen("F://DEM//Copernicus_DSM_COG_10_N21_00_W158_00_DEM-Error.tif", GA_ReadOnly);
    if (poDataset1 == NULL)
    {
        std::cout << "�򿪵�һ��Ӱ��ʧ�ܣ�" << std::endl;
        return 1;
    }

    // �򿪵ڶ���TIFFӰ��
    GDALDataset* poDataset2 = (GDALDataset*)GDALOpen("F://DEM//ICESat_data.tif", GA_ReadOnly);
    if (poDataset2 == NULL)
    {
        std::cout << "�򿪵ڶ���Ӱ��ʧ�ܣ�" << std::endl;
        return 1;
    }

    // ȷ������Ӱ������ͬ�Ĵ�С��ͶӰ
    if (poDataset1->GetRasterXSize() != poDataset2->GetRasterXSize() ||
        poDataset1->GetRasterYSize() != poDataset2->GetRasterYSize() ||
        strcmp(poDataset1->GetProjectionRef(), poDataset2->GetProjectionRef()) != 0)
    {
        std::cout << "����Ӱ���С��ͶӰ��һ�£�" << std::endl;
        GDALClose(poDataset1);
        GDALClose(poDataset2);
        return 1;
    }

    // ��ȡӰ��Ŀ�Ⱥ͸߶�
    int width = poDataset1->GetRasterXSize();
    int height = poDataset1->GetRasterYSize();

    // �������Ӱ���ļ�
    GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
    GDALDataset* poOutputDataset = poDriver->Create("F://DEM//dsm_output_correct.tif", width, height, 1, GDT_Float32, NULL);
    if (poOutputDataset == NULL)
    {
        std::cout << "�������Ӱ���ļ�ʧ�ܣ�" << std::endl;
        GDALClose(poDataset1);
        GDALClose(poDataset2);
        return 1;
    }

    // �������Ӱ��ĵ�������ת����Ϣ��ͶӰ��Ϣ
    double adfGeoTransform[6];
    poDataset1->GetGeoTransform(adfGeoTransform);
    poOutputDataset->SetGeoTransform(adfGeoTransform);
    const char* projection = poDataset1->GetProjectionRef();
    poOutputDataset->SetProjection(projection);

    // ��ȡ����Ӱ�����Ԫֵ
    float* image1Data = new float[width * height];
    float* image2Data = new float[width * height];
    poDataset1->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, width, height, image1Data, width, height, GDT_Float32, 0, 0);
    poDataset2->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, width, height, image2Data, width, height, GDT_Float32, 0, 0);

    // ��������Ӱ���Ӧ������ֵ֮���ƽ��ֵ
    float* outputData = new float[width * height];
    int validCount = 0;
    float sum = 0.0;
    //����ƽ�����
    float avgError = 0.0;
    for (int i = 0; i < width * height; i++)
    {
        if ( image2Data[i] != 0&& image1Data[i] != 0)//��Ϊ0�ĵط������ƽ��
        {
            outputData[i] = static_cast<float>(image1Data[i] - image2Data[i]);
            sum += outputData[i];
            validCount++;
            avgError = float(float(sum) / validCount);
        }
        
    }
    // �������߳����
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
     

   
    // д���������Ӱ���ļ�
    poOutputDataset->GetRasterBand(1)->RasterIO(GF_Write, 0, 0, width, height, correctedElevation, width, height, GDT_Float32, 0, 0);
    
    // �ͷ���Դ
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
