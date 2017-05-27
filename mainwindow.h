#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QGLWidget>
#include <QtOpenGL>
#include <QTimer>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fenv.h>

struct args
{
  int thread_num, total_threads, n;
  double hx, hy;
  double *x, *y;
  int rhs_type; // 0-usual, 1-test
};

class MainWindow : public QGLWidget
{
  Q_OBJECT
  
public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();
  void drawAxis ();
  void drawInterpolation ();
  void drawFunction ();
  void drawThreshold ();

  void initializeGL ();
  void resizeGL (int nWidth, int nHeight);
  void paintGL ();
  void keyPressEvent (QKeyEvent *ke);
  void mouseMoveEvent (QMouseEvent *me);
  void mousePressEvent (QMouseEvent *me);
  void mouseReleaseEvent (QMouseEvent *me);
  void wheelEvent (QWheelEvent *we);
  int parce_comand_line (int argc, char *argv[]);
  void recount ();

private:
  int n, N, h, w;
  double x[4], y[4];
  struct args *ar;
  double hx, hy, zmax;
  int total_threads;
  double *coefs;
  int draw_type;

  QPoint mouse_position;
  double xRot, zRot, scale;
  bool is_pressed;
};
void init_sparse_matrix_rhs (double *a, unsigned int *jnz, double *f, double *x, double *y, int n, int thread_num, int total_threads);
void init_sparse_matrix (double *a, unsigned int *jnz, double *f, double *x, double *y, int n, int thread_num, int total_threads);
void init_rhs_for_test (double *rhs, int N, double *f, double *a, unsigned int *jnz, int k, int p);
void print_sparse_matrix (double *a, unsigned int *jnz, int N, int non_zeros);
double ff (double x, double y);
void get_my_rows (int n, int k, int p, int *i1, int *i2);
int solve (double *a, unsigned int *rnz, int n, double *b, double *x, double eps, int maxit, double *r, double *u, double *v, int k, int p);
void matr_mult (double *a, unsigned int *jnz, int n, double *x, double *b, int thread_num, int total_threads);
void print_vector (double *a, int N);
void preconditioner (double *d, int n, double *y, double *x, int k, int p);
double scalp (double *x, double *y, int n, int k, int p);
void sub_vect (int n, double *u, double tau, double *v, int k, int p);
void *main_thread (void *arg);
double get_value (int i, int j, double *coefs, int n);

template <typename T>
void reduce_sum (int p, T *a = 0, int n = 0)
{
  static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
  static pthread_cond_t c_in = PTHREAD_COND_INITIALIZER;
  static pthread_cond_t c_out = PTHREAD_COND_INITIALIZER;

  static int t_in = 0;
  static int t_out = 0;
  static T *p_a = 0;

  int i;

  if (p <= 1)
    return;

  pthread_mutex_lock (&m);
  if (!p_a)
    p_a = a;
  else if (a)
    for (i = 0; i < n; i++)
      p_a[i] += a[i];

  t_in++;
  if (t_in < p)
    while (t_in < p)
      pthread_cond_wait (&c_in, &m);
  else
    {
      t_out = 0;
      pthread_cond_broadcast (&c_in);
    }

  if (p_a != a)
    for (i = 0; i < n; i++)
      a[i] = p_a[i];

  t_out++;
  if (t_out < p)
    while (t_out < p)
      pthread_cond_wait (&c_out, &m);
  else
    {
      p_a = 0, t_in = 0;
      pthread_cond_broadcast (&c_out);
    }
  pthread_mutex_unlock (&m);
}



#endif // MAINWINDOW_H
