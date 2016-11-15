#include "globals.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>

QApplication* GApplication = nullptr;
QMainWindow* GMainWindow = nullptr;

int main(int argc, char* argv[])
{
	GApplication = new QApplication(argc, argv);

	GMainWindow = new QMainWindow();
	GMainWindow->setWindowTitle("Texture Generator");

	GMainWindow->show();

	return GApplication->exec();
}