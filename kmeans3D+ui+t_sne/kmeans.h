#ifndef KMEANS_H
#define KMEANS_H
#include <vector>
#include <random>
#include <unordered_set>
#include <algorithm>
#include <cmath>
#include <limits>
#include "point.h"
using namespace std;
class Kmeans
{
private:
    int n_cluster;
    string init_type;
    int n_iter;
    vector<Point> centroid;
    enum option {
        random_real,
        random_sample,
        kmeans_plusplus
    };
    option resolve_option(string& init);
    void init_random_real(vector<Point> x);
    void init_random_sample(vector<Point> x);
    void init_kmeans_plusplus(vector<Point> x);

public:
    Kmeans(const int& K);
    Kmeans(const int& K, const string& init);
    Kmeans(const int& K, const string& init, const int& iter);
    void init_centroid(vector<Point> x);
    void fit(vector<Point> x);
    void fit(vector<Point> x, vector<vector<int>>& vec_y_hist, vector<vector<Point>>& vec_centroid_hist);
    vector<int> predict(vector<Point> x);
    float get_energy(vector<Point> x);
};

#endif // KMEANS_H
