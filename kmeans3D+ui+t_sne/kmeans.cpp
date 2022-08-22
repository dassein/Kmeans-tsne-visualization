#include "kmeans.h"
using namespace std;
Kmeans::Kmeans(const int& K) : n_cluster(K), init_type("random_real"), n_iter(100) { }
Kmeans::Kmeans(const int& K, const string& init) : n_cluster(K), init_type(init), n_iter(100) { }
Kmeans::Kmeans(const int& K, const string& init, const int& iter) : n_cluster(K), init_type(init), n_iter(iter) { }
Kmeans::option Kmeans::resolve_option(string& init) {
    if (init.compare("random_sample") == 0) return random_sample;
    if (init.compare("kmeans_plusplus") == 0) return kmeans_plusplus;
    return random_real;
}
void Kmeans::init_centroid(vector<Point> x) {
    switch( resolve_option(init_type) ) {
        case random_real: {
            init_random_real(x);
            break;
        }
        case random_sample: {
            init_random_sample(x);
            break;
        }
        case kmeans_plusplus: {
            init_kmeans_plusplus(x);
            break;
        }
        default: break;
    }
}
void Kmeans::init_random_real(vector<Point> x) {
    int d = x.at(0).d; // dimension
    int k = n_cluster; // number of clusters
    centroid.clear();
    // get range of d dimensions [limit_min, limit_max]
    float* plimit_min = new float[d];
    float* plimit_max = new float[d];
    for (auto i=0; i < d; i++) { plimit_min[i] = numeric_limits<float>::max(); }
    for (auto i=0; i < d; i++) { plimit_max[i] = numeric_limits<float>::min(); }
    for (Point& p : x) {
        float* p1 = plimit_min;
        float* p2 = p.pdata;
        for (auto i=0; i < d; i++) {
            if (*p1 > *p2) *p1 = *p2;
            p1++; p2++;
        }
        p1 = plimit_max;
        p2 = p.pdata;
        for (auto i=0; i < d; i++) {
            if (*p1 < *p2) *p1 = *p2;
            p1++; p2++;
        }
    }
    // initialize point within range [limit_min, limit_max]
    float* p_random = new float[d];
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(0.0, 1.0);
    for (auto iter_cluster=0; iter_cluster < k; iter_cluster++) {
        for (auto i=0; i < d; i++) {
            p_random[i] = plimit_min[i]+(plimit_max[i]-plimit_min[i]) * dis(gen);
        }
        centroid.push_back(Point(p_random, d));
    }
    delete [] p_random;
    // collect garbage
    delete [] plimit_min;
    delete [] plimit_max;
}
void Kmeans::init_random_sample(vector<Point> x) {
    int d = x.at(0).d; // dimension
    int n = x.size();  // number of points
    int k = n_cluster; // number of clusters
    centroid.clear();
    // pick k points from n points as initial centroids
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, n-1);
    unordered_set<int> elems;
    while ((int)elems.size() < k) {
        elems.insert(dis(gen));
    }
    vector<int> result(elems.begin(), elems.end());
    shuffle(result.begin(), result.end(), gen);
    // use index stored in result to locate the centroids
    for (auto index: result) {
        centroid.push_back(Point(x.at(index)));
    }
}
void Kmeans::init_kmeans_plusplus(vector<Point> x) {
    int d = x.at(0).d; // dimension
    int n = x.size();  // number of points
    int k = n_cluster; // number of clusters
    centroid.clear();
    // initialize 1st centroid with random sample method
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, n-1);
    centroid.push_back(Point(x.at(dis(gen))));
    // estimate 2nd->last centroids
    float* pdistance = new float[n];
    for (auto i=0; i < n; i++) { pdistance[i] = numeric_limits<float>::max(); }
    for (auto iter_cluster=1; iter_cluster < k; iter_cluster++) {
        int i = 0;
        for (Point& p : x) {
            for (Point& c : centroid) {
                if (pdistance[i] > p.distance(c)) { pdistance[i] = p.distance(c); }
            }
            i++;
        }
        float distance_max = pdistance[0];
        int ind_max = 0;
        for (auto i=0; i < n; i++) {
            if (pdistance[i] > distance_max) {
                distance_max = pdistance[i];
                ind_max = i;
            }
        }
        centroid.push_back(Point(x.at(ind_max)));
    }
}
void Kmeans::fit(vector<Point> x) {
    init_centroid(x);
    vector<int> y_prev;
    vector<int> y = predict(x);
    int iter = 0;
    int k = n_cluster;
    int d = centroid.at(0).d; // dimension
    float** ppsum = new float*[k];
    for (auto iter_cluster=0; iter_cluster < k; iter_cluster++) {
        ppsum[iter_cluster] = new float[d];
    }
    int* pnum_clusterpoint = new int[k];
    do {
        y_prev.clear();
        y_prev = y;
        y.clear();
        memset(pnum_clusterpoint, 0, k*sizeof(*pnum_clusterpoint));
        for (auto iter_cluster=0; iter_cluster < k; iter_cluster++) {
            memset(ppsum[iter_cluster], 0, d*sizeof(*ppsum[iter_cluster]));
        }
        int ind_x = 0;
        for (auto k_min : y_prev) {
            pnum_clusterpoint[k_min] += 1;
            float* p1 = ppsum[k_min], * p2 = x.at(ind_x).pdata;
            for (auto i=0; i < d; i++) {
                *p1 += *p2;
                p1++; p2++;
            }
            ind_x++;
        }
        int iter_cluster = 0;
        for (Point& c : centroid) {
            switch (pnum_clusterpoint[iter_cluster]) {
            case 0: {
                cout << "k: " << iter_cluster << endl;
                float distance_min = c.distance(x.at(0));
                Point p_min = x.at(0);
                for (Point& p : x) {
                    if (c.distance(p) < distance_min) {
                        distance_min = c.distance(p);
                        p_min = p;
                    }
                }
                c.update(p_min.pdata, d);
                cout << "close point for cluster:" << iter_cluster << endl;
                break;
            }
            default: {
                for (auto i=0; i < d; i++) {
                    ppsum[iter_cluster][i] /= (float) pnum_clusterpoint[iter_cluster];
                }
                c.update(ppsum[iter_cluster], d);
                break;
            }
            }
            iter_cluster++;
        }
        y = predict(x);
        iter++;
    } while( !(y == y_prev) && iter < n_iter); // end iteration conditions
    for (auto iter_cluster=0; iter_cluster < k; iter_cluster++) {
        delete [] ppsum[iter_cluster];
    }
    delete [] ppsum;
    delete [] pnum_clusterpoint;
}
void Kmeans::fit(vector<Point> x,
                 vector<vector<int>>& vec_y_hist,
                 vector<vector<Point>>& vec_centroid_hist) {
    for (vector<int>& y_hist : vec_y_hist) { y_hist.clear(); y_hist.shrink_to_fit(); }
    vec_y_hist.clear(); vec_y_hist.shrink_to_fit();
    for (vector<Point>& centroid_hist : vec_centroid_hist) { centroid_hist.clear(); centroid_hist.shrink_to_fit(); }
    vec_centroid_hist.clear(); vec_centroid_hist.shrink_to_fit();
    init_centroid(x);
    vector<int> y_prev;
    vector<int> y = predict(x);
    int iter = 0;
    int k = n_cluster;
    int d = centroid.at(0).d; // dimension
    float** ppsum = new float*[k];
    for (auto iter_cluster=0; iter_cluster < k; iter_cluster++) {
        ppsum[iter_cluster] = new float[d];
    }
    int* pnum_clusterpoint = new int[k];
    do {
        vec_y_hist.push_back(y);
        vec_centroid_hist.push_back(centroid);
        y_prev.clear();
        y_prev = y;
        y.clear();
        memset(pnum_clusterpoint, 0, k*sizeof(*pnum_clusterpoint));
        for (auto iter_cluster=0; iter_cluster < k; iter_cluster++) {
            memset(ppsum[iter_cluster], 0, d*sizeof(*ppsum[iter_cluster]));
        }
        int ind_x = 0;
        for (auto k_min : y_prev) {
            pnum_clusterpoint[k_min] += 1;
            float* p1 = ppsum[k_min], * p2 = x.at(ind_x).pdata;
            for (auto i=0; i < d; i++) {
                *p1 += *p2;
                p1++; p2++;
            }
            ind_x++;
        }
        int iter_cluster = 0;
        for (Point& c : centroid) {
            switch (pnum_clusterpoint[iter_cluster]) {
            case 0: {
                float distance_min = c.distance(x.at(0));
                Point p_min = x.at(0);
                for (Point& p : x) {
                    if (c.distance(p) < distance_min) {
                        distance_min = c.distance(p);
                        p_min = p;
                    }
                }
                c.update(p_min.pdata, d);
                break;
            }
            default: {
                for (auto i=0; i < d; i++) {
                    ppsum[iter_cluster][i] /= (float) pnum_clusterpoint[iter_cluster];
                }
                c.update(ppsum[iter_cluster], d);
                break;
            }
            }
            iter_cluster++;
        }
        y = predict(x);
        iter++;
    } while( !(y == y_prev) && iter < n_iter); // end iteration conditions
    vec_y_hist.push_back(y);
    vec_centroid_hist.push_back(centroid);
    for (auto iter_cluster=0; iter_cluster < k; iter_cluster++) {
        delete [] ppsum[iter_cluster];
    }
    delete [] ppsum;
    delete [] pnum_clusterpoint;
}

vector<int> Kmeans::predict(vector<Point> x) {
    vector<int> y;
    for (Point& p : x) {
        float distance_min = numeric_limits<float>::max();
        int k = 0, k_min = 0;
        for (Point& c : centroid) {
            if (p.distance(c) < distance_min) {
                distance_min = p.distance(c);
                k_min = k;
            }
            k++;
        }
        y.push_back(k_min);
    }
    return y;
}

float Kmeans::get_energy(vector<Point> x) {
    float energy = 0.0;
    for (Point& p : x) {
        float distance_min = numeric_limits<float>::max();
        for (Point& c : centroid) {
            if (p.distance(c) < distance_min) {
                distance_min = p.distance(c);
            }
        }
        energy += distance_min;
    }
    return energy / x.size();
}
