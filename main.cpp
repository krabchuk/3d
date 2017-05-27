//#include <QtGui/QApplication>
#include "mainwindow.h"

static double *coefss = 0;


void *main_thread (void *arg)
{
  struct args *ar = (struct args *)arg;
  static double *a = 0, *rhs = 0, *c = 0, *u = 0, *v = 0, *r = 0;
  static unsigned int *rnz = 0;
  int n = ar->n, len, error = 0;
  int non_zeros = 14 * n * n + 6 * n + 1;
  //double res3;

  len = 2 * n * (n + 1) + 1;
  if (ar->thread_num == 0)
    {
      if (!(a   = new double [non_zeros + 1])       ||
          !(rnz = new unsigned int [non_zeros + 1]) ||
          !(rhs = new double [len])                 ||
          !(c   = new double [len])                 ||
          !(u   = new double [len])                 ||
          !(r   = new double [len])                 ||
          !(v   = new double [len]))
        {
          if (a)
            delete [] a;
          a = 0;
          if (rnz)
            delete [] rnz;
          rnz = 0;
          if (rhs)
            delete [] rhs;
          rhs = 0;
          if (c)
            delete [] c;
          c = 0;
          if (u)
            delete [] u;
          u = 0;
          if (r)
            delete [] r;
          r = 0;
          if (v)
            delete [] v;
          v = 0;
          error = -1;
        }
      else
        memset (c, 0, len * sizeof (double));
    }
  reduce_sum <int>(ar->total_threads, &error, 1);
  if (error == -1)
    return 0;
  if (1)
    {
      init_sparse_matrix (a, rnz, r, ar->x, ar->y, n, ar->thread_num, ar->total_threads);
      init_rhs_for_test(rhs, len, r, a, rnz, ar->thread_num, ar->total_threads);
    }
  else
    init_sparse_matrix_rhs (a, rnz, rhs, ar->x, ar->y, n, ar->thread_num, ar->total_threads);

//  for (int i = 0; i < n + 1; i++)
//    {
//      for (int j = 0; j < 2 * i + 1; j++)
//        {
//          int k = i * i + j;
//          printf ("wtf #%d %le\n", k, get_value(i, j, rhs, n) - rhs [k]);
//        }
//    }
//  for (int i = n - 1; i > -1; i--)
//    {
//      for (int j = 0; j < 2 * i + 1; j++)
//        {
//          int k = (n + 1) * (n + 1) + j + (n - 1 - i) * (n + i + 1);
//          printf ("wtf #%d %le\n", k, get_value(2 * n - i, j, rhs, n) - rhs [k]);
//        }
//    }

  solve (a, rnz, len, rhs, c, 1e-10, 1000, r, u, v, ar->thread_num, ar->total_threads);
  reduce_sum <int>(ar->total_threads);
  if (c || ar->thread_num == 0)
    memcpy (coefss, c, sizeof (double) * len);

  if (ar->thread_num == ar->total_threads - 1)
    {
      printf ("number of equations: %d\n", len);

        delete [] a;
        delete [] rnz;
        delete [] rhs;
        delete [] c;
        delete [] u;
        delete [] r;
        delete [] v;
    }

  return 0;
}

void MainWindow::recount ()
{
  int i;
  pthread_t tid;

  if (coefss) delete [] coefss;
  if (!(coefss = new double [N]))
    return;

  for (i = 0; i < total_threads; i++)
    {
      ar[i].thread_num = i;
      ar[i].total_threads = total_threads;
      ar[i].hx = hx;
      ar[i].hy = hy;
      ar[i].n = n;
      ar[i].x = x;
      ar[i].y = y;
    }

  if (fabs (hx * hy) < 1e-15)
    {
      printf ("too small interpolation\n");
      return;
    }

  for (i = 1; i < total_threads; i++)
    pthread_create (&tid, 0, main_thread, ar + i);
  main_thread (ar + 0);

  coefs = coefss;
}

int main(int argc, char *argv[])
{
//  double x[4] = {0,0,1,1};
//  double y[4] = {0,1,1,0};
//  int n = 5;
//  printf ("n %d\n", n);
//  int N = 2 * n * (n + 1) + 1;

//  int non_zeros = 14 * n * n + 6 * n + 1;

//  double *a = new double [non_zeros + 1];
//  if (!a)
//    return 1;
//  unsigned int *jnz = new unsigned int [non_zeros + 1];
//  if (!jnz)
//    {
//      delete [] a;
//      return 1;
//    }
//  double *f   = new double [N];
//  double *rhs = new double [N];

//  init_sparse_matrix (a, jnz, f, x, y, n, 0, 1);
//  //init_sparse_matrix (a, jnz, f, x, y, n, 1, 2);
//  print_sparse_matrix (a, jnz, N ,non_zeros);
//  init_rhs_for_test(rhs, N, f, a, jnz, 0, 1);
//  print_vector (f, N);
//  print_vector (rhs, N);

//  delete [] a;
//  delete []jnz;
//  delete [] f;
//  delete [] rhs;
//  return 0;
  QApplication app(argc, argv);
  MainWindow *window = new MainWindow;

  if (window->parce_comand_line (argc, argv))
    {
      printf ("usage: %s <x1> <y1> <x2> <y2> <n> <total_threads>\n", argv[0]);
      delete window;
      return 1;
    }
  window->setGeometry (100, 100, 950, 950);

  window->recount ();
  window->show ();
  app.exec ();

  delete window;
  return 0;
}
