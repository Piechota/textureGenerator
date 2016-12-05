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
	QLabel* m_lShadersOutput;

	UINT m_imageWidth;
	UINT m_imageHeight;

private:
	void GenerateImage();

private slots:
	void SlotGenerateImage();
	void SlotImageSizeChange();

public:
	CMainWindow();
};
