/// <summary>
/// 1、导入DEM数据;
/// 2、定义二次多项式模型：
/// 3、遍历每个控制点点建立误差方程：
/// 4、构建参数估计问题、最小二乘求解
/// 5、评估平差的精度
/// </summary>
#include<iostream>
#include<gdal_priv.h>
#include <fstream>
#include<sstream>
#include <vector>
#include <iomanip>
#include"Armadillo"
void Dem_Correct_poly() {
    //注册驱动GDAL
    GDALAllRegister();
    // 导入Raw_DEM数据，存储在矩阵或数组中,
    const char* filename = "F:\\DEM\\Copernicus_DSM_COG_10_N21_00_W158_00_DEM-Error.tif";
    GDALDataset* dataset = (GDALDataset*)GDALOpen(filename, GA_ReadOnly);
    int width = dataset->GetRasterXSize();
    int height = dataset->GetRasterYSize();
    //获取投影信息、坐标转换信息
    double geoTransform[6];
    dataset->GetGeoTransform(geoTransform);
    const char* projection = dataset->GetProjectionRef();
    double xOrigin = geoTransform[0];
    double yOrigin = geoTransform[3];
    double pixelWidth = geoTransform[1];
    double pixelHeight = geoTransform[5];
    // 计算像元的位置
    std::ifstream csvFile("F://DEM//Cleaned_ICESat2_N21W158.csv");
    // 读取CSV文件中的经纬度坐标和像元值
    std::vector<double> latitudes;
    std::vector<double> longitudes;
    //std::vector<double> pixelValues;
    std::string line;
    // 逐行读取CSV文件并解析经纬度和像元值
    while (std::getline(csvFile, line))
    {
        std::istringstream iss(line);
        std::string token;
        std::getline(iss, token, ',');
        double latitude = std::stod(token);//将string类型转换成double类型的函数
        std::getline(iss, token, ',');
        double longitude = std::stod(token);
        std::getline(iss, token, ',');
        float pixelValue = std::stof(token);

        latitudes.push_back(latitude);//纬度
        longitudes.push_back(longitude);//经度
        //pixelValues.push_back(pixelValue);
    }
    csvFile.close();
    arma::mat A(latitudes.size(), 5, arma::fill::zeros);
    for (size_t i = 0; i < latitudes.size(); i++) {
        double latitude = latitudes[i];
        double longitude = longitudes[i];
        // 计算像元的位置
        int x = static_cast<int>((longitude - geoTransform[0]) / geoTransform[1]);
        int y = static_cast<int>((latitude - geoTransform[3]) / geoTransform[5]);
        // 定义二次多项式模型
        // Z(x, y) = a_0 + a_1x + a_2x^2 + b_1y + kxy ;
        A(i, 0) = 1;
        A(i, 1) = x;
        A(i, 2) = x * x;
        A(i, 3) = y;
        A(i, 4) = y * y;

    }
    arma::mat V(latitudes.size(), 1);
    arma::mat parameters(5, 1, arma::fill::zeros);
    arma::mat data_ice;//cleaned_csv
    arma::mat data_raw;//raw_dem
    data_ice.load("F://DEM//Cleaned_ICESat2_N21W158.csv", arma::csv_ascii);
    data_raw.load("F:/DEM/height_error.csv", arma::csv_ascii);//对应经纬度的高程值提取的csv
    arma::vec column_ice = data_ice.col(2);
    arma::vec column_raw = data_raw.col(2);
    arma::vec herr = column_raw - column_ice;//Y
    // 构建参数估计问题、最小二成求解参数估计
    parameters = arma::inv(A.t() * A) * A.t() * herr;
    parameters.print("parameters: ");
    // 评估平差的精度
    //Q=inv(ATA) 5*5
    arma::mat Q(5, 5, arma::fill::zeros);
    Q = arma::inv(A.t() * A);
    Q.print("Q:");
    // 读取像元值和行列号
    float* hout = new float[width * height];

    for (int row = 0; row < height; ++row) {//y
        for (int col = 0; col < width; ++col) {//x
            float rawdem;
            GDALRasterBand* band = dataset->GetRasterBand(1); // 1表示第一个波段
            band->RasterIO(GF_Read, col, row, 1, 1, &rawdem, 1, 1, GDT_Float32, 0, 0);
            float hba;
            //g(x,y)
            hba = parameters(0) + parameters(1) * row + parameters(2) * row * row + parameters(3) * col + parameters(4) * row * col;
            hout[row * width + col] = rawdem - hba;
            // 输出像元值和行列号
            //std::cout << "Row: " << row << ", Column: " << col << ", Pixel Value: " << hout[row * width + col] << std::endl;
        }
    }
    //输出tif影像
    // 创建输出影像文件
    GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
    GDALDataset* poOutputDataset = poDriver->Create("F://DEM//elevetion_correct.tif", width, height, 1, GDT_Float32, NULL);
    if (poOutputDataset == NULL)
    {
        std::cout << "创建输出影像文件失败！" << std::endl;

    }
    // 设置输出影像的地理坐标转换信息和投影信息
    poOutputDataset->SetGeoTransform(geoTransform);
    poOutputDataset->SetProjection(projection);
    // 写入结果到输出影像文件
    poOutputDataset->GetRasterBand(1)->RasterIO(GF_Write, 0, 0, width, height, hout, width, height, GDT_Float32, 0, 0);
    std::cout << "Success!" << std::endl;
    GDALClose(poOutputDataset);
    GDALClose(dataset);
}


