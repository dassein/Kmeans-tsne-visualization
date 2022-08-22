#include "ViewWidget.h"

using namespace std;
ViewWidget::ViewWidget(QWidget *parent, Qt::WindowFlags f) :
  QOpenGLWidget(parent, f)
{
  size_point = 2;
  size_centroid = 15;
  frame_rate = 1;
  n_cluster = 2;
  init_type = "random_real";
  n_iter = 100;
  filename = "a.txt";
  stopped = false;
  pkmeans = NULL;
  m_timer = new QTimer(this);
  m_timer->callOnTimeout(this, &ViewWidget::updateTurntable);
  m_turntableAngle = 0.0f;
  translation_x = 0, translation_y = 0;
  zoom_factor = 0;
  dimension_tsne = 2;
  // const char *vinit[] = {"#00ffff", "#ff86c1", "#ffff00", "#ff00ff", "#ff0000"};
  const float vinit_r[] = {0.0, 1.0,             1.0, 1.0, 1.0};
  const float vinit_g[] = {1.0, float(0x86)/255, 1.0, 0.0, 0.0};
  const float vinit_b[] = {1.0, float(0xc1)/255, 0.0, 1.0, 0.0};
  vec_r.insert(vec_r.end(), vinit_r, vinit_r+(sizeof(vinit_r)/sizeof(vinit_r[0])));
  vec_g.insert(vec_g.end(), vinit_g, vinit_g+(sizeof(vinit_g)/sizeof(vinit_g[0])));
  vec_b.insert(vec_b.end(), vinit_b, vinit_b+(sizeof(vinit_b)/sizeof(vinit_b[0])));
  run_kmeans();
}

ViewWidget::~ViewWidget() {
    delete pkmeans;
    delete m_timer;
}

vector<Point> ViewWidget::read_in() {
    QFile input(path_in.c_str());
    if(!input.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, path_in.c_str(), input.errorString());
    }
    QTextStream instream(&input);
    vector<Point> x;
    float * pdata;
    int is1st = 1;
    bool do_tsne = false;
    while (!instream.atEnd()) {
        QString line = instream.readLine();
        QRegExp separator("(,| )");
        QStringList fields = line.split(separator);
        switch (is1st) {
        case 1: {
            dimension = fields.size();
            pdata = new float[dimension];
            is1st = 0;
            break;
        }
        default: { break; }
        }
        if (dimension > 3) { do_tsne = true; break; }
        memset(pdata, 0, dimension*sizeof(*pdata));
        int i=0; for (auto& str : fields) { pdata[i] = str.toFloat(); i++; }
        x.push_back(Point(pdata, dimension));
    }
    if (is1st == 0) { delete [] pdata; }
    input.close();
    if (do_tsne) {
        int num_point;
        int dimension_in = dimension - 1; // first data for each row in MNIST is 0-9
        dimension = dimension_tsne; // dimension_out for t-SNE
        double* Y = exec_tsne(path_in, num_point, dimension_in, dimension);
        double* pdata = Y;
        for (auto i=0; i < num_point; i++) {
            x.push_back(Point(pdata, dimension));
            pdata += dimension;
        }
        free(Y);
    }
    return x;
}
// https://lvdmaaten.github.io/publications/papers/JMLR_2014.pdf
// Accelerating t-SNE using Tree-Based Algorithms
double* ViewWidget::exec_tsne(string path, int& N, int dimension_in, int dimension_out) {
    // N: number of points
    int max_iter=550;
    double perplexity; // related to the number of nearest neighbors, consider selecting 5-50
    double theta=0.5; // for Barnes-Hut T-SNE. range of 0.2-0.8.  < 0.2 increases computation time, > 0.8 has increases error
    int rand_seed = -1;
    double *data, *Y=NULL; // input/outyput of t-SNE
    TSNE* tsne = new TSNE();
    if(tsne->read_data(path, &data, N, dimension_in)) {
        cout << "number of points in t-SNE: " << N << endl;
        perplexity = int(sqrt(N)/2) > 5? int(sqrt(N)/2) : 5;
        Y = (double*) malloc(N * dimension_out * sizeof(double));
        tsne->run(data, N, dimension_in, Y, dimension_out, perplexity, theta, rand_seed, false, max_iter);
        free(data); data = NULL;
    }
    delete(tsne);
    return Y;
}

void ViewWidget::get_sample_space() {
    float x_min = numeric_limits<float>::max(),
          y_min = numeric_limits<float>::max(),
          z_min = numeric_limits<float>::max();
    float x_max = numeric_limits<float>::min(),
          y_max = numeric_limits<float>::min(),
          z_max = numeric_limits<float>::min();
    switch (dimension) {
    case 2: {
        z_min = 0; z_max = 0;
        for (Point& p : x) {
            if (x_min > p.pdata[0]) { x_min = p.pdata[0]; }
            if (x_max < p.pdata[0]) { x_max = p.pdata[0]; }
            if (y_min > p.pdata[1]) { y_min = p.pdata[1]; }
            if (y_max < p.pdata[1]) { y_max = p.pdata[1]; }
        }
        break;
    }
    default: {
        for (Point& p : x) {
            if (x_min > p.pdata[0]) { x_min = p.pdata[0]; }
            if (x_max < p.pdata[0]) { x_max = p.pdata[0]; }
            if (y_min > p.pdata[1]) { y_min = p.pdata[1]; }
            if (y_max < p.pdata[1]) { y_max = p.pdata[1]; }
            if (z_min > p.pdata[2]) { z_min = p.pdata[2]; }
            if (z_max < p.pdata[2]) { z_max = p.pdata[2]; }
        }
        break;
    }
    }
    xbar = (x_min + x_max) / 2;
    ybar = (y_min + y_max) / 2;
    zbar = (z_min + z_max) / 2;
    dx = x_max - x_min;
    dy = y_max - y_min;
    dz = (z_max - z_min) > 4? (z_max - z_min) : 4; // default 4
}

void ViewWidget::initializeGL()
{
  initializeOpenGLFunctions();
  glClearColor(0.2, 0.2, 0.2, 1.0);
  glEnable(GL_DEPTH_TEST);
  m_pointProgram.addShaderFromSourceCode(QOpenGLShader::Vertex,
    "attribute highp vec4 vertex;\n"
    "attribute mediump vec4 color;\n"
    "varying mediump vec4 vColor;\n"
    "uniform highp mat4 matrix;\n"
    "void main(void)\n"
    "{\n"
    "   gl_Position = matrix * vertex;\n"
    "   vColor = color;\n"
    "}");
  m_pointProgram.addShaderFromSourceCode(QOpenGLShader::Fragment,
    "varying mediump vec4 vColor;\n"
    "void main(void)\n"
    "{\n"
    "   gl_FragColor = vColor;\n"
    "}");
  m_pointProgram.link();
}

void ViewWidget::paintGL()
{
  QOpenGLShaderProgram program;
  program.addShaderFromSourceCode(QOpenGLShader::Vertex,
        "attribute highp vec4 vertex;\n"
        "uniform highp mat4 matrix;\n"
        "void main(void)\n"
        "{\n"
        "   gl_Position = matrix * vertex;\n"
        "}");
  program.addShaderFromSourceCode(QOpenGLShader::Fragment,
        "uniform mediump vec4 color;\n"
        "void main(void)\n"
        "{\n"
        "   gl_FragColor = color;\n"
        "}");
  program.link();
  int vertexLocation = program.attributeLocation("vertex");
  int matrixLocation = program.uniformLocation("matrix");
  // int colorLocation = program.uniformLocation("color");
  QMatrix4x4 pmvMatrix;
  float angle_persp = 2 * 180 * atan2(0.5*max(dx, dy), 2*dz) / M_PI;
  float far_plane = 3.5*dz > 1500.0f? 3.5*dz : 1500.0f;
  pmvMatrix.perspective(angle_persp, float(width())/height(), 0.01f, far_plane);
  pmvMatrix.lookAt({0, 0, (2.5f+zoom_factor)*dz}, {0, 0, 0}, {0, 1, 0});
  pmvMatrix.translate(-xbar+translation_x*dx, -ybar+translation_y*dy,  -zbar);
  pmvMatrix.rotate(m_turntableAngle+angle_controlled,{0.0f, 1.0f, 0.0f});
  program.setUniformValue(matrixLocation, pmvMatrix);
  m_pointProgram.bind();
  m_pointProgram.enableAttributeArray("vertex");
  m_pointProgram.enableAttributeArray("color");
  m_pointProgram.setUniformValue("martix",pmvMatrix);
  // draw data
  glPointSize(size_point);
  m_pointProgram.setAttributeArray("vertex",m_points.constData(),3);
  m_pointProgram.setAttributeArray("color",m_colors.constData(),3);
  glDrawArrays(GL_POINTS,0, m_points.count()/3);
  // draw centroids
  glPointSize(size_centroid);
  m_pointProgram.setAttributeArray("vertex",m_centroid.constData(),3);
  m_pointProgram.setAttributeArray("color",m_centroid_color.constData(),3);
  glDrawArrays(GL_POINTS,0, m_centroid.count()/3);
  m_pointProgram.disableAttributeArray("vertex");
  m_pointProgram.disableAttributeArray("color");
  program.enableAttributeArray(vertexLocation);
  program.setUniformValue(matrixLocation,pmvMatrix);
  program.disableAttributeArray(vertexLocation);
  // draw iteration text
  QPainter painter(this);
  QFont font = painter.font() ;
  font.setPointSize(font.pointSize() * 4);
  painter.setFont(font);
  painter.drawText(QRect(0, height() -80, width(), 80),
                   QString("iteration: ") + QString::number(iteration, 10)); // base 10 digit
}

void ViewWidget::updateTurntable()
{
  if (!stopped)
      iteration = (iteration + 1) % vec_y_hist.size();
  vector<int> y_hist = vec_y_hist.at(iteration);
  vector<Point> centroid_hist = vec_centroid_hist.at(iteration);
  m_colors.clear();
  m_centroid.clear();
  for (auto k_min : y_hist) {
       float r = vec_r.at(k_min % vec_r.size());
       float g = vec_g.at(k_min % vec_g.size());
       float b = vec_b.at(k_min % vec_b.size());
       m_colors.append({r, g, b});
  }
  switch (dimension) {
  case 2: {
      for (Point& c : centroid_hist) {
          m_centroid.append({c.pdata[0], c.pdata[1], 0.0});
      }
      break;
  }
  default: {
      for (Point& c : centroid_hist) {
          m_centroid.append({c.pdata[0], c.pdata[1], c.pdata[2]});
      }
      if (!stopped)
          m_turntableAngle += 5.0f;
      break;
  }
  }
  update();
}

void ViewWidget::update_frame_rate(int frame_rate) { this->frame_rate = (float)frame_rate; }
void ViewWidget::update_size_point(int size_point) { this->size_point = (float)size_point; }
void ViewWidget::update_size_centroid(int size_centroid) { this->size_centroid = (float)size_centroid; }
void ViewWidget::update_n_cluster(int n_cluster) { this->n_cluster = n_cluster; }
void ViewWidget::update_init_type(string init_type) { this->init_type = init_type; }
void ViewWidget::update_n_iter(int n_iter) { this->n_iter = n_iter; }
void ViewWidget::update_filename(string filename) { this->filename = filename; }
void ViewWidget::update_dimension_tsne(int dimension_tsne) { this->dimension_tsne = dimension_tsne; }
void ViewWidget::update_translation_x(int pos_x) { this->translation_x = (float)(pos_x-50)/100; }
void ViewWidget::update_translation_y(int pos_y) { this->translation_y = (float)(pos_y-50)/100; }
void ViewWidget::update_zoom_factor(int pos_zoom){ this->zoom_factor =(float)(pos_zoom-50)/100; }
void ViewWidget::update_angle_controlled(int pos_angle) {
    if (stopped) {
        this->angle_controlled = 360 * (float)(pos_angle-50) / 100;
    } else {
        this->angle_controlled = 0;
    }
}
void ViewWidget::run_kmeans()
{
  stopped = false;
  m_timer->stop();
  delete pkmeans;
  pkmeans = new Kmeans(n_cluster, init_type, n_iter);
  path_in_prev = path_in;
  path_in = string("/home/dassein/Documents/C++_Qt/Kmeans3D+ui+t_SNE/data/") + filename;
  m_turntableAngle = 0.0f;
  if (path_in.compare(path_in_prev) != 0) {
      x.clear();
      x = read_in();
  }
  // check number of clusters && number of points
  if( this->n_cluster == 0 ) { // K = 0
        QMessageBox::information(0, "", "K = 0");
  }
  if( this->n_cluster == 1 ) { // K = 1
        QMessageBox::information(0, "", "K = 1");
  }
  if( x.size() == 0 )  { // N = 0
        QMessageBox::information(0, "", "N = 0");
  }
  if( (int)x.size() < this->n_cluster )  { // K > N
        QMessageBox::information(0, "", "K > N");
  }
  dimension = x.at(0).d;
  get_sample_space();
  // record history of kmeans
  vec_y_hist.clear();
  vec_centroid_hist.clear();
  pkmeans->fit(x, vec_y_hist, vec_centroid_hist);
  float energy = pkmeans->get_energy(x);
  cout << "energy: " << energy << endl;
  cout << "number of iterations: " << vec_y_hist.size()-1 << endl;
  iteration = 0;
  // set color list
  vector<int> y_hist = vec_y_hist.at(iteration);
  vector<Point> centroid_hist = vec_centroid_hist.at(iteration);
  m_colors.clear();
  for (auto k_min : y_hist) {
      float r = vec_r.at(k_min % vec_r.size());
      float g = vec_g.at(k_min % vec_g.size());
      float b = vec_b.at(k_min % vec_b.size());
      m_colors.append({r, g, b});
  }
  m_centroid_color.clear();
  for (auto k=0; k < n_cluster; k++) {
      m_centroid_color.append({1, 1, 1});
  }
  // set coordinates of points
  m_points.clear();
  m_centroid.clear();
  switch (dimension) {
  case 2: {
      for (Point& p : x) {
          m_points.append({p.pdata[0], p.pdata[1], 0.0});
      }
      for (Point& c : centroid_hist) {
          m_centroid.append({c.pdata[0], c.pdata[1], 0.0});
      }
      break;
  }
  default: {
      for (Point& p : x) {
          m_points.append({p.pdata[0], p.pdata[1], p.pdata[2]});
      }
      for (Point& c : centroid_hist) {
          m_centroid.append({c.pdata[0], c.pdata[1], c.pdata[2]});
      }
      break;
  }
  }
  m_timer->start(1000/frame_rate); // set frame rate, 1000 means 1000ms for each frame
}

void ViewWidget::stop() { stopped = !(stopped); }
void ViewWidget::prev() {
    if (stopped) {
        iteration = (iteration - 1) % vec_y_hist.size();
        switch (dimension) {
        case 2: break;
        default: { m_turntableAngle -= 5.0f; break; }
        }
    }

}
void ViewWidget::next() {
    if (stopped) {
        iteration = (iteration + 1) % vec_y_hist.size();
        switch (dimension) {
        case 2: break;
        default: { m_turntableAngle += 5.0f; break; }
        }
    }
}
