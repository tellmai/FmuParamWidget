#include "paramsetwidget.h"

#include "fmuparamwidget.h"
#include "stylesheetloader.h"

#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QTextDocument>
#include <QTextEdit>
#include <QToolButton>
#include <QVBoxLayout>

class ParamCategoryArrowButton : public QToolButton
{
public:
    explicit ParamCategoryArrowButton(QWidget *parent = nullptr)
        : QToolButton(parent)
    {}

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(QPen(QColor(40, 40, 40), 1.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

        const QPointF center = rect().center();
        if (arrowType() == Qt::DownArrow) {
            painter.drawLine(QPointF(center.x() - 4.5, center.y() - 2.5), QPointF(center.x(), center.y() + 3.0));
            painter.drawLine(QPointF(center.x(), center.y() + 3.0), QPointF(center.x() + 4.5, center.y() - 2.5));
        } else {
            painter.drawLine(QPointF(center.x() - 2.5, center.y() - 5.0), QPointF(center.x() + 3.0, center.y()));
            painter.drawLine(QPointF(center.x() + 3.0, center.y()), QPointF(center.x() - 2.5, center.y() + 5.0));
        }
    }
};

class ParamCategoryWidget : public QWidget
{
public:
    explicit ParamCategoryWidget(const QString &title, QWidget *contentWidget, QWidget *parent = nullptr)
        : QWidget(parent),
          m_arrowButton(nullptr),
          m_titleLabel(nullptr),
          m_headerWidget(nullptr),
          m_contentContainer(nullptr),
          m_contentWidget(contentWidget),
          m_expanded(true)
    {
        if (!m_contentWidget) {
            m_contentWidget = new QWidget(this);
        }

        m_contentWidget->setParent(this);
        SetupUI(title);
        SetExpanded(true);
    }

    void SetExpanded(bool expanded)
    {
        m_expanded = expanded;
        m_arrowButton->setArrowType(m_expanded ? Qt::DownArrow : Qt::RightArrow);
        m_contentContainer->setVisible(m_expanded);
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if ((watched == m_headerWidget || watched == m_titleLabel)
            && event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                SetExpanded(!m_expanded);
                return true;
            }
        }

        return QWidget::eventFilter(watched, event);
    }

private:
    void SetupUI(const QString &title)
    {
        setObjectName(QStringLiteral("ParamCategoryWidget"));

        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);

        m_headerWidget = new QWidget(this);
        m_headerWidget->setObjectName(QStringLiteral("CategoryHeader"));
        m_headerWidget->setCursor(Qt::PointingHandCursor);
        m_headerWidget->installEventFilter(this);

        QHBoxLayout *headerLayout = new QHBoxLayout(m_headerWidget);
        headerLayout->setContentsMargins(4, 0, 10, 0);
        headerLayout->setSpacing(4);

        m_arrowButton = new ParamCategoryArrowButton(m_headerWidget);
        m_arrowButton->setObjectName(QStringLiteral("CategoryArrow"));
        m_arrowButton->setFixedSize(18, 28);
        m_arrowButton->setArrowType(Qt::DownArrow);
        m_arrowButton->setCursor(Qt::PointingHandCursor);

        m_titleLabel = new QLabel(title, m_headerWidget);
        m_titleLabel->setObjectName(QStringLiteral("CategoryTitle"));
        m_titleLabel->setCursor(Qt::PointingHandCursor);
        m_titleLabel->installEventFilter(this);

        headerLayout->addWidget(m_arrowButton);
        headerLayout->addWidget(m_titleLabel);
        headerLayout->addStretch();

        m_contentContainer = new QWidget(this);
        m_contentContainer->setObjectName(QStringLiteral("CategoryContent"));
        QVBoxLayout *contentLayout = new QVBoxLayout(m_contentContainer);
        contentLayout->setContentsMargins(0, 0, 0, 0);
        contentLayout->setSpacing(0);
        contentLayout->addWidget(m_contentWidget);

        mainLayout->addWidget(m_headerWidget);
        mainLayout->addWidget(m_contentContainer);

        connect(m_arrowButton, &QToolButton::clicked, this, [this]() {
            SetExpanded(!m_expanded);
        });
    }

    ParamCategoryArrowButton *m_arrowButton;
    QLabel *m_titleLabel;
    QWidget *m_headerWidget;
    QWidget *m_contentContainer;
    QWidget *m_contentWidget;
    bool m_expanded;
};

ParamSetWidget::ParamSetWidget(QWidget *parent)
    : QWidget(parent),
      m_moduleImageLabel(nullptr),
      m_moduleNameLabel(nullptr),
      m_moduleDescriptionEdit(nullptr),
      m_fmuParamWidget(nullptr),
      m_secondCategoryPlaceholder(nullptr)
{
    SetupUI();

    SetModuleImage("C:\\baltamatica\\library\\Baltamulink\\maths\\abs\\abs.svg");
}

void ParamSetWidget::SetModuleImage(const QPixmap &pixmap)
{
    if (!m_moduleImageLabel) {
        return;
    }

    if (pixmap.isNull()) {
        m_moduleImageLabel->clear();
        m_moduleImageLabel->setText(tr("模块图片"));
        return;
    }

    const QSize targetSize = m_moduleImageLabel->contentsRect().size();
    m_moduleImageLabel->setText(QString());
    m_moduleImageLabel->setPixmap(pixmap.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void ParamSetWidget::SetModuleImage(const QString &imagePath)
{
    SetModuleImage(QPixmap(imagePath));
}

void ParamSetWidget::SetModuleName(const QString &name)
{
    if (m_moduleNameLabel) {
        m_moduleNameLabel->setText(name);
    }
}

void ParamSetWidget::SetModuleDescription(const QString &description)
{
    if (m_moduleDescriptionEdit) {
        m_moduleDescriptionEdit->setPlainText(description);
    }
}

void ParamSetWidget::SetupUI()
{
    setObjectName(QStringLiteral("ParamSetWidget"));
    setWindowTitle(tr("模块参数"));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setObjectName(QStringLiteral("ParamSetScrollArea"));
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QWidget *contentWidget = new QWidget(scrollArea);
    contentWidget->setObjectName(QStringLiteral("ParamSetContent"));
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    m_fmuParamWidget = new FmuParamWidget(contentWidget);
    m_fmuParamWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_secondCategoryPlaceholder = CreateSecondCategoryPlaceholder();

    ParamCategoryWidget *firstCategory = new ParamCategoryWidget(tr("一级分类"),
                                                                 m_fmuParamWidget,
                                                                 contentWidget);
    ParamCategoryWidget *secondCategory = new ParamCategoryWidget(tr("二级分类"),
                                                                  m_secondCategoryPlaceholder,
                                                                  contentWidget);

    contentLayout->addWidget(CreateModuleSummaryWidget());
    contentLayout->addWidget(firstCategory);
    contentLayout->addWidget(secondCategory);
    contentLayout->addStretch();

    scrollArea->setWidget(contentWidget);

    mainLayout->addWidget(scrollArea);
    mainLayout->addWidget(CreateButtonBar());

    setStyleSheet(LoadStyleSheet(QStringLiteral(":/qss/paramsetwidget.qss")));

    resize(800, 724);
    setMinimumSize(320, 240);
}

QWidget *ParamSetWidget::CreateModuleSummaryWidget()
{
    QWidget *summaryWidget = new QWidget();
    summaryWidget->setObjectName(QStringLiteral("ModuleSummary"));

    QHBoxLayout *summaryLayout = new QHBoxLayout(summaryWidget);
    summaryLayout->setContentsMargins(23, 11, 24, 12);
    summaryLayout->setSpacing(34);

    m_moduleImageLabel = new QLabel(tr("模块图片"), summaryWidget);
    m_moduleImageLabel->setObjectName(QStringLiteral("ModuleImageBox"));
    m_moduleImageLabel->setAlignment(Qt::AlignCenter);
    m_moduleImageLabel->setScaledContents(false);

    QWidget *textWidget = new QWidget(summaryWidget);
    QVBoxLayout *textLayout = new QVBoxLayout(textWidget);
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(12);

    m_moduleNameLabel = new QLabel(tr("模块名称"), textWidget);
    m_moduleNameLabel->setObjectName(QStringLiteral("ModuleName"));

    m_moduleDescriptionEdit = new QTextEdit(textWidget);
    m_moduleDescriptionEdit->setObjectName(QStringLiteral("ModuleDescription"));
    m_moduleDescriptionEdit->setReadOnly(true);
    m_moduleDescriptionEdit->setFrameShape(QFrame::NoFrame);
    m_moduleDescriptionEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_moduleDescriptionEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_moduleDescriptionEdit->setLineWrapMode(QTextEdit::WidgetWidth);
    m_moduleDescriptionEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_moduleDescriptionEdit->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_moduleDescriptionEdit->document()->setDocumentMargin(0);
    m_moduleDescriptionEdit->setPlainText(tr(
        "模块说明模块说明模块说明模块说明模块说明模块说明模块说明模块说明模块说明模块说明模块说明模块说明"
        "模块说明模块说明模块说明模块说明模块说明模块说明模块。"));

    textLayout->addWidget(m_moduleNameLabel);
    textLayout->addWidget(m_moduleDescriptionEdit, 1);

    summaryLayout->addWidget(m_moduleImageLabel);
    summaryLayout->addWidget(textWidget, 1);

    return summaryWidget;
}

QWidget *ParamSetWidget::CreateSecondCategoryPlaceholder() const
{
    QWidget *placeholder = new QWidget();
    placeholder->setObjectName(QStringLiteral("SecondCategoryPlaceholder"));
    placeholder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    placeholder->setMinimumHeight(220);
    return placeholder;
}

QWidget *ParamSetWidget::CreateButtonBar()
{
    QWidget *buttonBar = new QWidget(this);
    buttonBar->setObjectName(QStringLiteral("ButtonBar"));

    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonBar);
    buttonLayout->setContentsMargins(12, 12, 12, 12);
    buttonLayout->setSpacing(8);

    QPushButton *helpButton = new QPushButton(tr("帮助"), buttonBar);
    QPushButton *okButton = new QPushButton(tr("确定"), buttonBar);
    QPushButton *applyButton = new QPushButton(tr("应用"), buttonBar);
    QPushButton *cancelButton = new QPushButton(tr("取消"), buttonBar);
    okButton->setObjectName(QStringLiteral("PrimaryButton"));

    buttonLayout->addWidget(helpButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(applyButton);
    buttonLayout->addWidget(cancelButton);

    connect(helpButton, &QPushButton::clicked, this, &ParamSetWidget::HelpRequested);
    connect(okButton, &QPushButton::clicked, this, &ParamSetWidget::Accepted);
    connect(applyButton, &QPushButton::clicked, this, &ParamSetWidget::Applied);
    connect(cancelButton, &QPushButton::clicked, this, &ParamSetWidget::Canceled);

    return buttonBar;
}
