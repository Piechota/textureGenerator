#include "mainWindow.h"

#include <QtWidgets/QScrollArea>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSplitter>

void CMainWindow::GenerateImage()
{
	GRender.GenerateImage();

	BYTE* textureData = (BYTE*)GRender.GetRenderTargetData();
	STextureMetadata const& textureMetadata = GRender.GetTextureMetadata();

	QImage textureImage(m_imageWidth, m_imageHeight, m_formatsToQtFormat[m_imageFormatID]);
	for (UINT rowID = 0; rowID < textureMetadata.m_numRows; ++rowID)
	{
		BYTE* srcData = textureData + rowID * textureMetadata.m_textureFootprint.Footprint.RowPitch;
		BYTE* dstData = textureImage.bits() + rowID * textureMetadata.m_rowSize;
		memcpy(dstData, srcData, textureMetadata.m_rowSize);
	}
	m_lGeneratedImage->setPixmap(QPixmap::fromImage(textureImage));
}

void CMainWindow::SlotGenerateImage()
{
	QString const shaderCodeStr = m_pteCodeEditor->toPlainText();
	QByteArray const shaderCodeCharArray = shaderCodeStr.toLatin1();

	QString output;
	if (GRender.ChangePixelShader(shaderCodeCharArray.data(), output))
	{
		GenerateImage();
	}
	m_lShadersOutput->setText(output);
}

void CMainWindow::SlotImageSettingsChange()
{
	m_imageWidth = m_leImageWidth->text().toUInt();
	m_imageHeight = m_leImageHeight->text().toUInt();
	m_imageFormatID = m_cbFormats->currentIndex();
	DXGI_FORMAT const dxgiFormat = m_formatsToDXGI[m_imageFormatID];
	if (GRender.ChangeImageSettings(m_imageWidth, m_imageHeight, dxgiFormat))
	{
		GenerateImage();
	}
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

	m_lShadersOutput = new QLabel();
	m_lShadersOutput->setWordWrap(true);
	m_lShadersOutput->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	m_lShadersOutput->setStyleSheet("QLabel { background-color : grey; }");

	m_leImageWidth = new QLineEdit(QString::number(m_imageWidth));
	m_leImageHeight = new QLineEdit(QString::number(m_imageHeight));
	QPushButton* applyImageSize = new QPushButton("Apply");
	connect(applyImageSize, SIGNAL(clicked()), this, SLOT(SlotImageSettingsChange()));

	QStringList formatsNames;
	formatsNames.append("R8G8B8A8");
	formatsNames.append("Grayscale");

	m_cbFormats = new QComboBox();
	m_cbFormats->addItems(formatsNames);

	QBoxLayout* imageSizeLayout = new QBoxLayout(QBoxLayout::LeftToRight);
	imageSizeLayout->addWidget(new QLabel("Width: "));
	imageSizeLayout->addWidget(m_leImageWidth);
	imageSizeLayout->addWidget(new QLabel("Height: "));
	imageSizeLayout->addWidget(m_leImageHeight);
	imageSizeLayout->addWidget(new QLabel("Format: "));
	imageSizeLayout->addWidget(m_cbFormats);
	imageSizeLayout->addWidget(applyImageSize);

	QBoxLayout* imageLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	imageLayout->addLayout(imageSizeLayout);
	imageLayout->addWidget(m_lGeneratedImage);

	QSplitter* shaderCodeOutputSplitter = new QSplitter(Qt::Vertical);
	shaderCodeOutputSplitter->addWidget(m_pteCodeEditor);
	shaderCodeOutputSplitter->addWidget(m_lShadersOutput);

	QBoxLayout* codeEditLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	codeEditLayout->addWidget(shaderCodeOutputSplitter);
	codeEditLayout->addWidget(btnGenerate);

	QBoxLayout* boxLayout = new QBoxLayout(QBoxLayout::LeftToRight);
	boxLayout->addLayout(imageLayout);
	boxLayout->addLayout(codeEditLayout);

	QScrollArea* centerWidget = new QScrollArea();
	centerWidget->setLayout(boxLayout);

	QMainWindow::setBaseSize(1000, 600);
	QMainWindow::setCentralWidget(centerWidget);
	QMainWindow::show();

	m_formatsToQtFormat[ETextureFormats::R8G8B8A8] = QImage::Format_RGBA8888;
	m_formatsToQtFormat[ETextureFormats::Grayscale] = QImage::Format_Grayscale8;

	m_formatsToDXGI[ETextureFormats::R8G8B8A8] = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_formatsToDXGI[ETextureFormats::Grayscale] = DXGI_FORMAT_R8_UNORM;
}
