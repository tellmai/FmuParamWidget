#include "internalparamdelegate.h"

#include "ParamItem.h"
#include "internalparammodel.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QSpinBox>

namespace {
// 表格单行高度，和交付要求保持一致。QTreeView 通过 sizeHint 使用这个高度。
constexpr int RowHeight = 24;
// 每深入一级参数树，名称列左侧增加的缩进距离。
constexpr int LevelIndent = 18;
// 展开/折叠箭头的绘制区域尺寸，保持小尺寸以适配 24px 行高。
constexpr int ArrowSize = 10;
// 普通单元格左右文字留白，避免文字贴近网格线。
constexpr int CellPadding = 5;
// 一级参数名称距离名称列左边框的基础留白。
constexpr int LeftPadding = 4;
// 名称列中预留给展开/折叠箭头的宽度。
constexpr int ArrowBoxWidth = 14;
// 二级及更深层级参数在缩进分隔线右侧继续保留的文字间距。
constexpr int ChildTextPadding = 8;
// 二级及更深层级参数的箭头相对缩进分隔线右侧的偏移量。
constexpr int ChildArrowPadding = 2;
// 表格网格线颜色，对应设计稿 border: 1px solid #cccccc。
const QColor GridColor(204, 204, 204);
// 表格正文文字颜色。
const QColor TextColor(86, 86, 86);
// 不可编辑单元格背景色，对应 #e1e1e1 叠加 0.4 透明度。
const QColor DisabledCellColor(225, 225, 225, 102);
}

InternalParamDelegate::InternalParamDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}

void InternalParamDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    // 先铺白底，再叠加模型给出的背景色。这样能模拟 CSS opacity，
    // 同时避免半透明背景透出平台默认选区颜色。
    painter->fillRect(option.rect, Qt::white);

    QBrush backgroundBrush = index.data(Qt::BackgroundRole).value<QBrush>();
    if (backgroundBrush.style() == Qt::NoBrush) {
        backgroundBrush = QBrush(DisabledCellColor);
    }
    painter->fillRect(option.rect, backgroundBrush);

    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, QColor(218, 236, 255, 130));
    }

    const bool sectionHeader = IsSectionHeader(index);
    QFont font = opt.font;
    font.setFamily(QStringLiteral("Microsoft YaHei UI"));
    font.setPixelSize(14);
    font.setBold(false);
    painter->setFont(font);

    if (index.column() == InternalParamModel::ParamColumn) {
        const int depth = DepthForIndex(index);
        const bool itemHasChildren = HasChildren(index);

        if (depth > 0) {
            // 子级名称列左侧是“合并缩进区”：只显示白底和竖向层级线，
            // 不绘制每个单元格的完整横线，视觉上更接近设计图里的层级表格。
            const int indentWidth = depth * LevelIndent + 6;
            const QRect indentRect(option.rect.left(), option.rect.top(), indentWidth, option.rect.height());
            painter->fillRect(indentRect, Qt::white);
            painter->setPen(GridColor);
            for (int level = 1; level <= depth; ++level) {
                // 多级参数每一级都绘制一条竖线，帮助用户辨认当前节点深度。
                const int guideX = option.rect.left() + level * LevelIndent + 6;
                painter->drawLine(QPoint(guideX, option.rect.top()), QPoint(guideX, option.rect.bottom()));
            }
        }

        if (itemHasChildren) {
            // QTreeView 默认树形装饰已经关闭，这里用线框 chevron 自绘展开状态。
            const QRect indicatorRect = ArrowRect(option, index);
            const QPoint center = indicatorRect.center();
            painter->setPen(QPen(QColor(92, 92, 92), 1.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            if (index.data(Qt::UserRole + 4).toBool()) {
                painter->drawLine(QPoint(center.x() - 3, center.y() - 1), QPoint(center.x(), center.y() + 3));
                painter->drawLine(QPoint(center.x(), center.y() + 3), QPoint(center.x() + 3, center.y() - 1));
            } else {
                painter->drawLine(QPoint(center.x() - 1, center.y() - 3), QPoint(center.x() + 3, center.y()));
                painter->drawLine(QPoint(center.x() + 3, center.y()), QPoint(center.x() - 1, center.y() + 3));
            }
        }

        // 子级文字不能直接使用 depth * LevelIndent 作为起点，否则会贴近缩进竖线。
        // 这里先算出缩进区右边界，再额外增加 ChildTextPadding。
        const int indentWidth = depth > 0 ? depth * LevelIndent + 6 : 0;
        const int contentLeft = option.rect.left()
            + (depth > 0 ? indentWidth + ChildTextPadding : LeftPadding);
        const int textLeft = contentLeft + (itemHasChildren ? ArrowBoxWidth + 6 : 0);
        const QRect textRect(textLeft, option.rect.top(), option.rect.right() - textLeft - CellPadding, option.rect.height());
        painter->setPen(sectionHeader ? QColor(74, 74, 74) : TextColor);
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter,
                          painter->fontMetrics().elidedText(index.data(Qt::DisplayRole).toString(),
                                                            Qt::ElideRight,
                                                            textRect.width()));
    } else {
        const QRect textRect = option.rect.adjusted(CellPadding, 0, -CellPadding, 0);
        painter->setPen(TextColor);
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter,
                          painter->fontMetrics().elidedText(index.data(Qt::DisplayRole).toString(),
                                                            Qt::ElideRight,
                                                            textRect.width()));
    }

    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setPen(GridColor);

    if (index.column() == InternalParamModel::ParamColumn
        && DepthForIndex(index) > 0
        && !IsLastChildInParent(index)) {
        // 子级名称列缩进区内不画横线，让同一层级的缩进区域看起来是连续的。
        const int indentWidth = DepthForIndex(index) * LevelIndent + 6;
        painter->drawLine(QPoint(option.rect.left() + indentWidth, option.rect.bottom()),
                          option.rect.bottomRight());
    } else {
        painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
    }

    painter->drawLine(option.rect.topRight(), option.rect.bottomRight());

    if (index.column() == InternalParamModel::ParamColumn) {
        painter->drawLine(option.rect.topLeft(), option.rect.bottomLeft());
    }

    painter->restore();
}

QSize InternalParamDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    // 统一控制所有行高，避免不同编辑器或字体导致行高抖动。
    size.setHeight(RowHeight);
    return size;
}

bool InternalParamDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index)
{
    Q_UNUSED(model);

    if (!index.isValid()) {
        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }

    if (event->type() == QEvent::MouseButtonPress
        && index.column() == InternalParamModel::ParamColumn
        && HasChildren(index)) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        // 只有点击箭头热区才触发展开/折叠，点击名称文本仍交给视图处理选择。
        if (mouseEvent->button() == Qt::LeftButton && ArrowRect(option, index).contains(mouseEvent->pos())) {
            emit ExpandClicked(index);
            return true;
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

QWidget *InternalParamDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                             const QModelIndex &index) const
{
    Q_UNUSED(option);

    if (index.column() != InternalParamModel::ValueColumn) {
        return nullptr;
    }

    ParamItem *item = static_cast<ParamItem*>(index.internalPointer());
    if (!item || !item->m_editable) {
        return nullptr;
    }

    // 内部参数只允许编辑“值”列，并根据 FMU 参数的数据类型创建对应编辑器。
    switch (item->m_dataType) {
    case ParamDataType::CheckBox:
        return new QCheckBox(parent);
    case ParamDataType::ComboBox: {
        QComboBox *combo = new QComboBox(parent);
        combo->addItems({QStringLiteral("auto"), QStringLiteral("manual"), QStringLiteral("semi-auto")});
        return combo;
    }
    case ParamDataType::SpinBox: {
        QSpinBox *spin = new QSpinBox(parent);
        spin->setRange(-100000000, 100000000);
        return spin;
    }
    case ParamDataType::DoubleSpinBox: {
        QDoubleSpinBox *doubleSpin = new QDoubleSpinBox(parent);
        doubleSpin->setRange(-100000000.0, 100000000.0);
        doubleSpin->setDecimals(6);
        return doubleSpin;
    }
    case ParamDataType::LineEdit:
    default:
        return new QLineEdit(parent);
    }
}

void InternalParamDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    const QString value = index.model()->data(index, Qt::DisplayRole).toString();

    if (QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor)) {
        lineEdit->setText(value);
    } else if (QCheckBox *checkBox = qobject_cast<QCheckBox*>(editor)) {
        checkBox->setChecked(value.compare(QStringLiteral("true"), Qt::CaseInsensitive) == 0 || value == QStringLiteral("1"));
    } else if (QComboBox *combo = qobject_cast<QComboBox*>(editor)) {
        combo->setCurrentText(value);
    } else if (QSpinBox *spin = qobject_cast<QSpinBox*>(editor)) {
        spin->setValue(value.toInt());
    } else if (QDoubleSpinBox *doubleSpin = qobject_cast<QDoubleSpinBox*>(editor)) {
        doubleSpin->setValue(value.toDouble());
    }
}

void InternalParamDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                         const QModelIndex &index) const
{
    QString value;

    if (QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor)) {
        value = lineEdit->text();
    } else if (QCheckBox *checkBox = qobject_cast<QCheckBox*>(editor)) {
        value = checkBox->isChecked() ? QStringLiteral("true") : QStringLiteral("false");
    } else if (QComboBox *combo = qobject_cast<QComboBox*>(editor)) {
        value = combo->currentText();
    } else if (QSpinBox *spin = qobject_cast<QSpinBox*>(editor)) {
        value = QString::number(spin->value());
    } else if (QDoubleSpinBox *doubleSpin = qobject_cast<QDoubleSpinBox*>(editor)) {
        value = QString::number(doubleSpin->value(), 'g', 12);
    }

    model->setData(index, value, Qt::EditRole);
}

void InternalParamDelegate::updateEditorGeometry(QWidget *editor,
                                                 const QStyleOptionViewItem &option,
                                                 const QModelIndex &index) const
{
    Q_UNUSED(index);
    editor->setGeometry(option.rect.adjusted(3, 3, -3, -3));
}

bool InternalParamDelegate::IsSectionHeader(const QModelIndex &index) const
{
    return index.data(Qt::UserRole).toBool();
}

int InternalParamDelegate::DepthForIndex(const QModelIndex &index) const
{
    // QModelIndex 的 parent 链就是树模型的层级链，向上数 parent 即可得到深度。
    int depth = 0;
    QModelIndex parentIndex = index.parent();
    while (parentIndex.isValid()) {
        ++depth;
        parentIndex = parentIndex.parent();
    }
    return depth;
}

QRect InternalParamDelegate::ArrowRect(const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const
{
    const int depth = DepthForIndex(index);
    // 子级箭头也放在缩进区右侧，避免贴近左侧网格线或层级竖线。
    const int indentWidth = depth > 0 ? depth * LevelIndent + 6 : 0;
    const int contentLeft = option.rect.left()
        + (depth > 0 ? indentWidth + ChildArrowPadding : LeftPadding);
    const int left = contentLeft + (ArrowBoxWidth - ArrowSize) / 2;
    return QRect(left, option.rect.top() + (option.rect.height() - ArrowSize) / 2, ArrowSize, ArrowSize);
}

bool InternalParamDelegate::HasChildren(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return false;
    }

    const QModelIndex paramIndex = index.sibling(index.row(), InternalParamModel::ParamColumn);
    return index.model()->rowCount(paramIndex) > 0;
}

bool InternalParamDelegate::IsLastChildInParent(const QModelIndex &index) const
{
    if (!index.isValid() || !index.parent().isValid()) {
        return false;
    }

    return index.row() == index.model()->rowCount(index.parent()) - 1;
}
