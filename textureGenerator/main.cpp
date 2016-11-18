#include "globals.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>

#include "render.h"

QApplication* GApplication = nullptr;
QMainWindow* GMainWindow = nullptr;

int main(int argc, char* argv[])
{
	GRender.Init();

	GApplication = new QApplication(argc, argv);

	GMainWindow = new QMainWindow();
	GMainWindow->setWindowTitle("Texture Generator");

	GMainWindow->show();

	int const retValue = GApplication->exec();

	GRender.Release();

	return retValue;
}