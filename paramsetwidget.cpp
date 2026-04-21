#include "paramsetwidget.h"

#include "fmuparamwidget.h"
#include "stylesheetloader.h"

#include <QDebug>
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

ParamCategoryContentData::ParamCategoryContentData(const QString &typeValue,
                                                   const QStringList &titleValues,
                                                   const QVariantMap &propertyValues)
    : type(typeValue),
      titles(titleValues),
      properties(propertyValues)
{}

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
          m_contentLayout(nullptr),
          m_contentWidget(nullptr),
          m_expanded(true)
    {
        SetupUI(title);
        SetContentWidget(contentWidget);
        SetExpanded(true);
    }

    void SetContentWidget(QWidget *contentWidget)
    {
        if (!m_contentLayout) {
            return;
        }

        if (!contentWidget) {
            contentWidget = new QWidget(m_contentContainer);
        }

        if (m_contentWidget == contentWidget) {
            return;
        }

        if (m_contentWidget) {
            m_contentLayout->removeWidget(m_contentWidget);
            m_contentWidget->deleteLater();
        }

        m_contentWidget = contentWidget;
        m_contentWidget->setParent(m_contentContainer);
        m_contentLayout->addWidget(m_contentWidget);
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
        m_contentLayout = new QVBoxLayout(m_contentContainer);
        m_contentLayout->setContentsMargins(0, 0, 0, 0);
        m_contentLayout->setSpacing(0);

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
    QVBoxLayout *m_contentLayout;
    QWidget *m_contentWidget;
    bool m_expanded;
};

ParamSetWidget::ParamSetWidget(QWidget *parent)
    : ParamSetWidget(ParamCategoryContentData(), parent)
{}

ParamSetWidget::ParamSetWidget(const ParamCategoryContentData &categoryData, QWidget *parent)
    : QWidget(parent),
      m_moduleImageLabel(nullptr),
      m_moduleNameLabel(nullptr),
      m_moduleDescriptionEdit(nullptr),
      m_contentWidget(nullptr),
      m_contentLayout(nullptr),
      m_fmuParamWidget(nullptr)
{
    SetupUI(categoryData);
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

void ParamSetWidget::SetCategoryContentData(const ParamCategoryContentData &categoryData)
{
    BuildCategoryWidgets(categoryData);
}

void ParamSetWidget::SetupUI(const ParamCategoryContentData &categoryData)
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

    m_contentWidget = new QWidget(scrollArea);
    m_contentWidget->setObjectName(QStringLiteral("ParamSetContent"));
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    m_contentLayout->setSpacing(0);

    m_contentLayout->addWidget(CreateModuleSummaryWidget());
    BuildCategoryWidgets(categoryData);
    m_contentLayout->addStretch();

    scrollArea->setWidget(m_contentWidget);

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

ParamCategoryContentData ParamSetWidget::DefaultCategoryContentData() const
{
    return ParamCategoryContentData(QString(), QStringList{tr("一级分类"), tr("二级分类")});
}

ParamCategoryContentData ParamSetWidget::NormalizeCategoryContentData(const ParamCategoryContentData &categoryData) const
{
    ParamCategoryContentData normalizedData = categoryData;
    if (normalizedData.titles.isEmpty()) {
        normalizedData = DefaultCategoryContentData();
    }

    for (int index = 0; index < normalizedData.titles.size(); ++index) {
        if (normalizedData.titles.at(index).isEmpty()) {
            normalizedData.titles[index] = index == 0
                ? tr("一级分类")
                : (index == 1 ? tr("二级分类") : tr("分类%1").arg(index + 1));
        }
    }

    return normalizedData;
}

ParamCategoryWidget *ParamSetWidget::CreateCategoryWidget(const QString &title,
                                                          QWidget *contentWidget,
                                                          QWidget *parent)
{
    return new ParamCategoryWidget(title, contentWidget, parent);
}

QWidget *ParamSetWidget::CreateTypedContentWidget(const ParamCategoryContentData &categoryData, QWidget *parent)
{
    Q_UNUSED(categoryData.properties);

    const QString type = categoryData.type.trimmed().toLower();
    if (type == QStringLiteral("fmu")) {
        FmuParamWidget *fmuWidget = new FmuParamWidget(parent);
        fmuWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        if (!m_fmuParamWidget) {
            m_fmuParamWidget = fmuWidget;
        }
        return fmuWidget;
    }

    return nullptr;
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

    connect(helpButton, &QPushButton::clicked, this, &ParamSetWidget::OnHelpButtonClicked);
    connect(okButton, &QPushButton::clicked, this, &ParamSetWidget::OnOkButtonClicked);
    connect(applyButton, &QPushButton::clicked, this, &ParamSetWidget::OnApplyButtonClicked);
    connect(cancelButton, &QPushButton::clicked, this, &ParamSetWidget::OnCancelButtonClicked);

    return buttonBar;
}

void ParamSetWidget::BuildCategoryWidgets(const ParamCategoryContentData &categoryData)
{
    if (!m_contentLayout || !m_contentWidget) {
        return;
    }

    ClearCategoryWidgets();
    m_fmuParamWidget = nullptr;

    const ParamCategoryContentData normalizedData = NormalizeCategoryContentData(categoryData);
    int insertIndex = m_contentLayout->count();
    if (insertIndex > 0 && m_contentLayout->itemAt(insertIndex - 1)->spacerItem()) {
        --insertIndex;
    }

    for (int index = 0; index < normalizedData.titles.size(); ++index) {
        QWidget *categoryContent = index == 0 ? CreateTypedContentWidget(normalizedData, m_contentWidget) : nullptr;
        ParamCategoryWidget *categoryWidget = CreateCategoryWidget(normalizedData.titles.at(index),
                                                                   categoryContent,
                                                                   m_contentWidget);
        m_categoryWidgets.append(categoryWidget);
        m_contentLayout->insertWidget(insertIndex + index, categoryWidget);
    }
}

void ParamSetWidget::ClearCategoryWidgets()
{
    if (!m_contentLayout) {
        return;
    }

    for (ParamCategoryWidget *categoryWidget : m_categoryWidgets) {
        m_contentLayout->removeWidget(categoryWidget);
        categoryWidget->deleteLater();
    }
    m_categoryWidgets.clear();
}

void ParamSetWidget::OnHelpButtonClicked()
{
    emit HelpRequested();
    qDebug() << "OnHelpButtonClicked---";
}

void ParamSetWidget::OnOkButtonClicked()
{
    emit Accepted();
    qDebug() << "OnOkButtonClicked---";
}

void ParamSetWidget::OnApplyButtonClicked()
{
    emit Applied();
    qDebug() << "OnApplyButtonClicked---";
}

void ParamSetWidget::OnCancelButtonClicked()
{
    emit Canceled();
    this->close();
}
