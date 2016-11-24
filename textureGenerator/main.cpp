#include "globals.h"
#include "moc_mainWindow.h"

#include <QtWidgets/QApplication>

QApplication* GApplication = nullptr;

int main(int argc, char* argv[])
{
	GRender.Init();

	GApplication = new QApplication(argc, argv);

	CMainWindow* mainWindow = new CMainWindow();

	int const retValue = GApplication->exec();

	GRender.Release();

	return retValue;
}