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
	
	QImage textureImage(600, 600, QImage::Format_RGBA8888);
	for (UINT rowID = 0; rowID < textureMetadata.m_numRows; ++rowID)
	{
		BYTE* srcData = textureData + rowID * textureMetadata.m_textureFootprint.Footprint.RowPitch;
		BYTE* dstData = textureImage.bits() + rowID * textureMetadata.m_rowSize;
		memcpy(dstData, srcData, textureMetadata.m_rowSize);
	}
	m_lGeneratedImage->setPixmap(QPixmap::fromImage(textureImage));
}

CMainWindow::CMainWindow()
{
	QMainWindow::setWindowTitle("Texture Generator");

	m_lGeneratedImage = new QLabel();
	m_lGeneratedImage->setMinimumSize(600, 600);

	QPushButton* btnGenerate = new QPushButton("Generate Image");
	connect(btnGenerate, SIGNAL(clicked()), this, SLOT(SlotGenerateImage()));

	m_pteCodeEditor = new QPlainTextEdit();
	m_pteCodeEditor->setMinimumSize(400, 400);

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

	QBoxLayout* boxLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	boxLayout->addWidget(m_lGeneratedImage);
	boxLayout->addWidget(btnGenerate);
	boxLayout->addWidget(m_pteCodeEditor);

	QScrollArea* centerWidget = new QScrollArea();
	centerWidget->setLayout(boxLayout);

	QMainWindow::setCentralWidget(centerWidget);
	QMainWindow::show();
}
