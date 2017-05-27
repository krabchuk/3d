#include "mainwindow.h"

double integrate_FA_FA (double hx, double hy)
{
  double s = hx * hy / 2;
  return s / 6;
}

double integrate_FA_FB (double hx, double hy)
{
  double s = hx * hy / 2;
  return s / 12;
}

double integrate_FA_FC (double hx, double hy)
{
  double s = hx * hy / 2;
  return s / 12;
}

double integrate_FB_FB (double hx, double hy)
{
  double s = hx * hy / 2;
  return s / 6;
}

double integrate_FB_FC (double hx, double hy)
{
  double s = hx * hy / 2;
  return s / 12;
}

double integrate_FC_FC (double hx, double hy)
{
  double s = hx * hy / 2;
  return s / 6;
}
