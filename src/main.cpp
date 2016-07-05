#include <FL/Fl.H>

#include "pichi.h"
#include "ui/mainwindow.h"


int main(int argc, char* argv[])
{
  Pichi pichi{Configuration("config.txt")};

  MainWindow window{&pichi};
  window.show(argc, argv);

  return Fl::run();
}