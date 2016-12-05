#pragma once
#include "globals.h"

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QLineEdit>

class CMainWindow : public QMainWindow
{
	Q_OBJECT

private:
	QLineEdit* m_leImageWidth;
	QLineEdit* m_leImageHeight;
	QLabel* m_lGeneratedImage;
	QPlainTextEdit* m_pteCodeEditor;

	UINT m_imageWidth;
	UINT m_imageHeight;

private slots:
	void SlotGenerateImage();
	void SlotImageSizeChange();

public:
	CMainWindow();
};
