#include "CustomHeaderView.h"

#include <QFont>
#include <QMouseEvent>
#include <QPainter>
#include <QPolygon>

#include <cstdlib>

namespace {
// 列边缘 6px 作为拖拽热区，绘制时用浅青色提示。
constexpr int ResizeHandleWidth = 5;
constexpr int SortButtonWidth = 16;
}

CustomHeaderView::CustomHeaderView(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent),
      m_currentSortSection(-1),
      m_currentSortMode(ParamSortMode::Default),
      m_pressedSection(-1),
      m_pressedOnResizeHandle(false)
{
    setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    setSectionsClickable(true);
    setSectionsMovable(false);
    setHighlightSections(false);
    setStretchLastSection(false);
    setSectionResizeMode(QHeaderView::Interactive);
    setMinimumSectionSize(50);
    setMouseTracking(true);
}

void CustomHeaderView::SetSortableSections(const QSet<int> &sections)
{
    // 由外部决定哪些列可排序，当前 InputParamEditorWidget 只传入名称列。
    m_sortableSections = sections;
    viewport()->update();
}

bool CustomHeaderView::IsSortableSection(int logicalIndex) const
{
    return m_sortableSections.contains(logicalIndex);
}

ParamSortMode CustomHeaderView::AdvanceSortState(int logicalIndex)
{
    if (!IsSortableSection(logicalIndex)) {
        return ParamSortMode::Default;
    }

    // 排序按钮三态循环：默认 -> 顺序 -> 逆序 -> 默认。
    if (m_currentSortSection != logicalIndex) {
        m_currentSortSection = logicalIndex;
        m_currentSortMode = ParamSortMode::Ascending;
    } else {
        switch (m_currentSortMode) {
        case ParamSortMode::Default:
            m_currentSortMode = ParamSortMode::Ascending;
            break;
        case ParamSortMode::Ascending:
            m_currentSortMode = ParamSortMode::Descending;
            break;
        case ParamSortMode::Descending:
            m_currentSortMode = ParamSortMode::Default;
            m_currentSortSection = -1;
            break;
        }
    }

    viewport()->update();
    return m_currentSortMode;
}

ParamSortMode CustomHeaderView::SortMode() const
{
    return m_currentSortMode;
}

int CustomHeaderView::SortSection() const
{
    return m_currentSortSection;
}

void CustomHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    if (!rect.isValid()) {
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    painter->fillRect(rect, QColor(250, 250, 250));

    painter->setPen(QColor(204, 204, 204));
    painter->drawRect(rect.adjusted(0, 0, -1, -1));

    painter->setPen(QColor(204, 204, 204));
    painter->drawLine(rect.bottomLeft(), rect.bottomRight());

    // 青色区域是列宽拖拽热区，对应交互说明里的列宽调整器。
    const QRect resizeRect(rect.right() - ResizeHandleWidth + 1, rect.top(), ResizeHandleWidth, rect.height());
    painter->fillRect(resizeRect, QColor(0, 188, 212, 34));

    const bool sortable = IsSortableSection(logicalIndex);
    const QString text = model()
        ? model()->headerData(logicalIndex, orientation(), Qt::DisplayRole).toString()
        : QString();

    QFont headerFont = painter->font();
    headerFont.setFamily(QStringLiteral("Microsoft YaHei UI"));
    headerFont.setPixelSize(15);
    headerFont.setWeight(QFont::DemiBold);
    painter->setFont(headerFont);
    painter->setPen(QColor(66, 66, 66));

    const int rightPadding = sortable ? SortButtonWidth + ResizeHandleWidth + 8 : ResizeHandleWidth + 10;
    const QRect textRect = rect.adjusted(6, 0, -rightPadding, 0);
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, painter->fontMetrics().elidedText(text, Qt::ElideRight, textRect.width()));

    if (sortable) {
        // 只有可排序列显示双三角按钮。
        const QRect sortRect(rect.right() - ResizeHandleWidth - SortButtonWidth, rect.top(), SortButtonWidth, rect.height());
        const ParamSortMode mode = m_currentSortSection == logicalIndex ? m_currentSortMode : ParamSortMode::Default;
        DrawSortButton(painter, sortRect, mode);
    }

    painter->restore();
}

QSize CustomHeaderView::sectionSizeFromContents(int logicalIndex) const
{
    QSize size = QHeaderView::sectionSizeFromContents(logicalIndex);
    size.setHeight(28);
    return size;
}

void CustomHeaderView::mousePressEvent(QMouseEvent *event)
{
    // 记录按下时的位置，用于 release 时区分排序点击和拖拽列宽。
    m_pressedSection = logicalIndexAt(event->pos());
    m_pressedOnResizeHandle = IsOnResizeHandle(event->pos());
    QHeaderView::mousePressEvent(event);
}

void CustomHeaderView::mouseReleaseEvent(QMouseEvent *event)
{
    const int releasedSection = logicalIndexAt(event->pos());
    const bool shouldSort = event->button() == Qt::LeftButton
        && !m_pressedOnResizeHandle
        && m_pressedSection == releasedSection
        && IsSortableSection(releasedSection);

    QHeaderView::mouseReleaseEvent(event);

    if (shouldSort) {
        // 确认不是拖拽后才推进排序状态并通知外部模型排序。
        emit SortRequested(releasedSection, AdvanceSortState(releasedSection));
        event->accept();
    }

    m_pressedSection = -1;
    m_pressedOnResizeHandle = false;
}

void CustomHeaderView::mouseMoveEvent(QMouseEvent *event)
{
    // 鼠标靠近列边缘时显示水平调整大小指针。
    if (IsOnResizeHandle(event->pos())) {
        viewport()->setCursor(Qt::SplitHCursor);
    } else {
        viewport()->unsetCursor();
    }

    QHeaderView::mouseMoveEvent(event);
}

void CustomHeaderView::leaveEvent(QEvent *event)
{
    viewport()->unsetCursor();
    QHeaderView::leaveEvent(event);
}

void CustomHeaderView::DrawSortButton(QPainter *painter, const QRect &rect, ParamSortMode mode) const
{
    // 用上下两个三角表示顺序/逆序，当前模式加深颜色。
    const QPoint center = rect.center();
    const int upY = center.y() - 5;
    const int downY = center.y() + 2;

    const QPolygon upTriangle({
        QPoint(center.x(), upY),
        QPoint(center.x() - 4, upY + 5),
        QPoint(center.x() + 4, upY + 5)
    });

    const QPolygon downTriangle({
        QPoint(center.x(), downY + 5),
        QPoint(center.x() - 4, downY),
        QPoint(center.x() + 4, downY)
    });

    const QColor active(146, 146, 146);
    const QColor inactive(218, 218, 218);

    painter->setPen(Qt::NoPen);
    painter->setBrush(mode == ParamSortMode::Ascending ? active : inactive);
    painter->drawPolygon(upTriangle);
    painter->setBrush(mode == ParamSortMode::Descending ? active : inactive);
    painter->drawPolygon(downTriangle);
}

bool CustomHeaderView::IsOnResizeHandle(const QPoint &pos) const
{
    const int logicalIndex = logicalIndexAt(pos);
    if (logicalIndex < 0) {
        return false;
    }

    const int sectionRight = sectionViewportPosition(logicalIndex) + sectionSize(logicalIndex);
    return std::abs(pos.x() - sectionRight) <= ResizeHandleWidth;
}
