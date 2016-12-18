#pragma once
#include "globals.h"

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QLineEdit>
#include <QtGui/QImage>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsPixmapItem>

class CImageView : public QGraphicsView
{
	Q_OBJECT
private:
	QPoint m_oldMousePosition;
	QGraphicsItem* m_rootItem;

	QBrush m_checkerboardBrush;
	QMatrix m_checkerboardMatrix;

protected:
	virtual void mouseMoveEvent(QMouseEvent *event) override;
	virtual void mousePressEvent(QMouseEvent *event) override;
	virtual void wheelEvent(QWheelEvent *event) override;

public:
	CImageView( QGraphicsScene* scene, QGraphicsItem* root )
		: QGraphicsView( scene )
		, m_rootItem( root )
	{}

	QBrush& GetCheckerboardBrush() { return m_checkerboardBrush; }
	QMatrix& GetCheckerboardMatrix() { return m_checkerboardMatrix; }
	void ResetPosition();
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
	QGraphicsScene* m_gsImageScene;
	CImageView* m_ivImageView;
	QGraphicsPixmapItem* m_gpiGeneratedImage;
	QPixmap m_pGeneratedImage;
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
	void SlotFitView();

public:
	CMainWindow();
};
