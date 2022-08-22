#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

public slots:
//  void update_frame_rate();
//  void update_size_point();
//  void update_size_centroid();
//  void update_n_cluster();
//  void update_n_iter();
  void update_init_type();
  void update_filename();
  void update_dimension_tsne();
private:
  Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
