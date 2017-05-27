//#include <QtGui/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  MainWindow *window = new MainWindow;

  if (window->parce_comand_line (argc, argv))
    {
      printf ("usage: %s <x1> <y1> <x2> <y2> <n>\n", argv[0]);
      delete window;
      return 1;
    }
  window->setGeometry (100, 100, 950, 950);

  window->show ();
  app.exec ();

  delete window;
  return 0;
}
