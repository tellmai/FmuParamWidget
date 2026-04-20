#include "internalparameditorwidget.h"

#include "ParamItem.h"
#include "borderedtreeview.h"

#include <QFont>
#include <QHeaderView>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QResizeEvent>
#include <QVBoxLayout>

InternalParamEditorWidget::InternalParamEditorWidget(QWidget *parent)
    : QWidget(parent),
      m_treeView(nullptr),
      m_model(nullptr),
      m_delegate(nullptr),
      m_headerView(nullptr),
      m_filterEdit(nullptr)
{
    SetupUI();
    SetupConnections();
    RestoreExpandedRows();
}

void InternalParamEditorWidget::SetupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(6);

    QFont appFont(QStringLiteral("Microsoft YaHei UI"));
    appFont.setPixelSize(14);
    setFont(appFont);

    m_filterEdit = new QLineEdit(this);
    m_filterEdit->setPlaceholderText(QString::fromUtf8("搜索"));
    QPalette filterPalette = m_filterEdit->palette();
    filterPalette.setColor(QPalette::Text, QColor(102, 102, 102));
    filterPalette.setColor(QPalette::PlaceholderText, QColor(169, 169, 169));
    m_filterEdit->setPalette(filterPalette);
    m_filterEdit->setClearButtonEnabled(true);
    m_filterEdit->addAction(CreateSearchIcon(), QLineEdit::LeadingPosition);
    m_filterEdit->setTextMargins(4, 0, 8, 0);
    m_filterEdit->setFixedHeight(28);
    m_filterEdit->setMinimumWidth(0);
    m_filterEdit->setMaximumWidth(500);
    m_filterEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_treeView = new BorderedTreeView(this);
    m_model = new InternalParamModel(this);
    m_delegate = new InternalParamDelegate(this);
    m_headerView = new CustomHeaderView(Qt::Horizontal, m_treeView);
    m_headerView->SetSortableSections(QSet<int>{
        InternalParamModel::ParamColumn,
        InternalParamModel::ValueColumn
    });

    m_treeView->setModel(m_model);
    m_treeView->setItemDelegate(m_delegate);
    m_treeView->setHeader(m_headerView);

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

    setStyleSheet(QStringLiteral(R"(
        InternalParamEditorWidget {
            background-color: #ffffff;
        }
        QLineEdit {
            border: 1px solid #d0d0d0;
            border-radius: 3px;
            padding: 0 8px;
            background-color: #ffffff;
            color: #666666;
            font-family: "Microsoft YaHei UI", "Microsoft YaHei", "Segoe UI";
            font-size: 14px;
            font-weight: 400;
            placeholder-text-color: #a9a9a9;
            selection-background-color: #2f7ed8;
        }
        QLineEdit:focus {
            border-color: #b8b8b8;
        }
        QTreeView {
            border: 1px solid #cccccc;
            background-color: #eeeeee;
            outline: none;
            show-decoration-selected: 0;
            font-size: 14px;
        }
        QTreeView::item {
            padding: 0;
            border: none;
        }
        QTreeView::item:selected {
            background: transparent;
            color: #545454;
        }
        QScrollBar:horizontal, QScrollBar:vertical {
            background: #ffffff;
            border: none;
            margin: 0;
        }
        QScrollBar:horizontal {
            height: 8px;
        }
        QScrollBar:vertical {
            width: 8px;
        }
        QScrollBar::handle:horizontal {
            background: #e1e1e1;
            border-radius: 4px;
            margin: 2px 0;
            min-width: 24px;
        }
        QScrollBar::handle:vertical {
            background: #e1e1e1;
            border-radius: 4px;
            margin: 0 2px;
            min-height: 24px;
        }
        QScrollBar::add-line, QScrollBar::sub-line {
            width: 0;
            height: 0;
        }
        QScrollBar::add-page, QScrollBar::sub-page {
            background: #ffffff;
        }
    )"));

    mainLayout->addWidget(m_filterEdit);
    mainLayout->addWidget(m_treeView);

    ResizeColumnsToAvailableWidth();
    setMinimumSize(0, 0);
}

void InternalParamEditorWidget::SetupConnections()
{
    connect(m_delegate, &InternalParamDelegate::ExpandClicked,
            this, &InternalParamEditorWidget::OnExpandClicked);
    connect(m_filterEdit, &QLineEdit::textChanged,
            this, &InternalParamEditorWidget::OnFilterTextChanged);
    connect(m_headerView, &CustomHeaderView::SortRequested, this, [this](int section, ParamSortMode mode) {
        m_model->SortByColumn(section, mode);
        RestoreExpandedRows();

        const QString filterText = m_filterEdit->text().trimmed();
        if (!filterText.isEmpty()) {
            ApplyFilter(QModelIndex(), filterText);
        }
    });
}

void InternalParamEditorWidget::LoadDefaultData()
{
    m_model->SetupModelData();
    RestoreExpandedRows();
}

void InternalParamEditorWidget::ClearData()
{
    m_model->Clear();
}

void InternalParamEditorWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    ResizeColumnsToAvailableWidth();
}

void InternalParamEditorWidget::ResizeColumnsToAvailableWidth()
{
    if (!m_treeView) {
        return;
    }

    const int availableWidth = m_treeView->viewport()->width();
    if (availableWidth <= 0) {
        return;
    }

    constexpr int MinParamWidth = 220;
    constexpr int MinValueWidth = 220;
    constexpr int MinUnitWidth = 160;
    constexpr int MinDescriptionWidth = 220;
    constexpr int MinTotalWidth = MinParamWidth + MinValueWidth + MinUnitWidth + MinDescriptionWidth;

    if (availableWidth <= MinTotalWidth) {
        m_treeView->setColumnWidth(InternalParamModel::ParamColumn, MinParamWidth);
        m_treeView->setColumnWidth(InternalParamModel::ValueColumn, MinValueWidth);
        m_treeView->setColumnWidth(InternalParamModel::UnitColumn, MinUnitWidth);
        m_treeView->setColumnWidth(InternalParamModel::DescriptionColumn, MinDescriptionWidth);
        return;
    }

    const int paramWidth = qMax(MinParamWidth, availableWidth * 22 / 100);
    const int valueWidth = qMax(MinValueWidth, availableWidth * 29 / 100);
    const int unitWidth = qMax(MinUnitWidth, availableWidth * 22 / 100);
    const int descriptionWidth = qMax(MinDescriptionWidth, availableWidth - paramWidth - valueWidth - unitWidth);

    m_treeView->setColumnWidth(InternalParamModel::ParamColumn, paramWidth);
    m_treeView->setColumnWidth(InternalParamModel::ValueColumn, valueWidth);
    m_treeView->setColumnWidth(InternalParamModel::UnitColumn, unitWidth);
    m_treeView->setColumnWidth(InternalParamModel::DescriptionColumn, descriptionWidth);
}

void InternalParamEditorWidget::ToggleExpandForIndex(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    const QModelIndex paramIndex = index.sibling(index.row(), InternalParamModel::ParamColumn);
    ParamItem *item = m_model->GetItem(paramIndex);
    if (!item || item->m_children.isEmpty()) {
        return;
    }

    m_model->ToggleExpand(paramIndex);

    if (item->m_isExpanded) {
        m_treeView->expand(paramIndex);
    } else {
        m_treeView->collapse(paramIndex);
    }
}

void InternalParamEditorWidget::OnExpandClicked(const QModelIndex &index)
{
    ToggleExpandForIndex(index);
}

void InternalParamEditorWidget::OnFilterTextChanged(const QString &text)
{
    const QString trimmedText = text.trimmed();
    ApplyFilter(QModelIndex(), trimmedText);

    if (trimmedText.isEmpty()) {
        RestoreExpandedRows();
    }
}

void InternalParamEditorWidget::RestoreExpandedRows()
{
    m_treeView->collapseAll();
    RestoreExpandedRows(QModelIndex());
}

void InternalParamEditorWidget::RestoreExpandedRows(const QModelIndex &parentIndex)
{
    const int rows = m_model->rowCount(parentIndex);
    for (int row = 0; row < rows; ++row) {
        const QModelIndex index = m_model->index(row, InternalParamModel::ParamColumn, parentIndex);
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

bool InternalParamEditorWidget::ApplyFilter(const QModelIndex &parentIndex, const QString &text)
{
    bool anyAccepted = false;
    const int rows = m_model->rowCount(parentIndex);

    for (int row = 0; row < rows; ++row) {
        const QModelIndex index = m_model->index(row, InternalParamModel::ParamColumn, parentIndex);
        const bool selfMatches = RowMatchesFilter(index, text);
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

bool InternalParamEditorWidget::RowMatchesFilter(const QModelIndex &index, const QString &text) const
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

QIcon InternalParamEditorWidget::CreateSearchIcon() const
{
    QPixmap pixmap(18, 18);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(QColor(174, 174, 174), 1.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawEllipse(QRectF(4.0, 4.0, 7.5, 7.5));
    painter.drawLine(QPointF(10.5, 10.5), QPointF(14.5, 14.5));

    return QIcon(pixmap);
}
