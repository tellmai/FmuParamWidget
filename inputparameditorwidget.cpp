#include "inputparameditorwidget.h"

#include "borderedtreeview.h"
#include "stylesheetloader.h"

#include <QAction>
#include <QFile>
#include <QFont>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonDocument>
#include <QMessageBox>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QResizeEvent>
#include <QTextStream>
#include <QVBoxLayout>

InputParamEditorWidget::InputParamEditorWidget(QWidget *parent)
    : QWidget(parent),
      m_treeView(nullptr),
      m_model(nullptr),
      m_delegate(nullptr),
      m_headerView(nullptr),
      m_filterEdit(nullptr),
      m_filterLabel(nullptr)
{
    SetupUI();
    SetupConnections();
    SetupModelData();
}

void InputParamEditorWidget::SetupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(6);

    // 整体字号按参考图放大，搜索框、表头和单元格再分别细调。
    QFont appFont(QStringLiteral("Microsoft YaHei UI"));
    appFont.setPixelSize(14);
    setFont(appFont);

    // 搜索框使用自绘放大镜图标和 palette 设置占位文字色，贴近设计稿。
    m_filterEdit = new QLineEdit(this);
    m_filterEdit->setPlaceholderText(tr("搜索"));
    QPalette filterPalette = m_filterEdit->palette();
    filterPalette.setColor(QPalette::Text, QColor(102, 102, 102));
    filterPalette.setColor(QPalette::PlaceholderText, QColor(169, 169, 169));
    m_filterEdit->setPalette(filterPalette);
    m_filterEdit->setClearButtonEnabled(true);
    m_filterEdit->addAction(CreateSearchIcon(), QLineEdit::LeadingPosition);
    m_filterEdit->setTextMargins(4, 0, 8, 0);
    m_filterEdit->setFixedSize(304, 24);
    m_filterEdit->setMinimumWidth(0);
    m_filterEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_treeView = new BorderedTreeView(this);
    m_model = new InputParamModel(this);
    m_delegate = new InputParamDelegate(this);
    m_headerView = new CustomHeaderView(Qt::Horizontal, m_treeView);
    // 交付要求里当前只保留名称列排序，开始列不开放排序入口。
    m_headerView->SetSortableSections(QSet<int>{InputParamModel::NameColumn});

    m_treeView->setModel(m_model);
    m_treeView->setItemDelegate(m_delegate);
    m_treeView->setHeader(m_headerView);

    // 展开箭头由 m_delegate 自绘，禁用 QTreeView 默认树形装饰。
    m_treeView->setRootIsDecorated(false);
    m_treeView->setItemsExpandable(false);
    m_treeView->setExpandsOnDoubleClick(false);
    m_treeView->setIndentation(0);
    m_treeView->setUniformRowHeights(true);
    m_treeView->setAllColumnsShowFocus(true);
    m_treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_treeView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    m_treeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_treeView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_treeView->setAutoScroll(false);

    setStyleSheet(LoadStyleSheet(QStringLiteral(":/qss/inputparameditorwidget.qss")));

    mainLayout->addWidget(m_filterEdit);
    mainLayout->addWidget(m_treeView);

    ResizeColumnsToAvailableWidth();
    setMinimumSize(0, 0);
}

void InputParamEditorWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    ResizeColumnsToAvailableWidth();
}

void InputParamEditorWidget::ResizeColumnsToAvailableWidth()
{
    if (!m_treeView) {
        return;
    }

    const int availableWidth = m_treeView->viewport()->width();
    if (availableWidth <= 0) {
        return;
    }

    constexpr int MinNameWidth = 260;
    constexpr int MinVisibleWidth = 120;
    constexpr int MinStartWidth = 120;
    constexpr int MinUnitWidth = 100;
    constexpr int MinTotalWidth = MinNameWidth + MinVisibleWidth + MinStartWidth + MinUnitWidth;

    if (availableWidth <= MinTotalWidth) {
        m_treeView->setColumnWidth(InputParamModel::NameColumn, MinNameWidth);
        m_treeView->setColumnWidth(InputParamModel::VisibleColumn, MinVisibleWidth);
        m_treeView->setColumnWidth(InputParamModel::StartColumn, MinStartWidth);
        m_treeView->setColumnWidth(InputParamModel::UnitColumn, MinUnitWidth);
        return;
    }

    const int nameWidth = qMax(MinNameWidth, availableWidth * 52 / 100);
    const int visibleWidth = qMax(MinVisibleWidth, availableWidth * 18 / 100);
    const int startWidth = qMax(MinStartWidth, availableWidth * 15 / 100);
    const int unitWidth = qMax(MinUnitWidth, availableWidth - nameWidth - visibleWidth - startWidth);

    m_treeView->setColumnWidth(InputParamModel::NameColumn, nameWidth);
    m_treeView->setColumnWidth(InputParamModel::VisibleColumn, visibleWidth);
    m_treeView->setColumnWidth(InputParamModel::StartColumn, startWidth);
    m_treeView->setColumnWidth(InputParamModel::UnitColumn, unitWidth);
}

void InputParamEditorWidget::SetupConnections()
{
    connect(m_delegate, &InputParamDelegate::ExpandClicked,
            this, &InputParamEditorWidget::OnExpandClicked);
    connect(m_delegate, &InputParamDelegate::RowDoubleClicked,
            this, &InputParamEditorWidget::OnRowDoubleClicked);
    connect(m_filterEdit, &QLineEdit::textChanged,
            this, &InputParamEditorWidget::OnFilterTextChanged);
    connect(m_headerView, &CustomHeaderView::SortRequested, this, [this](int section, ParamSortMode mode) {
        // 排序会 reset 模型，之后需要重新把原本展开的节点展开出来。
        m_model->SortByColumn(section, mode);
        RestoreExpandedRows();

        const QString filterText = m_filterEdit->text().trimmed();
        if (!filterText.isEmpty()) {
            ApplyFilter(QModelIndex(), filterText);
        }
    });
}

void InputParamEditorWidget::SetupModelData()
{
    RestoreExpandedRows();
}

QJsonObject InputParamEditorWidget::GetModelData() const
{
    return m_model->ToJson();
}

QString InputParamEditorWidget::GetModelDataString() const
{
    return m_model->ToJsonString();
}

bool InputParamEditorWidget::SetModelData(const QJsonObject &json)
{
    m_model->FromJson(json);
    RestoreExpandedRows();
    return true;
}

bool InputParamEditorWidget::SetModelDataFromString(const QString &jsonString)
{
    const bool loaded = m_model->FromJsonString(jsonString);
    if (loaded) {
        RestoreExpandedRows();
    }
    return loaded;
}

void InputParamEditorWidget::LoadDefaultData()
{
    m_model->SetupModelData();
    RestoreExpandedRows();
}

void InputParamEditorWidget::ClearData()
{
    m_model->Clear();
}

void InputParamEditorWidget::SaveToFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("保存失败"),
                             tr("无法打开文件: %1\n错误: %2").arg(filePath, file.errorString()));
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << GetModelDataString();
}

void InputParamEditorWidget::LoadFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("加载失败"),
                             tr("无法打开文件: %1\n错误: %2").arg(filePath, file.errorString()));
        return;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    const QString jsonString = in.readAll();

    if (!SetModelDataFromString(jsonString)) {
        QMessageBox::warning(this, tr("加载失败"),
                             tr("文件格式错误: %1").arg(filePath));
    }
}

void InputParamEditorWidget::ToggleExpandForIndex(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    // 展开状态以名称列索引为准，避免点击其它列时拿到错误 parent/row。
    const QModelIndex nameIndex = index.sibling(index.row(), InputParamModel::NameColumn);
    ParamItem *item = m_model->GetItem(nameIndex);
    if (!item || item->m_children.isEmpty()) {
        return;
    }

    m_model->ToggleExpand(nameIndex);

    if (item->m_isExpanded) {
        m_treeView->expand(nameIndex);
    } else {
        m_treeView->collapse(nameIndex);
    }
}

void InputParamEditorWidget::OnExpandClicked(const QModelIndex &index)
{
    ToggleExpandForIndex(index);
}

void InputParamEditorWidget::OnRowDoubleClicked(const QModelIndex &index)
{
    ToggleExpandForIndex(index);
}

void InputParamEditorWidget::OnFilterTextChanged(const QString &text)
{
    const QString trimmedText = text.trimmed();
    ApplyFilter(QModelIndex(), trimmedText);

    if (trimmedText.isEmpty()) {
        RestoreExpandedRows();
    }
}

void InputParamEditorWidget::RestoreExpandedRows()
{
    m_treeView->collapseAll();
    RestoreExpandedRows(QModelIndex());
}

void InputParamEditorWidget::RestoreExpandedRows(const QModelIndex &parentIndex)
{
    const int rows = m_model->rowCount(parentIndex);
    for (int row = 0; row < rows; ++row) {
        const QModelIndex index = m_model->index(row, InputParamModel::NameColumn, parentIndex);
        ParamItem *item = m_model->GetItem(index);
        if (!item || item->m_children.isEmpty()) {
            continue;
        }

        if (item->m_isExpanded) {
            m_treeView->expand(index);
            RestoreExpandedRows(index);
        } else {
            m_treeView->collapse(index);
        }
    }
}

bool InputParamEditorWidget::ApplyFilter(const QModelIndex &parentIndex, const QString &text)
{
    bool anyAccepted = false;
    const int rows = m_model->rowCount(parentIndex);

    for (int row = 0; row < rows; ++row) {
        const QModelIndex index = m_model->index(row, InputParamModel::NameColumn, parentIndex);
        const bool selfMatches = RowMatchesFilter(index, text);
        // 先递归过滤子节点，再决定当前父节点是否需要保留。
        const bool childMatches = ApplyFilter(index, text);
        const bool accepted = text.isEmpty() || selfMatches || childMatches;

        m_treeView->setRowHidden(row, parentIndex, !accepted);

        if (!text.isEmpty() && childMatches) {
            m_treeView->expand(index);
        }

        anyAccepted = anyAccepted || accepted;
    }

    return anyAccepted;
}

bool InputParamEditorWidget::RowMatchesFilter(const QModelIndex &index, const QString &text) const
{
    if (text.isEmpty()) {
        return true;
    }

    for (int column = 0; column < m_model->columnCount(); ++column) {
        const QString value = m_model->data(index.sibling(index.row(), column), Qt::DisplayRole).toString();
        if (value.contains(text, Qt::CaseInsensitive)) {
            return true;
        }
    }

    return false;
}

QIcon InputParamEditorWidget::CreateSearchIcon() const
{
    // 使用 pixmap 绘制简单线框图标，避免额外引入资源文件。
    QPixmap pixmap(18, 18);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(QColor(103, 103, 103), 1.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawEllipse(QRectF(4.0, 4.0, 7.5, 7.5));
    painter.drawLine(QPointF(10.5, 10.5), QPointF(14.5, 14.5));

    return QIcon(pixmap);
}
