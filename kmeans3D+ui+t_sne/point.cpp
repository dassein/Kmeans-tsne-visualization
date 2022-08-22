#include "point.h"

float Point::distance(Point& p) {
    if (this == &p) return 0.0;
    float norm2sq = 0;
    float* pdata1 = pdata;
    float* pdata2 = p.pdata;
    for (int i=0; i < d; i++) {
        norm2sq += (*pdata1 - *pdata2) * (*pdata1 - *pdata2);
        pdata1++;
        pdata2++;
    }
    return norm2sq;
}

void Point::update(float* data, int& dimension) {
    assert (d == dimension);
    for (auto i=0; i < d; i++) {
        pdata[i] = data[i];
    }
}

Point& Point::operator=(const Point& p) {
    if (this != &p) {
        delete[] pdata;
        d = p.d;
        pdata = new float[d];
        for (int i = 0; i < d; i++){
            pdata[i] = p.pdata[i];
        }
    }
    return *this;
}
