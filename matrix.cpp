#include "mainwindow.h"

void init_sparse_matrix (double *a, unsigned int *jnz, double *f, double *x, double *y, int n, int thread_num, int total_threads)
{
  double hx, hy;
  double curr_x = x[1], curr_y = y[1];
  double int_AA, int_AB;
  int N = 2 * n * (n + 1) + 1;

  if (!a || !jnz || !f)
    return;

  hx = (x[1] - x[0]) / (n);
  hy = (y[1] - y[0]) / (n);

  //  find integral meanings
  {
    int_AA = hx * hy / 12;
    int_AB = hx * hy / 24;
    printf ("int_AA %f\nint_AB %f\n", int_AA, int_AB);
  }

  //input top part
  if (thread_num == 0)
    {
      a [0] = 2 * int_AA;
      jnz[0] = N + 1;
      f [0] = ff(x[1], y[1]);

      a   [N + 1] = int_AB;
      jnz [N + 1] = 1;
      a   [N + 2] = 2 * int_AB;
      jnz [N + 2] = 2;
      a   [N + 3] = int_AB;
      jnz [N + 3] = 3;
    }
  else
    {
      int i = thread_num;
      int k = i * i;
      int m = N + 2 + 6 * i * i - 4 * i; // = (N + 1) + 2 + (2 * 4 * (k - 1) ) + ( (k - 2) * (k - 1) * 6 / 2)
      a   [k] = 3 * int_AA;
      jnz [k] = m;

      curr_x = x[1] - i * hx;
      curr_y = y[1] - i * hy;
      f  [k] = ff(curr_x, curr_y);

      a   [m + 0] = int_AB;
      jnz [m + 0] = k - 2 * i + 1;

      a   [m + 1] = 2 * int_AB;
      jnz [m + 1] = k + 1;

      a   [m + 2] = int_AB;
      jnz [m + 2] = k + 2 * i + 2;

      a   [m + 3] = 2 * int_AB;
      jnz [m + 3] = k + 2 * i + 3;

      m += 4;

      for (int j = 1; j < 2 * i; j++)
        {
          a   [k + j] = 6 * int_AA;
          jnz [k + j] = m;
          f   [k + j] = ff(curr_x + j * hx, curr_y);

          a   [m + 0] = 2 * int_AB;
          jnz [m + 0] = k + j - 2 * i;

          a   [m + 1] = 2 * int_AB;
          jnz [m + 1] = k + j - 2 * i + 1;

          a   [m + 2] = 2 * int_AB;
          jnz [m + 2] = k + j - 1;

          a   [m + 3] = 2 * int_AB;
          jnz [m + 3] = k + j + 1;

          a   [m + 4] = 2 * int_AB;
          jnz [m + 4] = k + j + 2 * i + 2;

          a   [m + 5] = 2 * int_AB;
          jnz [m + 5] = k + j + 2 * i + 3;

          m += 6;
        }

      a   [k + 2 * i] = 2 * int_AA;
      jnz [k + 2 * i] = m;
      f   [k + 2 * i] = ff(curr_x + 2 * i * hx, curr_y);

      a   [m + 0] = int_AB;
      jnz [m + 0] = k - 1;

      a   [m + 1] = 2 * int_AB;
      jnz [m + 1] = k + 2 * i - 1;

      a   [m + 2] = 2 * int_AB;
      jnz [m + 2] = k + 4 * i + 2;

      a   [m + 3] = int_AB;
      jnz [m + 3] = k + 4 * i + 3;
    }

  for (int i = thread_num + total_threads; i < n; i += total_threads)
    {
      int k = i * i;
      int m = N + 2 + 6 * i * i - 4 * i; // = (N + 1) + 2 + (2 * 4 * (k - 1) ) + ( (k - 2) * (k - 1) * 6 / 2)
      a   [k] = 3 * int_AA;
      jnz [k] = m;

      curr_x = x[1] - i * hx;
      curr_y = y[1] - i * hy;
      f  [k] = ff(curr_x, curr_y);

      a   [m + 0] = int_AB;
      jnz [m + 0] = k - 2 * i + 1;

      a   [m + 1] = 2 * int_AB;
      jnz [m + 1] = k + 1;

      a   [m + 2] = int_AB;
      jnz [m + 2] = k + 2 * i + 2;

      a   [m + 3] = 2 * int_AB;
      jnz [m + 3] = k + 2 * i + 3;

      m += 4;

      for (int j = 1; j < 2 * i; j++)
        {
          a   [k + j] = 6 * int_AA;
          jnz [k + j] = m;
          f   [k + j] = ff(curr_x + j * hx, curr_y);

          a   [m + 0] = 2 * int_AB;
          jnz [m + 0] = k + j - 2 * i;

          a   [m + 1] = 2 * int_AB;
          jnz [m + 1] = k + j - 2 * i + 1;

          a   [m + 2] = 2 * int_AB;
          jnz [m + 2] = k + j - 1;

          a   [m + 3] = 2 * int_AB;
          jnz [m + 3] = k + j + 1;

          a   [m + 4] = 2 * int_AB;
          jnz [m + 4] = k + j + 2 * i + 2;

          a   [m + 5] = 2 * int_AB;
          jnz [m + 5] = k + j + 2 * i + 3;

          m += 6;
        }

      a   [k + 2 * i] = 3 * int_AA;
      jnz [k + 2 * i] = m;
      f   [k + 2 * i] = ff(curr_x + 2 * i * hx, curr_y);

      a   [m + 0] = int_AB;
      jnz [m + 0] = k - 1;

      a   [m + 1] = 2 * int_AB;
      jnz [m + 1] = k + 2 * i - 1;

      a   [m + 2] = 2 * int_AB;
      jnz [m + 2] = k + 4 * i + 2;

      a   [m + 3] = int_AB;
      jnz [m + 3] = k + 4 * i + 3;
    }

  //input diagonal
  if (n % total_threads == thread_num)
    {
      int k = n * n;
      int m = N + 2 + 6 * n * n - 4 * n;

      a   [k] = 2 * int_AA;
      jnz [k] = m;
      curr_x = x[0];
      curr_y = y[0];
      f [k] = ff (curr_x, curr_y);

      a   [m + 0] = int_AB;
      jnz [m + 0] = k - 2 * n + 1;

      a   [m + 1] = 2 * int_AB;
      jnz [m + 1] = k + 1;

      a   [m + 2] = int_AB;
      jnz [m + 2] = k + 2 * n + 1;

      m += 3;

      for (int j = 1; j < n; j++)
        {
          a   [k + j] = 6 * int_AA;
          jnz [k + j] = m;
          f   [k + j] = ff (curr_x + hx * j, curr_y);

          a   [m + 0] = 2 * int_AB;
          jnz [m + 0] = k + j - 2 * n;

          a   [m + 1] = 2 * int_AB;
          jnz [m + 1] = k + j - 2 * n + 1;

          a   [m + 2] = 2 * int_AB;
          jnz [m + 2] = k + j - 1;

          a   [m + 3] = 2 * int_AB;
          jnz [m + 3] = k + j + 1;

          a   [m + 4] = 2 * int_AB;
          jnz [m + 4] = k + j + 2 * n;

          a   [m + 5] = 2 * int_AB;
          jnz [m + 5] = k + j + 2 * n + 1;

          m += 6;
        }

      a   [k + n] = 4 * int_AA;
      jnz [k + n] = m;
      f   [k + n] = ff ((x[0] + x[2]) * 0.5, y[0]);

      a   [m + 0] = 2 * int_AB;
      jnz [m + 0] = k - n;

      a   [m + 1] = 2 * int_AB;
      jnz [m + 1] = k + n - 1;

      a   [m + 2] = 2 * int_AB;
      jnz [m + 2] = k + n + 1;

      a   [m + 3] = 2 * int_AB;
      jnz [m + 3] = k + 3 * n;

      m += 4;

      for (int j = n + 1; j < 2 * n; j++)
        {
          a   [k + j] = 6 * int_AA;
          jnz [k + j] = m;
          f   [k + j] = ff (curr_x + hx * j, curr_y);

          a   [m + 0] = 2 * int_AB;
          jnz [m + 0] = k + j - 2 * n;

          a   [m + 1] = 2 * int_AB;
          jnz [m + 1] = k + j - 2 * n + 1;

          a   [m + 2] = 2 * int_AB;
          jnz [m + 2] = k + j - 1;

          a   [m + 3] = 2 * int_AB;
          jnz [m + 3] = k + j + 1;

          a   [m + 4] = 2 * int_AB;
          jnz [m + 4] = k + j + 2 * n;

          a   [m + 5] = 2 * int_AB;
          jnz [m + 5] = k + j + 2 * n + 1;

          m += 6;
        }
      a   [k + 2 * n] = 2 * int_AA;
      jnz [k + 2 * n] = m;
      f   [k + 2 * n] = ff (x[2], y[2]);

      a   [m + 0] = int_AA;
      jnz [m + 0] = k - 1;

      a   [m + 1] = int_AA;
      jnz [m + 1] = k + 2 * n - 1;

      a   [m + 2] = int_AA;
      jnz [m + 2] = k + 4 * n;

    }

  //input bottom part
  int num_of_top_elements = (n + 1) * (n + 1);
  printf ("num_of_top %d \n", num_of_top_elements);
  int non_zeros_top = N - 2 + 6 * (n + 1) * (n + 1) - 4 * (n + 1);
  for (int i = thread_num; i < n - 1; i += total_threads)
    {
      int k = i * (2 * n - i);
      k += num_of_top_elements;

      int count_i = n - i - 1;

      int m = non_zeros_top + i * (12 * n - 4 - 6 * i);

      a   [k] = 3 * int_AA;
      jnz [k] = m;
      curr_x = x[0] + hx * (i + 1);
      curr_y = y[0] - hy * (i + 1);
      f [k] = ff (curr_x, curr_y);

      a   [m + 0] = int_AB;
      jnz [m + 0] = k - (2 * count_i + 1);

      a   [m + 1] = 2 * int_AB;
      jnz [m + 1] = k - 2 * count_i;

      a   [m + 2] = 2 * int_AB;
      jnz [m + 2] = k + 1;

      a   [m + 3] = int_AB;
      jnz [m + 3] = k + 2 * count_i - 1;

      m += 4;

      for (int j = 1; j < 2 * count_i; j++)
        {
          a   [k + j] = 6 * int_AA;
          jnz [k + j] = m;
          f   [k + j] = ff (curr_x + j * hx, curr_y);

          a   [m + 0] = 2 * int_AB;
          jnz [m + 0] = k + j - (2 * count_i + 1);

          a   [m + 1] = 2 * int_AB;
          jnz [m + 1] = k + j - 2 * count_i;

          a   [m + 2] = 2 * int_AB;
          jnz [m + 2] = k + j - 1;

          a   [m + 3] = 2 * int_AB;
          jnz [m + 3] = k + j + 1;

          a   [m + 4] = 2 * int_AB;
          jnz [m + 4] = k + j + 2 * count_i - 2;

          a   [m + 5] = 2 * int_AB;
          jnz [m + 5] = k + j + 2 * count_i - 1;

          m += 6;
        }
      int j = 2 * count_i;
      a   [k + j] = 3 * int_AA;
      jnz [k + j] = m;
      f   [k + j] = ff (curr_x + hx * j, curr_y);

      a   [m + 0] = int_AB;
      jnz [m + 0] = k + j - (2 * count_i + 1);

      a   [m + 1] = 2 * int_AB;
      jnz [m + 1] = k + j - 2 * count_i;

      a   [m + 2] = 2 * int_AB;
      jnz [m + 2] = k + j - 1;

      a   [m + 3] = int_AB;
      jnz [m + 3] = k + j + 2 * count_i - 2;
    }
  if (thread_num == 0)
    {
      int i = n - 1;
      int k = i * (2 * n - i);
      k += num_of_top_elements;

      int m = non_zeros_top + i * (12 * n - 4 - 6 * i);
      a  [N - 1] = int_AA;
      jnz[N - 1] = m;
      f  [N - 1] = ff (x[3], y[0] - hy * n);

      a  [m + 0] = int_AB;
      jnz[m + 0] = k - 3;
      a  [m + 1] = 2 * int_AB;
      jnz[m + 1] = k - 2;
      a  [m + 2] = int_AB;
      jnz[m + 2] = k - 1;
    }
  jnz [N] = jnz [N - 1] + 3;
  reduce_sum <int> (total_threads);
}

void init_sparse_matrix_rhs (double *a, unsigned int *jnz, double *f, double *x, double *y, int n, int thread_num, int total_threads)
{
  double hx, hy;
  double curr_x = x[1], curr_y = y[1];
  double int_AA, int_AB;
  int N = 2 * n * (n + 1) + 1;

  if (!a || !jnz || !f)
    return;

  hx = (x[1] - x[0]) / (n);
  hy = (y[1] - y[0]) / (n);

  //  find integral meanings
  {
    int_AA = hx * hy / 12;
    int_AB = hx * hy / 24;
    printf ("int_AA %f\nint_AB %f\n", int_AA, int_AB);
  }

  //input top part
  if (thread_num == 0)
    {
      a [0] = 2 * int_AA;
      jnz[0] = N + 1;
      f [0] = ff(x[1], y[1]);

      a   [N + 1] = int_AB;
      jnz [N + 1] = 1;
      a   [N + 2] = 2 * int_AB;
      jnz [N + 2] = 2;
      a   [N + 3] = int_AB;
      jnz [N + 3] = 3;
    }
  else
    {
      int i = thread_num;
      int k = i * i;
      int m = N + 2 + 6 * i * i - 4 * i; // = (N + 1) + 2 + (2 * 4 * (k - 1) ) + ( (k - 2) * (k - 1) * 6 / 2)
      a   [k] = 3 * int_AA;
      jnz [k] = m;

      curr_x = x[1] - i * hx;
      curr_y = y[1] - i * hy;
      f  [k] = ff(curr_x, curr_y);

      a   [m + 0] = int_AB;
      jnz [m + 0] = k - 2 * i + 1;

      a   [m + 1] = 2 * int_AB;
      jnz [m + 1] = k + 1;

      a   [m + 2] = int_AB;
      jnz [m + 2] = k + 2 * i + 2;

      a   [m + 3] = 2 * int_AB;
      jnz [m + 3] = k + 2 * i + 3;

      m += 4;

      for (int j = 1; j < 2 * i; j++)
        {
          a   [k + j] = 6 * int_AA;
          jnz [k + j] = m;
          f   [k + j] = ff(curr_x + j * hx, curr_y);

          a   [m + 0] = 2 * int_AB;
          jnz [m + 0] = k + j - 2 * i;

          a   [m + 1] = 2 * int_AB;
          jnz [m + 1] = k + j - 2 * i + 1;

          a   [m + 2] = 2 * int_AB;
          jnz [m + 2] = k + j - 1;

          a   [m + 3] = 2 * int_AB;
          jnz [m + 3] = k + j + 1;

          a   [m + 4] = 2 * int_AB;
          jnz [m + 4] = k + j + 2 * i + 2;

          a   [m + 5] = 2 * int_AB;
          jnz [m + 5] = k + j + 2 * i + 3;

          m += 6;
        }

      a   [k + 2 * i] = 2 * int_AA;
      jnz [k + 2 * i] = m;
      f   [k + 2 * i] = ff(curr_x + 2 * i * hx, curr_y);

      a   [m + 0] = int_AB;
      jnz [m + 0] = k - 1;

      a   [m + 1] = 2 * int_AB;
      jnz [m + 1] = k + 2 * i - 1;

      a   [m + 2] = 2 * int_AB;
      jnz [m + 2] = k + 4 * i + 2;

      a   [m + 3] = int_AB;
      jnz [m + 3] = k + 4 * i + 3;
    }

  for (int i = thread_num + total_threads; i < n; i += total_threads)
    {
      int k = i * i;
      int m = N + 2 + 6 * i * i - 4 * i; // = (N + 1) + 2 + (2 * 4 * (k - 1) ) + ( (k - 2) * (k - 1) * 6 / 2)
      a   [k] = 3 * int_AA;
      jnz [k] = m;

      curr_x = x[1] - i * hx;
      curr_y = y[1] - i * hy;
      f  [k] = ff(curr_x, curr_y);

      a   [m + 0] = int_AB;
      jnz [m + 0] = k - 2 * i + 1;

      a   [m + 1] = 2 * int_AB;
      jnz [m + 1] = k + 1;

      a   [m + 2] = int_AB;
      jnz [m + 2] = k + 2 * i + 2;

      a   [m + 3] = 2 * int_AB;
      jnz [m + 3] = k + 2 * i + 3;

      m += 4;

      for (int j = 1; j < 2 * i; j++)
        {
          a   [k + j] = 6 * int_AA;
          jnz [k + j] = m;
          f   [k + j] = ff(curr_x + j * hx, curr_y);

          a   [m + 0] = 2 * int_AB;
          jnz [m + 0] = k + j - 2 * i;

          a   [m + 1] = 2 * int_AB;
          jnz [m + 1] = k + j - 2 * i + 1;

          a   [m + 2] = 2 * int_AB;
          jnz [m + 2] = k + j - 1;

          a   [m + 3] = 2 * int_AB;
          jnz [m + 3] = k + j + 1;

          a   [m + 4] = 2 * int_AB;
          jnz [m + 4] = k + j + 2 * i + 2;

          a   [m + 5] = 2 * int_AB;
          jnz [m + 5] = k + j + 2 * i + 3;

          m += 6;
        }

      a   [k + 2 * i] = 3 * int_AA;
      jnz [k + 2 * i] = m;
      f   [k + 2 * i] = ff(curr_x + 2 * i * hx, curr_y);

      a   [m + 0] = int_AB;
      jnz [m + 0] = k - 1;

      a   [m + 1] = 2 * int_AB;
      jnz [m + 1] = k + 2 * i - 1;

      a   [m + 2] = 2 * int_AB;
      jnz [m + 2] = k + 4 * i + 2;

      a   [m + 3] = int_AB;
      jnz [m + 3] = k + 4 * i + 3;
    }

  //input diagonal
  if (n % total_threads == thread_num)
    {
      int k = n * n;
      int m = N + 2 + 6 * n * n - 4 * n;

      a   [k] = 2 * int_AA;
      jnz [k] = m;
      curr_x = x[0];
      curr_y = y[0];
      f [k] = ff (curr_x, curr_y);

      a   [m + 0] = int_AB;
      jnz [m + 0] = k - 2 * n + 1;

      a   [m + 1] = 2 * int_AB;
      jnz [m + 1] = k + 1;

      a   [m + 2] = int_AB;
      jnz [m + 2] = k + 2 * n + 1;

      m += 3;

      for (int j = 1; j < n; j++)
        {
          a   [k + j] = 6 * int_AA;
          jnz [k + j] = m;
          f   [k + j] = ff (curr_x + hx * j, curr_y);

          a   [m + 0] = 2 * int_AB;
          jnz [m + 0] = k + j - 2 * n;

          a   [m + 1] = 2 * int_AB;
          jnz [m + 1] = k + j - 2 * n + 1;

          a   [m + 2] = 2 * int_AB;
          jnz [m + 2] = k + j - 1;

          a   [m + 3] = 2 * int_AB;
          jnz [m + 3] = k + j + 1;

          a   [m + 4] = 2 * int_AB;
          jnz [m + 4] = k + j + 2 * n;

          a   [m + 5] = 2 * int_AB;
          jnz [m + 5] = k + j + 2 * n + 1;

          m += 6;
        }

      a   [k + n] = 4 * int_AA;
      jnz [k + n] = m;
      f   [k + n] = ff ((x[0] + x[2]) * 0.5, (y[0] + y[2]) * 0.5);

      a   [m + 0] = 2 * int_AB;
      jnz [m + 0] = k - n;

      a   [m + 1] = 2 * int_AB;
      jnz [m + 1] = k + n - 1;

      a   [m + 2] = 2 * int_AB;
      jnz [m + 2] = k + n + 1;

      a   [m + 3] = 2 * int_AB;
      jnz [m + 3] = k + 3 * n;

      m += 4;

      for (int j = n + 1; j < 2 * n; j++)
        {
          a   [k + j] = 6 * int_AA;
          jnz [k + j] = m;
          f   [k + j] = ff (curr_x + hx * j, curr_y);

          a   [m + 0] = 2 * int_AB;
          jnz [m + 0] = k + j - 2 * n;

          a   [m + 1] = 2 * int_AB;
          jnz [m + 1] = k + j - 2 * n + 1;

          a   [m + 2] = 2 * int_AB;
          jnz [m + 2] = k + j - 1;

          a   [m + 3] = 2 * int_AB;
          jnz [m + 3] = k + j + 1;

          a   [m + 4] = 2 * int_AB;
          jnz [m + 4] = k + j + 2 * n;

          a   [m + 5] = 2 * int_AB;
          jnz [m + 5] = k + j + 2 * n + 1;

          m += 6;
        }
      a   [k + 2 * n] = 2 * int_AA;
      jnz [k + 2 * n] = m;
      f   [k + 2 * n] = ff (x[2], y[2]);

      a   [m + 0] = int_AA;
      jnz [m + 0] = k - 1;

      a   [m + 1] = int_AA;
      jnz [m + 1] = k + 2 * n - 1;

      a   [m + 2] = int_AA;
      jnz [m + 2] = k + 4 * n;

    }

  //input bottom part
  int num_of_top_elements = (n + 1) * (n + 1);
  printf ("num_of_top %d \n", num_of_top_elements);
  int non_zeros_top = N - 2 + 6 * (n + 1) * (n + 1) - 4 * (n + 1);
  for (int i = thread_num; i < n - 1; i += total_threads)
    {
      int k = i * (2 * n - i);
      k += num_of_top_elements;

      int count_i = n - i - 1;

      int m = non_zeros_top + i * (12 * n - 4 - 6 * i);

      a   [k] = 3 * int_AA;
      jnz [k] = m;
      curr_x = x[0] + hx * (i + 1);
      curr_y = y[0] - hy * (i + 1);
      f [k] = ff (curr_x, curr_y);

      a   [m + 0] = int_AB;
      jnz [m + 0] = k - (2 * count_i + 1);

      a   [m + 1] = 2 * int_AB;
      jnz [m + 1] = k - 2 * count_i;

      a   [m + 2] = 2 * int_AB;
      jnz [m + 2] = k + 1;

      a   [m + 3] = int_AB;
      jnz [m + 3] = k + 2 * count_i - 1;

      m += 4;

      for (int j = 1; j < 2 * count_i; j++)
        {
          a   [k + j] = 6 * int_AA;
          jnz [k + j] = m;
          f   [k + j] = ff (curr_x + j * hx, curr_y);

          a   [m + 0] = 2 * int_AB;
          jnz [m + 0] = k + j - (2 * count_i + 1);

          a   [m + 1] = 2 * int_AB;
          jnz [m + 1] = k + j - 2 * count_i;

          a   [m + 2] = 2 * int_AB;
          jnz [m + 2] = k + j - 1;

          a   [m + 3] = 2 * int_AB;
          jnz [m + 3] = k + j + 1;

          a   [m + 4] = 2 * int_AB;
          jnz [m + 4] = k + j + 2 * count_i - 2;

          a   [m + 5] = 2 * int_AB;
          jnz [m + 5] = k + j + 2 * count_i - 1;

          m += 6;
        }
      int j = 2 * count_i;
      a   [k + j] = 3 * int_AA;
      jnz [k + j] = m;
      f   [k + j] = ff (curr_x + hx * j, curr_y);

      a   [m + 0] = int_AB;
      jnz [m + 0] = k + j - (2 * count_i + 1);

      a   [m + 1] = 2 * int_AB;
      jnz [m + 1] = k + j - 2 * count_i;

      a   [m + 2] = 2 * int_AB;
      jnz [m + 2] = k + j - 1;

      a   [m + 3] = int_AB;
      jnz [m + 3] = k + j + 2 * count_i - 2;
    }
  if (thread_num == 0)
    {
      int i = n - 1;
      int k = i * (2 * n - i);
      k += num_of_top_elements;

      int m = non_zeros_top + i * (12 * n - 4 - 6 * i);
      a  [N - 1] = int_AA;
      jnz[N - 1] = m;
      f  [N - 1] = ff (x[3], y[3]);

      a  [m + 0] = int_AB;
      jnz[m + 0] = k - 3;
      a  [m + 1] = 2 * int_AB;
      jnz[m + 1] = k - 2;
      a  [m + 2] = int_AB;
      jnz[m + 2] = k - 1;
    }
  jnz [N] = jnz [N - 1] + 3;
  reduce_sum <int> (total_threads);
}

void init_rhs_for_test (double *rhs, int N, double *f, double *a, unsigned int *jnz, int thread_num, int total_threads)
{
  matr_mult (a, jnz, N, f, rhs, thread_num, total_threads);
}

void print_sparse_matrix (double *a, unsigned int *jnz, int N, int non_zeros)
{
  printf ("non_zeros %d\n", non_zeros);
  for (int i = 0; i < N; i++)
    printf ("#%d [%f %d] \n", i, a[i], jnz[i]);
  printf("\n");
  for (int i = N + 1; i < non_zeros + 1; i++)
    printf ("#%d [%f %d] \n", i, a[i], jnz[i]);
}

void matr_mult (double *a, unsigned int *rnz, int n, double *x, double *b, int k, int p)
{
  int i1, i2, i, start, len, j, jj;
  double s;

  get_my_rows (n, k, p, &i1, &i2);
  for (i = i1; i <= i2; i++)
    {
      s = a[i] * x[i];
      start = rnz[i];
      len = rnz[i + 1] - rnz[i];

      for (j = 0; j < len; j++)
        {
          jj = start + j;
          s += a[jj] * x[rnz[jj]];
        }

      b[i] = s;
    }

  reduce_sum <int>(p);
}

void get_my_rows (int n, int k, int p, int *i1, int *i2)
{
  *i1 = n * k / p;
  *i2 = n * (k + 1) / p - 1;
}

void print_vector(double *a, int N)
{
  for (int i = 0; i < N; i++)
    printf ("#%d [%f]\n", i, a[i]);
  printf ("\n");
}

double get_value (int i, int j, double *coefs, int n)
{
  int k;
  if (i < n + 1)
    {
      k = i * i + j;
    }
  else
    {
      k = (n + 1) * (n + 1) + (i - n - 1) * (3 * n - i + 1) + j;
    }
 // printf ("#%d  i %d j %d %f\n", k, i, j, coefs[k]);
  return coefs [k];
}
