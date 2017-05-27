#include "mainwindow.h"
#include "GL/glu.h"

double ff (double x, double y)
{
  return exp(-x * x - y * y);
}

MainWindow::MainWindow(QWidget* parent) : QGLWidget(parent)
{
  setFormat(QGLFormat(QGL::DoubleBuffer));
  glDepthFunc(GL_LEQUAL);
  w = 800, h = 800;
  xRot = zRot = 0;
  is_pressed = false;
  scale = 1;
  ar = 0;
  coefs = 0;
  draw_type = 0;
}

MainWindow::~MainWindow()
{
  if (coefs) delete [] coefs;
  if (ar) delete [] ar;
}

int MainWindow::parce_comand_line (int argc, char *argv[])
{
  double x1, y1, x2, y2;
  if (argc != 7 ||
      !( n = atoi (argv[5])) ||
      !(total_threads = atoi (argv[6])))
    return 1;
  x1 = atof (argv[1]);
  y1 = atof (argv[2]);
  x2 = atof (argv[3]);
  y2 = atof (argv[4]);


  x[0] = x1;
  y[0] = y1;

  x[1] = x2;
  y[1] = y2;

  hx = x2 - x1;
  hy = y2 - y1;

  N = 2 * n * (n + 1) + 1;

  x[2] = x[0] + 2 * hx;
  y[2] = y[0];

  x[3] = x[1];
  y[3] = y[1] + 2 * hy;

  hx /= n;
  hy /= n;

  ar = new struct args [total_threads];

  return 0;
}




void MainWindow::paintGL()
{
  char buf[250];
  QFont font ("Arial", 12, QFont::Bold, false);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt (0, 5, 30, 0, 0, 0, 0, 1, 0);
  glRotated (xRot, 1, 0, 0);
  glRotated (zRot, 0, 0, 1);
  glScaled (scale, scale, scale);

  drawAxis ();
  switch (draw_type)
    {
    case 0:
      drawInterpolation ();
      qglColor(Qt::black);
      renderText (10, 15, QString ("interpolation plot (space to change)"), font);
      break;
    case 1:
      drawFunction ();
      drawInterpolation ();
      qglColor(Qt::black);
      renderText (10, 15, QString ("interpolation and function plots (space to change)"), font);
      break;
    case 2:
      drawThreshold ();
      qglColor(Qt::black);
      renderText (10, 15, QString ("residual plot (space to change)"), font);
      break;
    }

  sprintf (buf, "max absolute value: %.3e, hx = %.2e, hy = %.2e, n = %d (+/- to change)", zmax, hx, hy, n);
  setWindowTitle (QString (buf));
  glFlush ();
  swapBuffers ();
}

void MainWindow::mouseMoveEvent(QMouseEvent *me)
{
  if (is_pressed)
    {
      xRot += 180 * (GLfloat) (me->y () - mouse_position.y ()) / h;
      zRot += 180 * (GLfloat) (me->x () - mouse_position.x ()) / w;
      if (fabs (xRot) < 1e-14)
        xRot = 0;
      if (fabs (zRot) < 1e-14)
        zRot = 0;
      updateGL ();
    }
  mouse_position = me->pos ();
}

void MainWindow::mousePressEvent(QMouseEvent *me)
{
  if(me->button () == Qt::LeftButton)
    {
      is_pressed = true;
      mouse_position = me->pos ();
      updateGL ();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent */*me*/)
{
  is_pressed = false;
  updateGL ();
}

void MainWindow::wheelEvent (QWheelEvent *we)
{
  if (we->delta () > 0)
    scale *= 1.1;
  else if (we->delta () < 0)
    scale /= 1.1;
  updateGL ();
}

void MainWindow::resizeGL(int nWidth, int nHeight)
{
  glViewport(0, 0, w = nWidth, h = nHeight);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60, (double)w / h, 1, 500);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(0, 0, 30, 0, 0, 0, 0, 1, 0);
}

void MainWindow::drawAxis ()
{
  glBegin (GL_LINES);
  glColor3d (1, 0, 0);
  glVertex4d (0, 0, 0, 1);
  glVertex4d (1, 0, 0, 0);

  glColor3d (0, 1, 0);
  glVertex4d (0, 0, 0, 1);
  glVertex4d (0, 1, 0, 0);

  glColor3d (0, 0, 1);
  glVertex4d (0, 0, 0, 1);
  glVertex4d (0, 0, 1, 0);
  glEnd ();
}

void MainWindow::keyPressEvent(QKeyEvent *ke)
{
  switch (ke->key ())
    {
    case Qt::Key_Escape:
      close ();
      return;
    case Qt::Key_Plus:
      n = 2 * n;
      hx /= 2;
      hy /= 2;
      is_pressed = false;
      N = 2 * n * (n + 1) + 1;
      recount ();
      break;
    case Qt::Key_Minus:
      if (n <= 4)
        break;
      n = n / 2;
      N = 2 * n * (n + 1) + 1;
      hx *= 2;
      hy *= 2;
      is_pressed = false;
      recount ();
      break;
    case Qt::Key_Space:
      draw_type++;
      draw_type %= 3;
      break;
    default:
      break;
    }
  updateGL ();
}

void MainWindow::drawFunction ()
{
  int i, j;
  double x1, x2, x3, x4;
  double y1, y2, y3, y4;
  double z1, z2, z3, z4;
  double curr_x, curr_y;
  zmax = 0;
  glColor3d (0, 255, 0);
  for (i = 0; i < n; i++)
    {
      curr_x = x[1] - i * hx;
      curr_y = y[1] - i * hy;
      for (j = 0; j < i; j++)
        {
          x1 = curr_x + j * hx, y1 = curr_y, z1 = ff (x1, y1);
          x2 = curr_x + (j + 1) * hx, y2 = curr_y, z2 = ff(x2,y2);
          x3 = curr_x + (j - 1) * hx, y3 = curr_y - hy; z3 = ff(x3, y3);
          x4 = curr_x + j * hx, y4 = curr_y - hy, z4 = ff(x4, y4);

          glBegin (GL_TRIANGLES);
          glVertex3d (x1, y1, z1);
          glVertex3d (x2, y2, z2);
          glVertex3d (x4, y4, z4);


          glVertex3d (x1, y1, z1);
          glVertex3d (x3, y3, z3);
          glVertex3d (x4, y4, z4);
          glEnd ();
          if (fabs (z1) > zmax) zmax = fabs (z1);
          if (fabs (z2) > zmax) zmax = fabs (z2);
          if (fabs (z3) > zmax) zmax = fabs (z3);
          if (fabs (z4) > zmax) zmax = fabs (z4);
        }
      j = i;
      x1 = curr_x + j * hx, y1 = curr_y, z1 = ff (x1, y1);
      x2 = curr_x + (j - 1) * hx, y2 = curr_y - hy, z2 = ff(x2,y2);
      x3 = curr_x + (j + 0) * hx, y3 = curr_y - hy; z3 = ff(x3, y3);
      x4 = curr_x + (j + 1) * hx, y4 = curr_y - hy, z4 = ff(x4, y4);

      glBegin (GL_TRIANGLES);
      glVertex3d (x1, y1, z1);
      glVertex3d (x2, y2, z2);
      glVertex3d (x3, y3, z3);


      glVertex3d (x1, y1, z1);
      glVertex3d (x3, y3, z3);
      glVertex3d (x4, y4, z4);
      glEnd ();

      if (fabs (z1) > zmax) zmax = fabs (z1);
      if (fabs (z2) > zmax) zmax = fabs (z2);
      if (fabs (z2) > zmax) zmax = fabs (z3);
      if (fabs (z4) > zmax) zmax = fabs (z4);

      for (j = i + 1; j < 2 * i + 1; j++)
        {
          x1 = curr_x + (j - 1) * hx, y1 = curr_y, z1 = ff (x1, y1);
          x2 = curr_x + j * hx, y2 = curr_y, z2 = ff(x2,y2);
          x3 = curr_x + j * hx, y3 = curr_y - hy, z3 = ff(x3, y3);
          x4 = curr_x + (j + 1) * hx, y4 = curr_y - hy, z4 = ff (x4, y4);

          glBegin (GL_TRIANGLES);
          glVertex3d (x1, y1, z1);
          glVertex3d (x2, y2, z2);
          glVertex3d (x3, y3, z3);


          glVertex3d (x2, y2, z2);
          glVertex3d (x3, y3, z3);
          glVertex3d (x4, y4, z4);
          glEnd ();
          if (fabs (z1) > zmax) zmax = fabs (z1);
          if (fabs (z2) > zmax) zmax = fabs (z2);
          if (fabs (z3) > zmax) zmax = fabs (z3);
          if (fabs (z4) > zmax) zmax = fabs (z4);
        }
    }

  for (i = n; i > 0; i--)
    {
      curr_x = x[0] + (n - i) * hx;
      curr_y = y[0] - (n - i) * hy;
      for (j = 1; j < i; j++)
        {
          x1 = curr_x + (j - 1) * hx, y1 = curr_y, z1 = ff (x1, y1);
          x2 = curr_x + (j + 0) * hx, y2 = curr_y, z2 = ff(x2,y2);
          x3 = curr_x + (j + 0) * hx, y3 = curr_y - hy, z3 = ff(x3, y3);
          x4 = curr_x + (j + 1) * hx, y4 = curr_y - hy, z4 = ff(x4, y4);

          glBegin (GL_TRIANGLES);
          glVertex3d (x1, y1, z1);
          glVertex3d (x2, y2, z2);
          glVertex3d (x3, y3, z3);


          glVertex3d (x2, y2, z2);
          glVertex3d (x3, y3, z3);
          glVertex3d (x4, y4, z4);
          glEnd ();
          if (fabs (z1) > zmax) zmax = fabs (z1);
          if (fabs (z2) > zmax) zmax = fabs (z2);
          if (fabs (z3) > zmax) zmax = fabs (z3);
          if (fabs (z4) > zmax) zmax = fabs (z4);
        }
      j = i;
      x1 = curr_x + (j - 1) * hx, y1 = curr_y, z1 = ff (x1, y1);
      x2 = curr_x + (j + 0) * hx, y2 = curr_y, z2 = ff(x2,y2);
      x3 = curr_x + (j + 1) * hx, y3 = curr_y, z3 = ff(x3, y3);
      x4 = curr_x + (j + 0) * hx, y4 = curr_y - hy, z4 = ff(x4, y4);

      glBegin (GL_TRIANGLES);
      glVertex3d (x1, y1, z1);
      glVertex3d (x2, y2, z2);
      glVertex3d (x4, y4, z4);


      glVertex3d (x2, y2, z2);
      glVertex3d (x3, y3, z3);
      glVertex3d (x4, y4, z4);
      glEnd ();
      if (fabs (z1) > zmax) zmax = fabs (z1);
      if (fabs (z2) > zmax) zmax = fabs (z2);
      if (fabs (z3) > zmax) zmax = fabs (z3);
      if (fabs (z4) > zmax) zmax = fabs (z4);

      for (j = i + 1; j < 2 * i; j++)
        {
          x1 = curr_x + (j + 0) * hx, y1 = curr_y, z1 = ff (x1, y1);
          x2 = curr_x + (j + 1) * hx, y2 = curr_y, z2 = ff (x2, y2);
          x3 = curr_x + (j - 1) * hx, y3 = curr_y - hy, z3 = ff (x3, y3);
          x4 = curr_x + (j + 0) * hx, y4 = curr_y - hy, z4 = ff (x4, y4);

          glBegin (GL_TRIANGLES);
          glVertex3d (x1, y1, z1);
          glVertex3d (x3, y3, z3);
          glVertex3d (x4, y4, z4);


          glVertex3d (x1, y1, z1);
          glVertex3d (x2, y2, z2);
          glVertex3d (x4, y4, z4);
          glEnd ();
          if (fabs (z1) > zmax) zmax = fabs (z1);
          if (fabs (z2) > zmax) zmax = fabs (z2);
          if (fabs (z3) > zmax) zmax = fabs (z3);
          if (fabs (z4) > zmax) zmax = fabs (z4);
        }
    }
}

void MainWindow::drawInterpolation()
{
  int i, j;
  double x1, x2, x3, x4;
  double y1, y2, y3, y4;
  double z1, z2, z3, z4;
  double curr_x, curr_y;
  zmax = 0;
  glColor3d (255, 0, 0);
  for (i = 0; i < n; i++)
    {
      curr_x = x[1] - i * hx;
      curr_y = y[1] - i * hy;
      for (j = 0; j < i; j++)
        {
          x1 = curr_x + j * hx, y1 = curr_y, z1 = get_value (i, j, coefs, n);
          //printf ("x = %f, y = %f\n", x1, y1);
          x2 = curr_x + (j + 1) * hx, y2 = curr_y, z2 = get_value (i, j + 1, coefs, n);
         // printf ("x = %f, y = %f\n", x2, y2);
          x3 = curr_x + (j - 1) * hx, y3 = curr_y - hy; z3 = get_value (i + 1, j, coefs, n);
         // printf ("x = %f, y = %f\n", x3, y3);
          x4 = curr_x + j * hx, y4 = curr_y - hy, z4 = get_value (i + 1, j + 1, coefs, n);
         // printf ("x = %f, y = %f\n", x4, y4);


          glBegin (GL_TRIANGLES);
          glVertex3d (x1, y1, z1);
          glVertex3d (x2, y2, z2);
          glVertex3d (x4, y4, z4);


          glVertex3d (x1, y1, z1);
          glVertex3d (x3, y3, z3);
          glVertex3d (x4, y4, z4);
          glEnd ();
          if (fabs (z1) > zmax) zmax = fabs (z1);
          if (fabs (z2) > zmax) zmax = fabs (z2);
          if (fabs (z3) > zmax) zmax = fabs (z3);
          if (fabs (z4) > zmax) zmax = fabs (z4);
        }
      j = i;
      x1 = curr_x + j * hx, y1 = curr_y, z1 = get_value (i, j, coefs, n);
//      printf ("x = %f, y = %f\n", x1, y1);
      x2 = curr_x + (j - 1) * hx, y2 = curr_y - hy, z2 = get_value (i + 1, j - 0, coefs, n);
//      printf ("x = %f, y = %f\n", x2, y2);
      x3 = curr_x + (j + 0) * hx, y3 = curr_y - hy; z3 = get_value (i + 1, j + 1, coefs, n);
//      printf ("x = %f, y = %f\n", x3, y3);
      x4 = curr_x + (j + 1) * hx, y4 = curr_y - hy, z4 = get_value (i + 1, j + 2, coefs, n);
//      printf ("x = %f, y = %f\n", x4, y4);

      glBegin (GL_TRIANGLES);
      glVertex3d (x1, y1, z1);
      glVertex3d (x2, y2, z2);
      glVertex3d (x3, y3, z3);


      glVertex3d (x1, y1, z1);
      glVertex3d (x3, y3, z3);
      glVertex3d (x4, y4, z4);
      glEnd ();

      if (fabs (z1) > zmax) zmax = fabs (z1);
      if (fabs (z2) > zmax) zmax = fabs (z2);
      if (fabs (z2) > zmax) zmax = fabs (z3);
      if (fabs (z4) > zmax) zmax = fabs (z4);

      for (j = i + 1; j < 2 * i + 1; j++)
        {
          x1 = curr_x + (j - 1) * hx, y1 = curr_y, z1 = get_value (i, j - 1, coefs, n);
          x2 = curr_x + j * hx, y2 = curr_y, z2 = get_value (i, j, coefs, n);
          x3 = curr_x + j * hx, y3 = curr_y - hy, z3 = get_value (i + 1, j + 1, coefs, n);
          x4 = curr_x + (j + 1) * hx, y4 = curr_y - hy, z4 = get_value (i + 1, j + 2, coefs, n);

          glBegin (GL_TRIANGLES);
          glVertex3d (x1, y1, z1);
          glVertex3d (x2, y2, z2);
          glVertex3d (x3, y3, z3);


          glVertex3d (x2, y2, z2);
          glVertex3d (x3, y3, z3);
          glVertex3d (x4, y4, z4);
          glEnd ();
          if (fabs (z1) > zmax) zmax = fabs (z1);
          if (fabs (z2) > zmax) zmax = fabs (z2);
          if (fabs (z3) > zmax) zmax = fabs (z3);
          if (fabs (z4) > zmax) zmax = fabs (z4);
        }
    }

  for (i = n; i > 0; i--)
    {
      curr_x = x[0] + (n - i) * hx;
      curr_y = y[0] - (n - i) * hy;
      for (j = 1; j < i; j++)
        {
          x1 = curr_x + (j - 1) * hx, y1 = curr_y, z1 = get_value (2 * n - i, j - 1, coefs, n);
          x2 = curr_x + (j + 0) * hx, y2 = curr_y, z2 = get_value (2 * n - i, j, coefs, n);
          x3 = curr_x + (j + 0) * hx, y3 = curr_y - hy, z3 = get_value (2 * n - i + 1, j - 1, coefs, n);
          x4 = curr_x + (j + 1) * hx, y4 = curr_y - hy, z4 = get_value (2 * n - i + 1, j + 0, coefs, n);

          glBegin (GL_TRIANGLES);
          glVertex3d (x1, y1, z1);
          glVertex3d (x2, y2, z2);
          glVertex3d (x3, y3, z3);


          glVertex3d (x2, y2, z2);
          glVertex3d (x3, y3, z3);
          glVertex3d (x4, y4, z4);
          glEnd ();
          if (fabs (z1) > zmax) zmax = fabs (z1);
          if (fabs (z2) > zmax) zmax = fabs (z2);
          if (fabs (z3) > zmax) zmax = fabs (z3);
          if (fabs (z4) > zmax) zmax = fabs (z4);
        }
      j = i;
      x1 = curr_x + (j - 1) * hx, y1 = curr_y, z1 = get_value (2 * n - i, j - 1, coefs, n);
      x2 = curr_x + (j + 0) * hx, y2 = curr_y, z2 = get_value (2 * n - i, j, coefs, n);
      x3 = curr_x + (j + 1) * hx, y3 = curr_y, z3 = get_value (2 * n - i, j + 1, coefs, n);
      x4 = curr_x + (j + 0) * hx, y4 = curr_y - hy, z4 = get_value (2 * n - i + 1, j - 1, coefs, n);

      glBegin (GL_TRIANGLES);
      glVertex3d (x1, y1, z1);
      glVertex3d (x2, y2, z2);
      glVertex3d (x4, y4, z4);


      glVertex3d (x2, y2, z2);
      glVertex3d (x3, y3, z3);
      glVertex3d (x4, y4, z4);
      glEnd ();
      if (fabs (z1) > zmax) zmax = fabs (z1);
      if (fabs (z2) > zmax) zmax = fabs (z2);
      if (fabs (z3) > zmax) zmax = fabs (z3);
      if (fabs (z4) > zmax) zmax = fabs (z4);

      for (j = i + 1; j < 2 * i; j++)
        {
          x1 = curr_x + (j + 0) * hx, y1 = curr_y, z1 = get_value (2 * n - i, j, coefs, n);
          x2 = curr_x + (j + 1) * hx, y2 = curr_y, z2 = get_value (2 * n - i, j + 1, coefs, n);
          x3 = curr_x + (j - 1) * hx, y3 = curr_y - hy, z3 = get_value (2 * n - i + 1, j - 2, coefs, n);
          x4 = curr_x + (j + 0) * hx, y4 = curr_y - hy, z4 = get_value (2 * n - i + 1, j - 1, coefs, n);

          glBegin (GL_TRIANGLES);
          glVertex3d (x1, y1, z1);
          glVertex3d (x3, y3, z3);
          glVertex3d (x4, y4, z4);


          glVertex3d (x1, y1, z1);
          glVertex3d (x2, y2, z2);
          glVertex3d (x4, y4, z4);
          glEnd ();
          if (fabs (z1) > zmax) zmax = fabs (z1);
          if (fabs (z2) > zmax) zmax = fabs (z2);
          if (fabs (z3) > zmax) zmax = fabs (z3);
          if (fabs (z4) > zmax) zmax = fabs (z4);
        }
    }
}

void MainWindow::drawThreshold()
{
  int i, j;
  double x1, x2, x3, x4, x5, x6, x7, x8, x9;
  double y1, y2, y3, y4, y5, y6, y7, y8, y9;
  double z1, z2, z3, z4, z5, z6, z7, z8, z9;
  double curr_x, curr_y;
  zmax = 0;
  glColor3d (0, 0, 255);
  for (i = 0; i < n; i++)
    {
      curr_x = x[1] - i * hx;
      curr_y = y[1] - i * hy;
      for (j = 0; j < i; j++)
        {
          x1 = curr_x + j * hx, y1 = curr_y, z1 = get_value (i, j, coefs, n) - ff(x1,y1);
          x2 = curr_x + (j + 1) * hx, y2 = curr_y, z2 = get_value (i, j + 1, coefs, n) - ff(x2,y2);
          x3 = curr_x + (j - 1) * hx, y3 = curr_y - hy; z3 = get_value (i + 1, j - 0, coefs, n) - ff(x3,y3);
          x4 = curr_x + j * hx, y4 = curr_y - hy, z4 = get_value (i + 1, j + 1, coefs, n) - ff(x4,y4);

          glBegin (GL_TRIANGLES);
          glVertex3d (x1, y1, z1);
          glVertex3d (x2, y2, z2);
          glVertex3d (x4, y4, z4);


          glVertex3d (x1, y1, z1);
          glVertex3d (x3, y3, z3);
          glVertex3d (x4, y4, z4);
          glEnd ();
          if (fabs (z1) > zmax) zmax = fabs (z1);
          if (fabs (z2) > zmax) zmax = fabs (z2);
          if (fabs (z3) > zmax) zmax = fabs (z3);
          if (fabs (z4) > zmax) zmax = fabs (z4);
        }
      j = i;
      x1 = curr_x + j * hx, y1 = curr_y, z1 = get_value (i, j, coefs, n) - ff(x1,y1);
      x2 = curr_x + (j - 1) * hx, y2 = curr_y - hy, z2 = get_value (i + 1, j - 0, coefs, n) - ff(x2,y2);
      x3 = curr_x + (j + 0) * hx, y3 = curr_y - hy; z3 = get_value (i + 1, j + 1, coefs, n) - ff(x3,y3);
      x4 = curr_x + (j + 1) * hx, y4 = curr_y - hy, z4 = get_value (i + 1, j + 2, coefs, n) - ff(x4,y4);

      if (fabs (z1) > zmax) zmax = fabs (z1);
      if (fabs (z2) > zmax) zmax = fabs (z2);
      if (fabs (z2) > zmax) zmax = fabs (z3);
      if (fabs (z4) > zmax) zmax = fabs (z4);

      if (zmax > 1e-15)
        {
          z1 /= zmax;
          z2 /= zmax;
          z3 /= zmax;
          z4 /= zmax;
        }

      glBegin (GL_TRIANGLES);
      glVertex3d (x1, y1, z1);
      glVertex3d (x2, y2, z2);
      glVertex3d (x3, y3, z3);


      glVertex3d (x1, y1, z1);
      glVertex3d (x3, y3, z3);
      glVertex3d (x4, y4, z4);
      glEnd ();


      for (j = i + 1; j < 2 * i + 1; j++)
        {
          x1 = curr_x + (j - 1) * hx, y1 = curr_y, z1 = get_value (i, j - 1, coefs, n) - ff(x1,y1);
          x2 = curr_x + j * hx, y2 = curr_y, z2 = get_value (i, j, coefs, n) - ff(x2,y2);
          x3 = curr_x + j * hx, y3 = curr_y - hy, z3 = get_value (i + 1, j + 1, coefs, n) - ff(x3,y3);
          x4 = curr_x + (j + 1) * hx, y4 = curr_y - hy, z4 = get_value (i + 1, j + 2, coefs, n) - ff(x4,y4);

          if (fabs (z1) > zmax) zmax = fabs (z1);
          if (fabs (z2) > zmax) zmax = fabs (z2);
          if (fabs (z3) > zmax) zmax = fabs (z3);
          if (fabs (z4) > zmax) zmax = fabs (z4);

          if (zmax > 1e-15)
            {
              z1 /= zmax;
              z2 /= zmax;
              z3 /= zmax;
              z4 /= zmax;
            }

          glBegin (GL_TRIANGLES);
          glVertex3d (x1, y1, z1);
          glVertex3d (x2, y2, z2);
          glVertex3d (x3, y3, z3);


          glVertex3d (x2, y2, z2);
          glVertex3d (x3, y3, z3);
          glVertex3d (x4, y4, z4);
          glEnd ();
        }
    }

  for (i = n; i > 0; i--)
    {
      curr_x = x[0] + (n - i) * hx;
      curr_y = y[0] - (n - i) * hy;
      for (j = 1; j < i; j++)
        {
          x1 = curr_x + (j - 1) * hx, y1 = curr_y, z1 = get_value (2 * n - i, j - 1, coefs, n) - ff(x1,y1);
          x2 = curr_x + (j + 0) * hx, y2 = curr_y, z2 = get_value (2 * n - i, j, coefs, n) - ff(x2,y2);
          x3 = curr_x + (j + 0) * hx, y3 = curr_y - hy, z3 = get_value (2 * n - i + 1, j - 1, coefs, n) - ff(x3,y3);
          x4 = curr_x + (j + 1) * hx, y4 = curr_y - hy, z4 = get_value (2 * n - i + 1, j + 0, coefs, n) - ff(x4,y4);

          if (fabs (z1) > zmax) zmax = fabs (z1);
          if (fabs (z2) > zmax) zmax = fabs (z2);
          if (fabs (z3) > zmax) zmax = fabs (z3);
          if (fabs (z4) > zmax) zmax = fabs (z4);

          if (zmax > 1e-15)
            {
              z1 /= zmax;
              z2 /= zmax;
              z3 /= zmax;
              z4 /= zmax;
            }

          glBegin (GL_TRIANGLES);
          glVertex3d (x1, y1, z1);
          glVertex3d (x2, y2, z2);
          glVertex3d (x3, y3, z3);


          glVertex3d (x2, y2, z2);
          glVertex3d (x3, y3, z3);
          glVertex3d (x4, y4, z4);
          glEnd ();
        }
      j = i;
      x1 = curr_x + (j - 1) * hx, y1 = curr_y, z1 = get_value (2 * n - i, j - 1, coefs, n) - ff(x1,y1);
      x2 = curr_x + (j + 0) * hx, y2 = curr_y, z2 = get_value (2 * n - i, j, coefs, n) - ff(x2,y2);
      x3 = curr_x + (j + 1) * hx, y3 = curr_y, z3 = get_value (2 * n - i, j + 1, coefs, n) - ff(x3,y3);
      x4 = curr_x + (j + 0) * hx, y4 = curr_y - hy, z4 = get_value (2 * n - i + 1, j - 1, coefs, n) - ff(x4,y4);

      if (fabs (z1) > zmax) zmax = fabs (z1);
      if (fabs (z2) > zmax) zmax = fabs (z2);
      if (fabs (z3) > zmax) zmax = fabs (z3);
      if (fabs (z4) > zmax) zmax = fabs (z4);

      if (zmax > 1e-15)
        {
          z1 /= zmax;
          z2 /= zmax;
          z3 /= zmax;
          z4 /= zmax;
        }

      glBegin (GL_TRIANGLES);
      glVertex3d (x1, y1, z1);
      glVertex3d (x2, y2, z2);
      glVertex3d (x4, y4, z4);


      glVertex3d (x2, y2, z2);
      glVertex3d (x3, y3, z3);
      glVertex3d (x4, y4, z4);
      glEnd ();

      for (j = i + 1; j < 2 * i; j++)
        {
          x1 = curr_x + (j + 0) * hx, y1 = curr_y, z1 = get_value (2 * n - i, j, coefs, n) - ff(x1,y1);
          x2 = curr_x + (j + 1) * hx, y2 = curr_y, z2 = get_value (2 * n - i, j + 1, coefs, n) - ff(x2,y2);
          x3 = curr_x + (j - 1) * hx, y3 = curr_y - hy, z3 = get_value (2 * n - i + 1, j - 2, coefs, n) - ff(x3,y3);
          x4 = curr_x + (j + 0) * hx, y4 = curr_y - hy, z4 = get_value (2 * n - i + 1, j - 1, coefs, n) - ff(x4,y4);

          if (fabs (z1) > zmax) zmax = fabs (z1);
          if (fabs (z2) > zmax) zmax = fabs (z2);
          if (fabs (z3) > zmax) zmax = fabs (z3);
          if (fabs (z4) > zmax) zmax = fabs (z4);
          if (zmax > 1e-15)
            {
              z1 /= zmax;
              z2 /= zmax;
              z3 /= zmax;
              z4 /= zmax;
            }

          glBegin (GL_TRIANGLES);
          glVertex3d (x1, y1, z1);
          glVertex3d (x3, y3, z3);
          glVertex3d (x4, y4, z4);


          glVertex3d (x1, y1, z1);
          glVertex3d (x2, y2, z2);
          glVertex3d (x4, y4, z4);
          glEnd ();
        }
    }
}

void MainWindow::initializeGL()
{
  glClearColor(1, 1, 1, 1);

  glEnable(GL_NORMALIZE);
  glEnable(GL_DEPTH_TEST);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_COLOR_MATERIAL);

  float
    pos[4] = {0, 1, 6, 0}, dir[3] = {0, 0, -1},

    obj_amb[4] = {0.24725, 0.1995, 0.0745},
    obj_dif[4] = {0.75164, 0.60648, 0.22648},
    obj_spec[4] = {0.628281, 0.555802, 0.366065};

  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHTING);
  glEnable(GL_COLOR_MATERIAL);

  glLightfv(GL_LIGHT0, GL_POSITION, pos);
  glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, dir);

  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, obj_amb);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, obj_dif);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, obj_spec);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 51.2);
}
