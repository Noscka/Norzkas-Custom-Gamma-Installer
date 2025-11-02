#pragma once

#include <QtWidgets/QLayout>
#include <QtWidgets>
#include <QLayout>
#include <QRect>
#include <QStyle>

class FlowLayout : public QLayout
{
public:
	explicit FlowLayout(QWidget* parent, int margin = -1, int hSpacing = -1, int vSpacing = -1) : QLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing)
	{
		setContentsMargins(margin, margin, margin, margin);
	}

	explicit FlowLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1) : m_hSpace(hSpacing), m_vSpace(vSpacing)
	{
		setContentsMargins(margin, margin, margin, margin);
	}

	~FlowLayout()
	{
		QLayoutItem* item;
		while ((item = takeAt(0)))
			delete item;
	}

	void addItem(QLayoutItem* item) override
	{
		itemList.append(item);
	}

	int horizontalSpacing() const
	{
		if (m_hSpace >= 0)
		{
			return m_hSpace;
		}
		else
		{
			return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
		}
	}

	int verticalSpacing() const
	{
		if (m_vSpace >= 0)
		{
			return m_vSpace;
		}
		else
		{
			return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
		}
	}

	Qt::Orientations expandingDirections() const override
	{
		return { };
	}

	bool hasHeightForWidth() const override
	{
		return true;
	}

	int heightForWidth(int width) const override
	{
		int height = doLayout(QRect(0, 0, width, 0), true);
		return height;
	}

	int count() const override
	{
		return itemList.size();
	}

	QLayoutItem* itemAt(int index) const override
	{
		return itemList.value(index);
	}

	QSize minimumSize() const override
	{
		QSize size;
		for (const QLayoutItem* item : std::as_const(itemList))
			size = size.expandedTo(item->minimumSize());

		const QMargins margins = contentsMargins();
		size += QSize(margins.left() + margins.right(), margins.top() + margins.bottom());

		auto layoutSize = this->contentsRect();
		size.setHeight(heightForWidth(layoutSize.width()));

		return size;
	}

	void setGeometry(const QRect& rect) override
	{
		update();

		QLayout::setGeometry(rect);
		doLayout(rect, false);
	}

	QSize sizeHint() const override
	{
		return minimumSize();
	}

	QLayoutItem* takeAt(int index) override
	{
		if (index >= 0 && index < itemList.size())
			return itemList.takeAt(index);
		return nullptr;
	}

private:
	int doLayout(const QRect& rect, bool testOnly) const
	{
		int left, top, right, bottom;
		getContentsMargins(&left, &top, &right, &bottom);
		QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
		int x = effectiveRect.x();
		int y = effectiveRect.y();
		int lineHeight = 0;

		for (QLayoutItem* item : std::as_const(itemList))
		{
			const QWidget* wid = item->widget();
			int spaceX = horizontalSpacing();
			if (spaceX == -1)
				spaceX = wid->style()->layoutSpacing(QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);
			int spaceY = verticalSpacing();
			if (spaceY == -1)
				spaceY = wid->style()->layoutSpacing(QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);
			int nextX = x + item->sizeHint().width() + spaceX;
			if (nextX - spaceX > effectiveRect.right() && lineHeight > 0)
			{
				x = effectiveRect.x();
				y = y + lineHeight + spaceY;
				nextX = x + item->sizeHint().width() + spaceX;
				lineHeight = 0;
			}

			if (!testOnly)
				item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));

			x = nextX;
			lineHeight = qMax(lineHeight, item->sizeHint().height());
		}
		return y + lineHeight - rect.y() + bottom;
	}


	int smartSpacing(QStyle::PixelMetric pm) const
	{
		QObject* parent = this->parent();
		if (!parent)
		{
			return -1;
		}
		else if (parent->isWidgetType())
		{
			QWidget* pw = static_cast<QWidget*>(parent);
			return pw->style()->pixelMetric(pm, nullptr, pw);
		}
		else
		{
			return static_cast<QLayout*>(parent)->spacing();
		}
	}

	QList<QLayoutItem*> itemList;
	int m_hSpace;
	int m_vSpace;
};