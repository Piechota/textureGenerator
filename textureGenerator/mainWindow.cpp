#include "mainWindow.h"

#include <QtWidgets/QScrollArea>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QDockWidget>

void CMainWindow::FitImage()
{
	m_lGeneratedImage->resize(m_scrollAreaSize);
	m_pGeneratedImage = m_pGeneratedImage.scaled(m_scrollAreaSize, Qt::KeepAspectRatio);
	m_lGeneratedImage->setPixmap(m_pGeneratedImage);
}

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
	m_pGeneratedImage.convertFromImage(textureImage);
	m_lGeneratedImage->setPixmap(m_pGeneratedImage);
	FitImage();
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
	m_isaImageScroll->ResetScale();
	m_scrollAreaSize = m_isaImageScroll->size() * m_isaImageScroll->GetScale();
	if (GRender.ChangeImageSettings(m_imageWidth, m_imageHeight, dxgiFormat))
	{
		GenerateImage();
	}
}

void CMainWindow::SlotScaleChanged()
{
	m_scrollAreaSize = m_scrollAreaSize * m_isaImageScroll->GetScale();
	FitImage();
}

CMainWindow::CMainWindow()
	: m_imageWidth( 600 )
	, m_imageHeight( 600 )
{
	QMainWindow::setWindowTitle("Texture Generator");

	m_lGeneratedImage = new QLabel();

	QPushButton* btnGenerate = new QPushButton("Reload shader");
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

	m_isaImageScroll = new CImageScrollArea();
	m_isaImageScroll->setBackgroundRole(QPalette::Dark);
	m_isaImageScroll->setWidget(m_lGeneratedImage);
	m_isaImageScroll->setWidgetResizable(false);
	connect(m_isaImageScroll, SIGNAL(ScaleChanged()), this, SLOT(SlotScaleChanged()));

	QBoxLayout* imageLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	imageLayout->addLayout(imageSizeLayout);
	imageLayout->addWidget(m_isaImageScroll);

	QBoxLayout* codeEditLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	codeEditLayout->addWidget(m_pteCodeEditor);
	codeEditLayout->addWidget(btnGenerate);

	QWidget* dummyWidget = new QWidget();
	dummyWidget->setLayout(codeEditLayout);

	QDockWidget* codeDockWidget = new QDockWidget("Code edit");
	codeDockWidget->setFeatures(QFlags<QDockWidget::DockWidgetFeature>(QDockWidget::DockWidgetFeature::DockWidgetFloatable | QDockWidget::DockWidgetFeature::DockWidgetMovable));
	codeDockWidget->setWidget(dummyWidget);

	QDockWidget* outputDockWidget = new QDockWidget("Output");
	outputDockWidget->setFeatures(QFlags<QDockWidget::DockWidgetFeature>(QDockWidget::DockWidgetFeature::DockWidgetFloatable | QDockWidget::DockWidgetFeature::DockWidgetMovable));
	outputDockWidget->setWidget(m_lShadersOutput);

	QScrollArea* centerWidget = new QScrollArea();
	centerWidget->setLayout(imageLayout);

	QMainWindow::setCentralWidget(centerWidget);
	QMainWindow::addDockWidget(Qt::RightDockWidgetArea, codeDockWidget);
	QMainWindow::addDockWidget(Qt::BottomDockWidgetArea, outputDockWidget);
	QMainWindow::resize(600, 600);
	QMainWindow::show();

	m_formatsToQtFormat[ETextureFormats::R8G8B8A8] = QImage::Format_RGBA8888;
	m_formatsToQtFormat[ETextureFormats::Grayscale] = QImage::Format_Grayscale8;

	m_formatsToDXGI[ETextureFormats::R8G8B8A8] = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_formatsToDXGI[ETextureFormats::Grayscale] = DXGI_FORMAT_R8_UNORM;

	m_scrollAreaSize = m_isaImageScroll->size() * m_isaImageScroll->GetScale();
	SlotGenerateImage();
}

void CImageScrollArea::wheelEvent(QWheelEvent *event)
{
	QPoint const& angleDelta = event->angleDelta();
	m_scale = 1.f + angleDelta.y() * (1.f / (120.f)) * 0.1f;
	emit ScaleChanged();
}

CImageScrollArea::CImageScrollArea()
	: QScrollArea()
	, m_scale( 0.99f )
{}
