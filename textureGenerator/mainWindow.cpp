#include "mainWindow.h"

#include <QtWidgets/QScrollArea>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QShortcut>

float const GCheckerboardScale = 50.f;
uchar const GCheckerboardData[] =
{
	0xAA, 0x66, 0x00, 0x00,
	0x66, 0xAA, 0x00, 0x00,
};

void CMainWindow::GenerateImage()
{
	GRender.GenerateImage();

	BYTE* textureData = (BYTE*)GRender.GetRenderTargetData();
	STextureMetadata const& textureMetadata = GRender.GetTextureMetadata();

	QImage textureImage(m_imageWidth, m_imageHeight, m_formatsToQtFormat[m_imageFormatID]);
	for (UINT rowID = 0; rowID < textureMetadata.m_numRows; ++rowID)
	{
		BYTE* srcData = textureData + rowID * textureMetadata.m_rowPitch;
		BYTE* dstData = textureImage.bits() + rowID * textureMetadata.m_rowSize;
		memcpy(dstData, srcData, textureMetadata.m_rowSize);
	}
	m_pGeneratedImage.convertFromImage(textureImage);
	m_gpiGeneratedImage->setPixmap(m_pGeneratedImage);
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

void CMainWindow::SlotFitView()
{
	m_gsImageScene->setSceneRect(m_gpiGeneratedImage->boundingRect());
	m_ivImageView->fitInView(m_gpiGeneratedImage->boundingRect(), Qt::KeepAspectRatio);
	m_ivImageView->ResetPosition();
}

CMainWindow::CMainWindow()
	: m_imageWidth( 600 )
	, m_imageHeight( 600 )
	, m_imageFormatID( 0 )
{
	QMainWindow::setWindowTitle("Texture Generator");

	QPushButton* btnGenerate = new QPushButton("Compile(F5)");
	connect(btnGenerate, SIGNAL(clicked()), this, SLOT(SlotGenerateImage()));
	QShortcut* compileShortcut = new QShortcut(QKeySequence( "F5" ), this);
	connect(compileShortcut, SIGNAL(activated()), this, SLOT(SlotGenerateImage()));

	m_pteCodeEditor = new QPlainTextEdit();

	m_pteCodeEditor->setPlainText
	(
		"float4 psMain(\n"
		"	float4 position : SV_POSITION\n"
		",	float2 uv : TEXCOORD\n"
		") : SV_TARGET\n"
		"{\n"
		"	return float4(0.f, 0.f, 1.f, 1.f);\n"
		"}\n"
	);
	m_pteCodeEditor->setTabStopWidth(m_pteCodeEditor->tabStopWidth() / 4);

	m_lShadersOutput = new QLabel();
	m_lShadersOutput->setWordWrap(true);
	m_lShadersOutput->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	m_lShadersOutput->setStyleSheet("QLabel { background-color : grey; }");

	m_leImageWidth = new QLineEdit(QString::number(m_imageWidth));
	m_leImageHeight = new QLineEdit(QString::number(m_imageHeight));
	connect(m_leImageWidth, SIGNAL(returnPressed()), this, SLOT(SlotImageSettingsChange()));
	connect(m_leImageHeight, SIGNAL(returnPressed()), this, SLOT(SlotImageSettingsChange()));
	QPushButton* fitInView = new QPushButton("FitIn&View");
	connect(fitInView, SIGNAL(clicked()), this, SLOT(SlotFitView()));
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
	imageSizeLayout->addWidget(fitInView);
	imageSizeLayout->addWidget(applyImageSize);
	imageSizeLayout->addWidget(new QWidget(), 1);

	QImage checkerboardImg((uchar*)GCheckerboardData, 2, 2, QImage::Format_Grayscale8);
	m_gsImageScene = new QGraphicsScene();
	m_gpiGeneratedImage = new QGraphicsPixmapItem();
	m_gsImageScene->addItem(m_gpiGeneratedImage);

	m_ivImageView = new CImageView(m_gsImageScene, m_gpiGeneratedImage);

	m_ivImageView->GetCheckerboardBrush() = QBrush(checkerboardImg);
	m_ivImageView->GetCheckerboardMatrix().reset();
	m_ivImageView->GetCheckerboardMatrix().scale(GCheckerboardScale, GCheckerboardScale);
	m_ivImageView->GetCheckerboardBrush().setMatrix(m_ivImageView->GetCheckerboardMatrix());
	m_ivImageView->setBackgroundBrush(m_ivImageView->GetCheckerboardBrush());

	QBoxLayout* imageLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	imageLayout->addLayout(imageSizeLayout);
	imageLayout->addWidget(m_ivImageView);

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

	QString const shaderCodeStr = m_pteCodeEditor->toPlainText();
	QByteArray const shaderCodeCharArray = shaderCodeStr.toLatin1();
	DXGI_FORMAT const dxgiFormat = m_formatsToDXGI[m_imageFormatID];
	QString output;
	GRender.ChangePixelShader(shaderCodeCharArray.data(), output);
	GRender.ChangeImageSettings(m_imageWidth, m_imageHeight, dxgiFormat);
	GenerateImage();
	m_ivImageView->fitInView(m_gpiGeneratedImage->boundingRect(), Qt::KeepAspectRatio);
}

void CImageView::mouseMoveEvent(QMouseEvent *event)
{
	if (event->buttons() & Qt::LeftButton)
	{
		QPoint const mouseDelta = (event->pos() - m_oldMousePosition);
		m_oldMousePosition = event->pos();

		m_rootItem->moveBy( mouseDelta.x(), mouseDelta.y() );
		m_checkerboardMatrix.translate(mouseDelta.x() * (1.f / GCheckerboardScale), mouseDelta.y() * (1.f / GCheckerboardScale));
		m_checkerboardBrush.setMatrix(m_checkerboardMatrix);
		QGraphicsView::setBackgroundBrush(m_checkerboardBrush);
	}
}

void CImageView::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_oldMousePosition = event->pos();
	}
}

void CImageView::wheelEvent(QWheelEvent *event)
{
	int const wheelSteps = event->angleDelta().y() / (8 * 15);
	float const wheelScale = 1.f + wheelSteps * (4.f/360.f);
	QGraphicsView::scale(wheelScale, wheelScale);
}

void CImageView::ResetPosition()
{
	m_rootItem->setPos(0.f, 0.f);

	m_checkerboardMatrix.setMatrix
	(
		m_checkerboardMatrix.m11(), m_checkerboardMatrix.m12(),
		m_checkerboardMatrix.m21(), m_checkerboardMatrix.m22(),
		0.f, 0.f
	);
	m_checkerboardBrush.setMatrix(m_checkerboardMatrix);
	QGraphicsView::setBackgroundBrush(m_checkerboardBrush);
}
