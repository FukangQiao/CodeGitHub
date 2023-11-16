#include<iostream>
#include<math.h>
using namespace std;
double fun(double x)
{
   /* if (x==0)
    {
        return 1;
    }
    else
    {
        return sin(x) / x;
        
    }*/
    return x / (4 + x * x);
   
}
double Trapezoidal(double a, double b, int n)
{
    double h = (b - a) / n;
    double result = fun(a) + fun(b);
    for (int i = 1; i < n; i++) {
        a += h;
        result += 2 * fun(a);
    }
    return result * h / 2;
}

double Simpson(double a, double b, int n)
{
    double h = (b - a) / n;
    double s1 = fun(a + h / 2);//0项，后面就从1开始
    double s2 = 0;
    for (int i = 1; i < n; i++)
    {
        s1 += fun(a + i * h + h / 2);
        s2 += fun(a + i * h);
    }
    return h * (fun(a) + 4 * s1 + 2 * s2 + fun(b)) / 6;
}

int main()
{
    int n = 8;  //区间等分数	
    double a = 1, b = 2;  //上下限	
    cout << "被积函数 " << endl;
    cout << "区间等分数为 8" << endl;
    cout << "积分区间为（0，1）" << endl;
    cout << "积分值为：" << Simpson(a, b, 4) << endl;
    cout << "积分值为：" << Trapezoidal(a, b, 8) << endl;
    return 0;
}