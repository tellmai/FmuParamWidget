#include "fmuparamwidget.h"

#include "internalparameditorwidget.h"
#include "inputparameditorwidget.h"
#include "stylesheetloader.h"

#include <QAction>
#include <QApplication>
#include <QEvent>
#include <QFileDialog>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QRadioButton>
#include <QResizeEvent>
#include <QToolButton>
#include <QVBoxLayout>

class ChevronToolButton : public QToolButton
{
public:
    explicit ChevronToolButton(QWidget *parent = nullptr)
        : QToolButton(parent)
    {}

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(QPen(QColor(96, 96, 96), 1.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

        const QPointF center = rect().center();
        if (arrowType() == Qt::DownArrow) {
            painter.drawLine(QPointF(center.x() - 4.0, center.y() - 2.0), QPointF(center.x(), center.y() + 3.0));
            painter.drawLine(QPointF(center.x(), center.y() + 3.0), QPointF(center.x() + 4.0, center.y() - 2.0));
        } else {
            painter.drawLine(QPointF(center.x() - 2.0, center.y() - 5.0), QPointF(center.x() + 3.0, center.y()));
            painter.drawLine(QPointF(center.x() + 3.0, center.y()), QPointF(center.x() - 2.0, center.y() + 5.0));
        }
    }
};

FmuSectionWidget::FmuSectionWidget(const QString &title, QWidget *contentWidget, QWidget *parent)
    : QWidget(parent),
      m_arrowButton(nullptr),
      m_titleLabel(nullptr),
      m_headerWidget(nullptr),
      m_contentContainer(nullptr),
      m_contentWidget(contentWidget),
      m_expanded(false)
{
    if (!m_contentWidget) {
        m_contentWidget = new QWidget(this);
    }

    m_contentWidget->setParent(this);
    SetupUI(title);
    SetExpanded(false);
}

void FmuSectionWidget::SetExpanded(bool expanded)
{
    m_expanded = expanded;
    m_arrowButton->setArrowType(m_expanded ? Qt::DownArrow : Qt::RightArrow);
    m_contentContainer->setVisible(m_expanded);
}

bool FmuSectionWidget::IsExpanded() const
{
    return m_expanded;
}

bool FmuSectionWidget::eventFilter(QObject *watched, QEvent *event)
{
    if ((watched == m_headerWidget || watched == m_titleLabel)
        && event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            ToggleExpanded();
            return true;
        }
    }

    return QWidget::eventFilter(watched, event);
}

void FmuSectionWidget::SetupUI(const QString &title)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_headerWidget = new QWidget(this);
    m_headerWidget->setObjectName(QStringLiteral("SectionHeader"));
    m_headerWidget->setCursor(Qt::PointingHandCursor);
    m_headerWidget->installEventFilter(this);

    QHBoxLayout *headerLayout = new QHBoxLayout(m_headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(4);

    m_arrowButton = new ChevronToolButton(m_headerWidget);
    m_arrowButton->setObjectName(QStringLiteral("SectionArrow"));
    m_arrowButton->setArrowType(Qt::RightArrow);
    m_arrowButton->setCursor(Qt::PointingHandCursor);
    m_arrowButton->setFixedSize(16, 24);

    m_titleLabel = new QLabel(title, m_headerWidget);
    m_titleLabel->setObjectName(QStringLiteral("SectionTitle"));
    m_titleLabel->setCursor(Qt::PointingHandCursor);
    m_titleLabel->installEventFilter(this);

    headerLayout->addWidget(m_arrowButton);
    headerLayout->addWidget(m_titleLabel);
    headerLayout->addStretch();

    m_contentContainer = new QWidget(this);
    m_contentContainer->setObjectName(QStringLiteral("SectionContent"));
    QVBoxLayout *contentLayout = new QVBoxLayout(m_contentContainer);
    contentLayout->setContentsMargins(20, 4, 0, 0);
    contentLayout->setSpacing(0);
    contentLayout->addWidget(m_contentWidget);

    mainLayout->addWidget(m_headerWidget);
    mainLayout->addWidget(m_contentContainer);

    connect(m_arrowButton, &QToolButton::clicked, this, [this]() {
        ToggleExpanded();
    });
}

void FmuSectionWidget::ToggleExpanded()
{
    SetExpanded(!m_expanded);
}

FmuParamWidget::FmuParamWidget(QWidget *parent)
    : QWidget(parent),
      m_modelPathLabel(nullptr),
      m_helpLabel(nullptr),
      m_modelPathEdit(nullptr),
      m_coSimulationRadio(nullptr),
      m_modelExchangeRadio(nullptr),
      m_internalSection(nullptr),
      m_simulationSection(nullptr),
      m_inputSection(nullptr),
      m_outputSection(nullptr),
      m_internalParamEditor(nullptr),
      m_inputParamEditor(nullptr)
{
    SetupUI();
    SetSimulationModeRadiosVisible(false);
}

void FmuParamWidget::SetCoSimulationRadioVisible(bool visible)
{
    if (m_coSimulationRadio) {
        m_coSimulationRadio->setVisible(visible);
    }
}

void FmuParamWidget::SetModelExchangeRadioVisible(bool visible)
{
    if (m_modelExchangeRadio) {
        m_modelExchangeRadio->setVisible(visible);
    }
}

void FmuParamWidget::SetSimulationModeRadiosVisible(bool visible)
{
    SetCoSimulationRadioVisible(visible);
    SetModelExchangeRadioVisible(visible);
}

void FmuParamWidget::SetupUI()
{
    setObjectName(QStringLiteral("FmuParamWidget"));
    setWindowTitle(tr("FMU参数设置"));

    QFont appFont(QStringLiteral("Microsoft YaHei UI"));
    appFont.setPixelSize(14);
    setFont(appFont);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSizeConstraint(QLayout::SetNoConstraint);
    mainLayout->setContentsMargins(22, 8, 22, 18);
    mainLayout->setSpacing(18);

    QWidget *pathArea = new QWidget(this);
    pathArea->setObjectName(QStringLiteral("PathArea"));
    QVBoxLayout *pathLayout = new QVBoxLayout(pathArea);
    pathLayout->setContentsMargins(0, 0, 0, 0);
    pathLayout->setSpacing(6);

    QHBoxLayout *labelLayout = new QHBoxLayout();
    labelLayout->setContentsMargins(0, 0, 0, 0);
    labelLayout->setSpacing(6);

    m_modelPathLabel = new QLabel(tr("模型路径"), pathArea);
    m_modelPathLabel->setObjectName(QStringLiteral("ModelPathLabel"));

    m_helpLabel = new QLabel(QStringLiteral("?"), pathArea);
    m_helpLabel->setObjectName(QStringLiteral("HelpLabel"));
    m_helpLabel->setAlignment(Qt::AlignCenter);
    m_helpLabel->setFixedSize(14, 14);

    labelLayout->addWidget(m_modelPathLabel);
    labelLayout->addWidget(m_helpLabel);
    labelLayout->addStretch();

    QHBoxLayout *pathRowLayout = new QHBoxLayout();
    pathRowLayout->setContentsMargins(0, 0, 0, 0);
    pathRowLayout->setSpacing(14);

    m_modelPathEdit = new QLineEdit(pathArea);
    m_modelPathEdit->setObjectName(QStringLiteral("ModelPathEdit"));
    m_modelPathEdit->setText(QStringLiteral(""));
    m_modelPathEdit->setFixedHeight(30);
    QAction *browseAction = m_modelPathEdit->addAction(CreateFolderIcon(), QLineEdit::TrailingPosition);

    m_coSimulationRadio = new QRadioButton(tr("联合仿真"), pathArea);
    m_modelExchangeRadio = new QRadioButton(tr("模型交换"), pathArea);
    m_coSimulationRadio->setChecked(true);
    SetSimulationModeRadiosVisible(false);

    pathRowLayout->addWidget(m_modelPathEdit, 1);
    pathRowLayout->addWidget(m_coSimulationRadio);
    pathRowLayout->addWidget(m_modelExchangeRadio);

    pathLayout->addLayout(labelLayout);
    pathLayout->addLayout(pathRowLayout);

    QVBoxLayout *sectionsLayout = new QVBoxLayout();
    sectionsLayout->setContentsMargins(0, 0, 0, 0);
    sectionsLayout->setSpacing(12);
    CreateSections(sectionsLayout);

    mainLayout->addWidget(pathArea);
    mainLayout->addLayout(sectionsLayout);
    mainLayout->addStretch();

    setStyleSheet(LoadStyleSheet(QStringLiteral(":/qss/fmuparamwidget.qss")));

    SetupConnections(browseAction);

    resize(1038, 630);
    setMinimumSize(32, 0);
    UpdateResponsiveMargins();
}

void FmuParamWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    UpdateResponsiveMargins();
}

void FmuParamWidget::SetupConnections(QAction *browseAction)
{
    connect(browseAction, &QAction::triggered, this, [this]() {
        OnBrowseModelPathTriggered();
    });
}

void FmuParamWidget::UpdateResponsiveMargins()
{
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (!mainLayout) {
        return;
    }

    // 设计稿宽度下保留 22px 左右边距；窗口极窄时逐步收窄边距，
    // 这样顶层最小宽度设为 32 后，内容区域不会全部被外边距挤掉。
    int horizontalMargin = 22;
    if (width() < 96) {
        horizontalMargin = 0;
    } else if (width() < 180) {
        horizontalMargin = 4;
    } else if (width() < 360) {
        horizontalMargin = 10;
    }

    mainLayout->setContentsMargins(horizontalMargin, 8, horizontalMargin, 18);
}

void FmuParamWidget::CreateSections(QVBoxLayout *sectionsLayout)
{
    m_internalParamEditor = new InternalParamEditorWidget(this);
    m_internalParamEditor->setMinimumSize(0, 0);
    m_internalParamEditor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_internalParamEditor->setFixedHeight(292);

    m_internalSection = new FmuSectionWidget(tr("内部参数"), m_internalParamEditor, this);
    m_simulationSection = new FmuSectionWidget(tr("仿真"), CreateBlankContent(92), this);

    m_inputParamEditor = new InputParamEditorWidget(this);
    m_inputParamEditor->setMinimumSize(0, 0);
    m_inputParamEditor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_inputParamEditor->setFixedHeight(334);

    m_inputSection = new FmuSectionWidget(tr("输入"), m_inputParamEditor, this);
    m_outputSection = new FmuSectionWidget(tr("输出"), CreateBlankContent(92), this);

    sectionsLayout->addWidget(m_internalSection);
    sectionsLayout->addWidget(m_simulationSection);
    sectionsLayout->addWidget(m_inputSection);
    sectionsLayout->addWidget(m_outputSection);

    m_inputSection->SetExpanded(true);
}

QWidget *FmuParamWidget::CreateBlankContent(int height) const
{
    QWidget *content = new QWidget();
    content->setObjectName(QStringLiteral("BlankSectionContent"));
    content->setMinimumHeight(height);
    content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    return content;
}

QIcon FmuParamWidget::CreateFolderIcon() const
{
    QPixmap pixmap(18, 18);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(QColor(172, 172, 172), 1.4));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath([&]() {
        QPainterPath path;
        path.moveTo(2.5, 5.5);
        path.lineTo(7.0, 5.5);
        path.lineTo(8.5, 7.0);
        path.lineTo(15.5, 7.0);
        path.lineTo(15.0, 14.0);
        path.lineTo(3.0, 14.0);
        path.closeSubpath();
        return path;
    }());

    return QIcon(pixmap);
}

void FmuParamWidget::OnBrowseModelPathTriggered()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("选择FMU模型"),
        QString(),
        tr("FMU模型 (*.fmu);;所有文件 (*.*)"));

    if (!filePath.isEmpty()) {
        m_modelPathEdit->setText(filePath);
        SetSimulationModeRadiosVisible(true);
    }
}
