#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  ui->CentroidspinBox->setRange(0,500);
  ui->ClusterspinBox->setRange(0,500);
  ui->FramespinBox->setRange(0,500);
  ui->IterationspinBox->setRange(0,500);
  ui->PointsizespinBox->setRange(0,500);
  // update param
  connect(ui->FramespinBox, SIGNAL(valueChanged(int)), ui->viewWidget, SLOT(update_frame_rate(int)));
  connect(ui->PointsizespinBox, SIGNAL(valueChanged(int)), ui->viewWidget, SLOT(update_size_point(int)));
  connect(ui->CentroidspinBox, SIGNAL(valueChanged(int)), ui->viewWidget, SLOT(update_size_centroid(int)));
  connect(ui->ClusterspinBox, SIGNAL(valueChanged(int)), ui->viewWidget, SLOT(update_n_cluster(int)));
  connect(ui->IterationspinBox, SIGNAL(valueChanged(int)), ui->viewWidget, SLOT(update_n_iter(int)));
  connect(ui->methodcomboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(update_init_type()));
  connect(ui->filenamecomboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(update_filename()));
  connect(ui->dimension_tsne_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(update_dimension_tsne()));
  // run kmeans, stop/resume iterations, prev/next iteration
  connect(ui->pushButton, SIGNAL(released()), ui->viewWidget, SLOT(run_kmeans()));
  connect(ui->stop_pushButton, SIGNAL(released()), ui->viewWidget, SLOT(stop()));
  connect(ui->prev_pushButton, SIGNAL(released()), ui->viewWidget, SLOT(prev()));
  connect(ui->next_pushButton, SIGNAL(released()), ui->viewWidget, SLOT(next()));
  connect(ui->translate_x_Slider,SIGNAL(valueChanged(int)),ui->viewWidget,SLOT(update_translation_x(int)));
  connect(ui->translate_y_Slider,SIGNAL(valueChanged(int)),ui->viewWidget,SLOT(update_translation_y(int)));
  connect(ui->zoomSlider,SIGNAL(valueChanged(int)),ui->viewWidget,SLOT(update_zoom_factor(int)));
  connect(ui->rotationSlider,SIGNAL(valueChanged(int)),ui->viewWidget,SLOT(update_angle_controlled(int)));
}

MainWindow::~MainWindow()
{
  delete ui;
}

//void MainWindow::update_frame_rate() { ui->viewWidget->update_frame_rate(ui->FramespinBox->value()); }
//void MainWindow::update_size_point() { ui->viewWidget->update_size_point(ui->PointsizespinBox->value()); }
//void MainWindow::update_size_centroid() { ui->viewWidget->update_size_centroid(ui->CentroidspinBox->value()); }
//void MainWindow::update_n_cluster() { ui->viewWidget->update_n_cluster(ui->ClusterspinBox->value()); }
//void MainWindow::update_n_iter() { ui->viewWidget->update_n_iter(ui->IterationspinBox->value()); }
void MainWindow::update_init_type() { ui->viewWidget->update_init_type(ui->methodcomboBox->currentText().toStdString()); }
void MainWindow::update_filename() { ui->viewWidget->update_filename(ui->filenamecomboBox->currentText().toStdString()); }
void MainWindow::update_dimension_tsne() { ui->viewWidget->update_dimension_tsne(ui->dimension_tsne_comboBox->currentText().toInt()); }

