#include "mainWindow.h"

#include <QtWidgets/QScrollArea>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtGui/QImage>

void CMainWindow::SlotGenerateImage()
{
	QString const shaderCodeStr = m_pteCodeEditor->toPlainText();
	QByteArray const shaderCodeCharArray = shaderCodeStr.toLatin1();

	GRender.ChangePixelShader(shaderCodeCharArray.data());
	GRender.GenerateImage();

	BYTE* textureData = (BYTE*)GRender.GetRenderTargetData();
	STextureMetadata const& textureMetadata = GRender.GetTextureMetadata();
	
	QImage textureImage(m_imageWidth, m_imageHeight, QImage::Format_RGBA8888);
	for (UINT rowID = 0; rowID < textureMetadata.m_numRows; ++rowID)
	{
		BYTE* srcData = textureData + rowID * textureMetadata.m_textureFootprint.Footprint.RowPitch;
		BYTE* dstData = textureImage.bits() + rowID * textureMetadata.m_rowSize;
		memcpy(dstData, srcData, textureMetadata.m_rowSize);
	}
	m_lGeneratedImage->setPixmap(QPixmap::fromImage(textureImage));
}

void CMainWindow::SlotImageSizeChange()
{
	m_imageWidth = m_leImageWidth->text().toUInt();
	m_imageHeight = m_leImageHeight->text().toUInt();
	GRender.ChangeTargetSize(m_imageWidth, m_imageHeight);
	SlotGenerateImage();
}

CMainWindow::CMainWindow()
	: m_imageWidth( 600 )
	, m_imageHeight( 600 )
{
	QMainWindow::setWindowTitle("Texture Generator");

	m_lGeneratedImage = new QLabel();

	QPushButton* btnGenerate = new QPushButton("Generate Image");
	connect(btnGenerate, SIGNAL(clicked()), this, SLOT(SlotGenerateImage()));

	m_pteCodeEditor = new QPlainTextEdit();

	m_pteCodeEditor->setPlainText
	(
		"struct PSInput\n"
		"{\n"
		"	float4 position : SV_POSITION;\n"
		"	float2 uv : TEXCOORD;\n"
		"};\n"
		"\n"
		"float4 psMain(PSInput input) : SV_TARGET\n"
		"{\n"
		"	return float4(0.f, 0.f, 1.f, 1.f);\n"
		"}\n"
	);

	m_leImageWidth = new QLineEdit(QString::number(m_imageWidth));
	m_leImageHeight = new QLineEdit(QString::number(m_imageHeight));
	QPushButton* applyImageSize = new QPushButton("Apply");
	connect(applyImageSize, SIGNAL(clicked()), this, SLOT(SlotImageSizeChange()));

	QBoxLayout* imageSizeLayout = new QBoxLayout(QBoxLayout::LeftToRight);
	imageSizeLayout->addWidget(new QLabel("Width: "));
	imageSizeLayout->addWidget(m_leImageWidth);
	imageSizeLayout->addWidget(new QLabel("Height: "));
	imageSizeLayout->addWidget(m_leImageHeight);
	imageSizeLayout->addWidget(applyImageSize);

	QBoxLayout* imageLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	imageLayout->addLayout(imageSizeLayout);
	imageLayout->addWidget(m_lGeneratedImage);

	QBoxLayout* codeEditLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	codeEditLayout->addWidget(m_pteCodeEditor);
	codeEditLayout->addWidget(btnGenerate);

	QBoxLayout* boxLayout = new QBoxLayout(QBoxLayout::LeftToRight);
	boxLayout->addLayout(imageLayout);
	boxLayout->addLayout(codeEditLayout);

	QScrollArea* centerWidget = new QScrollArea();
	centerWidget->setLayout(boxLayout);

	QMainWindow::setBaseSize(1000, 600);
	QMainWindow::setCentralWidget(centerWidget);
	QMainWindow::show();
}
