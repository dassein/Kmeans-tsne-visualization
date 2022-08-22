#ifndef POINT_H
#define POINT_H
#include <cstring>
#include <iostream>
#include <assert.h>
using namespace std;
class Point
{
public:
    float* pdata;
    int d;
    Point(int& dimension) : d(dimension) {
        pdata = new float[d];
        memset(pdata, 0, d*sizeof(*pdata));
    };
    Point(float* data, int& dimension) : d(dimension) {
        pdata = new float[d];
        for (int i=0; i < d; i++) {
            pdata[i] = data[i];
        }
    }
    Point(double* data, int& dimension) : d(dimension) {
        pdata = new float[d];
        for (int i=0; i < d; i++) {
            pdata[i] = (float)data[i];
        }
    }
    Point(const Point& p) : d(p.d) {
        //Point(p.pdata, p.d);
        pdata = new float[d];
        for (int i=0; i < d; i++) {
            pdata[i] = p.pdata[i];
        }
    }
    ~Point() {
        delete[]  pdata;
    }
    float distance(Point& p);
    void update(float* data, int& dimension);
    Point& operator=(const Point& p);
};

#endif // POINT_H
