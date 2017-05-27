#include "mainwindow.h"

// u = u - tau * v
void sub_vect (int n, double *u, double tau, double *v, int k, int p)
{
  int i1, i2, i;

  get_my_rows (n, k, p, &i1, &i2);
  for (i = i1; i <= i2; i++)
    u[i] -= tau * v[i];

  reduce_sum <int>(p);
}

double scalp (double *x, double *y, int n, int k, int p)
{
  int i1, i2, i;
  double s = 0;

  get_my_rows (n, k, p, &i1, &i2);
  for (i = i1; i <= i2; i++)
    s += x[i] * y[i];

  reduce_sum <double>(p, &s, 1);
  return s;
}

// D * x = y => x = D^(-1) * y
void preconditioner (double *d, int n, double *y, double *x, int k, int p)
{
  int i1, i2, i;

  get_my_rows (n, k, p, &i1, &i2);
  for (i = i1; i <= i2; i++)
    x[i] = y[i] / d[i];

  reduce_sum <int>(p);
}

double residual_for_test (double *x, int nx, int ny, int nx_cut, int ny_cut, double hx, double hy, int k, int p)
{
  int i, j, i1, i2, border;
  double *x_ptr, sum = 0, c, xp, yp;

  get_my_rows (ny, k, p, &i1, &i2);
  if (i1 < ny_cut)
    x_ptr = x + i1 * (nx - nx_cut + 1);
  else
    x_ptr = x + (nx - nx_cut + 1) * ny_cut + nx_cut - 1 + (i1 - ny_cut) * nx;

  for (i = i1; i <= i2; i++)
    {
      border = i < ny_cut - 1 ? nx - nx_cut + 1 : nx;
      for (j = 0; j < border; j++)
        {
          xp = j * hx, yp = (ny - i - 1) * hy;
          //inverse_change_coordinates (&xp, &yp);
          c = fabs (*x_ptr++ - ff(xp, yp));
          sum += c * c;
        }
    }

  reduce_sum <double>(p, &sum, 1);
  return sqrt (sum);
}

int solve (double *a, unsigned int *rnz, int n, double *b, double *x, double eps, int maxit, double *r, double *u, double *v, int k, int p)
{
  double c1, c2, tau;
  int it;

  if (!a || !rnz || !b)
    return -1;

  // r = Ax - b => r = Ax, r -= 1 * b
  matr_mult (a, rnz, n, x, r, k, p);
  sub_vect (n, r, 1, b, k, p);
  c1 = scalp (r, r, n, k, p);

  if (fabs (sqrt(c1)) < eps)
    return 0;

  for (it = 1; it < maxit; it++)
    {
      // u = D^(-1) * r
      preconditioner (a, n, r, u, k, p);

      // v = A * u
      matr_mult (a, rnz, n, u, v, k, p);

      c1 = scalp (v, r, n, k, p);
      c2 = scalp (v, v, n, k, p);

      if (fabs (sqrt(c1)) < eps || fabs (sqrt(c2)) < eps)
        return it;

      tau = c1 / c2;

      sub_vect (n, r, tau, v, k, p);
      sub_vect (n, x, tau, u, k, p);

      if (k == 0)
        printf ("it = %d, residual = %e\n", it, fabs (sqrt(c2)));
    }

  return maxit;
}

int get_offset (int i, int j, int nx, int nx_cut, int ny_cut)
{
  if (i < ny_cut)
    return i * (nx - nx_cut + 1) + j;
  return (nx - nx_cut + 1) * ny_cut + nx_cut - 1 + (i - ny_cut) * nx + j;
}

double get_value (double xp, double yp, double *c, int nx, int ny, int nx_cut, int ny_cut, double hx, double hy)
{
  int i1, j1, i2, j2, i3, j3, i, j;
  double sum = 0;

  //change_coordinates (&xp, &yp);

  j = (int) (xp / hx);
  i = (int) (yp / hy);

  if (i > ny - 1 || i < 0 || (i > ny - ny_cut && j > nx - nx_cut) || j > nx - 1)
    return 0;

  if (fabs (yp - i * hy) < 1e-15)
    {
      i--;
      if (i < 0)
        i = 0;
    }
  if (fabs (xp - j * hx) < 1e-15)
    {
      j--;
      if (j < 0)
        j = 0;
    }

  i1 = i, j1 = j;
  i3 = i + 1, j3 = j + 1;
  if (xp - j * hx > yp - i * hy)
    {
      i2 = i, j2 = j + 1;
      sum += c[get_offset (ny - i1 - 1, j1, nx, nx_cut, ny_cut)] * (1 + j1 - xp / hx);
      sum += c[get_offset (ny - i2 - 1, j2, nx, nx_cut, ny_cut)] * (i2 - j2 + 1 - yp / hy + xp / hx);
      sum += c[get_offset (ny - i3 - 1, j3, nx, nx_cut, ny_cut)] * (1 - i3 + yp / hy);
    }
  else
    {
      i2 = i + 1, j2 = j;
      sum += c[get_offset (ny - i1 - 1, j1, nx, nx_cut, ny_cut)] * (1 + i1 - yp / hy);
      sum += c[get_offset (ny - i2 - 1, j2, nx, nx_cut, ny_cut)] * (j2 - i2 + 1 + yp / hy - xp / hx);
      sum += c[get_offset (ny - i3 - 1, j3, nx, nx_cut, ny_cut)] * (1 - j3 + xp / hx);
    }

  return sum;
}
