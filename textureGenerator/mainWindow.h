#pragma once
#include "globals.h"

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QLineEdit>
#include <QtGui/QImage>
#include <QtWidgets/QComboBox>

class CMainWindow : public QMainWindow
{
	Q_OBJECT

private: 
	enum ETextureFormats
	{  
		R8G8B8A8,
		Grayscale,
		FormatsNum
	};

private:
	QImage::Format m_formatsToQtFormat[ETextureFormats::FormatsNum];
	DXGI_FORMAT m_formatsToDXGI[ETextureFormats::FormatsNum];

	QLineEdit* m_leImageWidth;
	QLineEdit* m_leImageHeight;
	QComboBox* m_cbFormats;
	QLabel* m_lGeneratedImage;
	QPlainTextEdit* m_pteCodeEditor;
	QLabel* m_lShadersOutput;

	UINT m_imageWidth;
	UINT m_imageHeight;
	UINT m_imageFormatID;

private:
	void GenerateImage();

private slots:
	void SlotGenerateImage();
	void SlotImageSettingsChange();

public:
	CMainWindow();
};
