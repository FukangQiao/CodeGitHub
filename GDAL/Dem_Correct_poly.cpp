/// <summary>
/// 1������DEM����;
/// 2��������ζ���ʽģ�ͣ�
/// 3������ÿ�����Ƶ�㽨�����̣�
/// 4�����������������⡢��С�������
/// 5������ƽ��ľ���
/// </summary>
#include<iostream>
#include<gdal_priv.h>
#include <fstream>
#include<sstream>
#include <vector>
#include <iomanip>
#include"Armadillo"
void Dem_Correct_poly() {
    //ע������GDAL
    GDALAllRegister();
    // ����Raw_DEM���ݣ��洢�ھ����������,
    const char* filename = "F:\\DEM\\Copernicus_DSM_COG_10_N21_00_W158_00_DEM-Error.tif";
    GDALDataset* dataset = (GDALDataset*)GDALOpen(filename, GA_ReadOnly);
    int width = dataset->GetRasterXSize();
    int height = dataset->GetRasterYSize();
    //��ȡͶӰ��Ϣ������ת����Ϣ
    double geoTransform[6];
    dataset->GetGeoTransform(geoTransform);
    const char* projection = dataset->GetProjectionRef();
    double xOrigin = geoTransform[0];
    double yOrigin = geoTransform[3];
    double pixelWidth = geoTransform[1];
    double pixelHeight = geoTransform[5];
    // ������Ԫ��λ��
    std::ifstream csvFile("F://DEM//Cleaned_ICESat2_N21W158.csv");
    // ��ȡCSV�ļ��еľ�γ���������Ԫֵ
    std::vector<double> latitudes;
    std::vector<double> longitudes;
    //std::vector<double> pixelValues;
    std::string line;
    // ���ж�ȡCSV�ļ���������γ�Ⱥ���Ԫֵ
    while (std::getline(csvFile, line))
    {
        std::istringstream iss(line);
        std::string token;
        std::getline(iss, token, ',');
        double latitude = std::stod(token);//��string����ת����double���͵ĺ���
        std::getline(iss, token, ',');
        double longitude = std::stod(token);
        std::getline(iss, token, ',');
        float pixelValue = std::stof(token);

        latitudes.push_back(latitude);//γ��
        longitudes.push_back(longitude);//����
        //pixelValues.push_back(pixelValue);
    }
    csvFile.close();
    arma::mat A(latitudes.size(), 5, arma::fill::zeros);
    for (size_t i = 0; i < latitudes.size(); i++) {
        double latitude = latitudes[i];
        double longitude = longitudes[i];
        // ������Ԫ��λ��
        int x = static_cast<int>((longitude - geoTransform[0]) / geoTransform[1]);
        int y = static_cast<int>((latitude - geoTransform[3]) / geoTransform[5]);
        // ������ζ���ʽģ��
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
    data_raw.load("F:/DEM/height_error.csv", arma::csv_ascii);//��Ӧ��γ�ȵĸ߳�ֵ��ȡ��csv
    arma::vec column_ice = data_ice.col(2);
    arma::vec column_raw = data_raw.col(2);
    arma::vec herr = column_raw - column_ice;//Y
    // ���������������⡢��С��������������
    parameters = arma::inv(A.t() * A) * A.t() * herr;
    parameters.print("parameters: ");
    // ����ƽ��ľ���
    //Q=inv(ATA) 5*5
    arma::mat Q(5, 5, arma::fill::zeros);
    Q = arma::inv(A.t() * A);
    Q.print("Q:");
    // ��ȡ��Ԫֵ�����к�
    float* hout = new float[width * height];

    for (int row = 0; row < height; ++row) {//y
        for (int col = 0; col < width; ++col) {//x
            float rawdem;
            GDALRasterBand* band = dataset->GetRasterBand(1); // 1��ʾ��һ������
            band->RasterIO(GF_Read, col, row, 1, 1, &rawdem, 1, 1, GDT_Float32, 0, 0);
            float hba;
            //g(x,y)
            hba = parameters(0) + parameters(1) * row + parameters(2) * row * row + parameters(3) * col + parameters(4) * row * col;
            hout[row * width + col] = rawdem - hba;
            // �����Ԫֵ�����к�
            //std::cout << "Row: " << row << ", Column: " << col << ", Pixel Value: " << hout[row * width + col] << std::endl;
        }
    }
    //���tifӰ��
    // �������Ӱ���ļ�
    GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
    GDALDataset* poOutputDataset = poDriver->Create("F://DEM//elevetion_correct.tif", width, height, 1, GDT_Float32, NULL);
    if (poOutputDataset == NULL)
    {
        std::cout << "�������Ӱ���ļ�ʧ�ܣ�" << std::endl;

    }
    // �������Ӱ��ĵ�������ת����Ϣ��ͶӰ��Ϣ
    poOutputDataset->SetGeoTransform(geoTransform);
    poOutputDataset->SetProjection(projection);
    // д���������Ӱ���ļ�
    poOutputDataset->GetRasterBand(1)->RasterIO(GF_Write, 0, 0, width, height, hout, width, height, GDT_Float32, 0, 0);
    std::cout << "Success!" << std::endl;
    GDALClose(poOutputDataset);
    GDALClose(dataset);
}


