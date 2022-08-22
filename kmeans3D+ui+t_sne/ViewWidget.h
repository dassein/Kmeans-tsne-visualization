#ifndef VIEWWIDGET_H
#define VIEWWIDGET_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QElapsedTimer>
#include <QPainter>
#include <QtMath>
#include <QTimer>
#include <QMainWindow>
#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPen>
#include <QtAlgorithms>
#include <chrono>
#include <random>
#include <QDebug>
#include "kmeans.h"
#include "tsne.h"

class ViewWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
  Q_OBJECT

public:
  ViewWidget(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
  ~ViewWidget();
  float angleForTime(qint64 msTime,float secondsPreRotation) const;
  Kmeans* pkmeans;
  int n_cluster;
  string init_type;
  int n_iter;
  string path_in;
  string path_in_prev;
  string filename;
  float size_point;
  float size_centroid;
  float frame_rate;
  float translation_x, translation_y; // -0.5~0.5
  float zoom_factor; // -0.5~0.5
  float angle_controlled; // -180~180

protected:
  void initializeGL() override;
  void paintGL() override;

public slots:
  void updateTurntable();
  void update_frame_rate(int);
  void update_size_point(int);
  void update_size_centroid(int);
  void update_n_cluster(int);
  void update_init_type(string);
  void update_n_iter(int);
  void update_filename(string);
  void update_dimension_tsne(int);
  void run_kmeans();
  void stop();
  void prev();
  void next();
  void update_translation_x(int);
  void update_translation_y(int);
  void update_zoom_factor(int);
  void update_angle_controlled(int);

private:
  vector<Point> read_in();
  double* exec_tsne(string path, int& num_point, int dimension_in, int dimension_out);
  vector<Point> x;
  int dimension;
  int dimension_tsne;
  void get_sample_space();
  float xbar, ybar, zbar;
  float dx, dy, dz;
  vector<vector<int>> vec_y_hist;
  vector<vector<Point>> vec_centroid_hist;
  int iteration;
  bool stopped;
  vector<float> vec_r;
  vector<float> vec_g;
  vector<float> vec_b;

  float m_turntableAngle;

  QVector<float> m_points;
  QVector<float> m_colors;
  QVector<float> m_centroid;
  QVector<float> m_centroid_color;
  QOpenGLShaderProgram m_pointProgram;
  QTimer* m_timer;

//  QElapsedTimer m_elapsedTimer;
//  QElapsedTimer m_fpsTimer;
//  int m_frameCount;
//  float m_fps;
};

#endif // VIEWWIDGET_H
