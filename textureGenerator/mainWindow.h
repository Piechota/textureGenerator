#pragma once
#include "globals.h"

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QLineEdit>
#include <QtGui/QImage>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QScrollArea>

class CImageScrollArea : public QScrollArea
{
	Q_OBJECT

private:
	float m_scale;
	
protected:
	virtual void wheelEvent(QWheelEvent *event) override;

signals:
	void ScaleChanged();

public:
	CImageScrollArea();
	float GetScale() const { return m_scale; }
	void ResetScale() { m_scale = 0.99f; }
};

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
	CImageScrollArea* m_isaImageScroll;
	QLabel* m_lGeneratedImage;
	QPixmap m_pGeneratedImage;
	QPlainTextEdit* m_pteCodeEditor;
	QLabel* m_lShadersOutput;

	QSize m_scrollAreaSize;

	UINT m_imageWidth;
	UINT m_imageHeight;
	UINT m_imageFormatID;

private:
	void FitImage();
	void GenerateImage();

private slots:
	void SlotGenerateImage();
	void SlotImageSettingsChange();
	void SlotScaleChanged();

public:
	CMainWindow();
};
