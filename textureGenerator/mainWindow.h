#pragma once
#include "globals.h"

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>

class CMainWindow : public QMainWindow
{
	Q_OBJECT

private:
	QLabel* m_lGeneratedImage;
	QPlainTextEdit* m_pteCodeEditor;

private slots:
	void SlotGenerateImage();

public:
	CMainWindow();
};
