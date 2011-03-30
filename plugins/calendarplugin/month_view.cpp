#include <QPainter>

#include "month_view.h"

using namespace Calendar;

QSize MonthHeader::sizeHint() const {
	return QSize(0, 20);
}

void MonthHeader::paintEvent(QPaintEvent *event) {
	// fill all in light blue
	QPainter painter(this);
	painter.fillRect(rect(), QColor(220, 220, 255));

	// bottom line
	QPen pen = painter.pen();
	pen.setColor(QColor(200, 200, 255));
	painter.setPen(pen);
	painter.drawLine(0, rect().bottom(), rect().right(), rect().bottom());

	// text
	pen.setColor(QColor(150, 150, 255));
	painter.setPen(pen);

	int containWidth = rect().width();
	for (int i = 0; i < 7; ++i) {
		QRect r(QPoint((i * containWidth) / 7, 0), QPoint(((i + 1) * containWidth) / 7 - 1 + 2, rect().height())); // +2 is a vertical correction to not be stucked to the top line
		painter.drawText(r, Qt::AlignHCenter | Qt::AlignTop, QDate::shortDayName(i + 1));
	}
}

/////////////////////////////////////////////////////////////

MonthView::MonthView(QWidget *parent) : View(parent) {
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

int MonthView::topHeaderHeight() const {
	return 0;
}

int MonthView::leftHeaderWidth() const {
	return 0;
}

QWidget *MonthView::createHeaderWidget(QWidget *parent) {
	MonthHeader *widget = new MonthHeader(parent);
	widget->setFirstDate(m_firstDate);
	return widget;
}

void MonthView::paintBody(QPainter *painter, const QRect &visibleRect) {
	painter->fillRect(visibleRect, Qt::white);
	QPen pen = painter->pen();
	pen.setColor(QColor(200, 200, 200));
	pen.setCapStyle(Qt::FlatCap);
	painter->setPen(pen);

	// get the very first day of the grid (not necessarily the first day of the month)
	QDate firstDay = m_firstDate.addDays(-m_firstDate.dayOfWeek() + 1);

	// get the very last day of the grid (not necessarily the last day of the month)
	QDate lastDay = m_firstDate.addDays(m_firstDate.daysInMonth() - 1);
	lastDay = lastDay.addDays(7 - lastDay.dayOfWeek());

	// compute week count in way to englobe all month days
	QDate now = QDate::currentDate();
	int weekCount = 0;
	int focusRow = -1;
	for (QDate date = firstDay; date <= lastDay; date = date.addDays(7)) {
		if (now >= date && now < date.addDays(7)) {
			focusRow = weekCount;
		}
		weekCount++;
	}

	int horiAmount = visibleRect.width() - 6; // total width without lines width
	int vertiAmount = visibleRect.height() - weekCount + 1; // total height without lines height

	// draw current day?
	if (focusRow >= 0) {
		int i = now.dayOfWeek();
		int j = focusRow;
		QRect r((i * horiAmount) / 7 + i, (j * vertiAmount) / weekCount + j,
				((i + 1) * horiAmount) / 7 - (i * horiAmount) / 7,
				((j + 1) * vertiAmount) / weekCount - (j * vertiAmount) / weekCount);
		painter->fillRect(r, QColor(255, 255, 200));
	}

	// vertical lines
	for (int i = 1; i < 7; ++i)
		painter->drawLine((i * horiAmount) / 7 + i - 1, 0,
						  (i * horiAmount) / 7 + i - 1, visibleRect.height());

	// horizontal lines
	for (int i = 1; i < weekCount; ++i)
		painter->drawLine(0, (i * vertiAmount) / weekCount + i - 1,
						  visibleRect.width(), (i * vertiAmount) / weekCount + i - 1);

	// day texts
	for (int j = 0; j < weekCount; ++j)
		for (int i = 0; i < 7; ++i) {
			QRect r((i * horiAmount) / 7 + i, (j * vertiAmount) / weekCount + j + 2, // +2 is a correction to be not stucked to the top line
					((i + 1) * horiAmount) / 7 - (i * horiAmount) / 7 - 2, // -2 is a correction to be not stucked to the right line
					((j + 1) * vertiAmount) / weekCount - (j * vertiAmount) / weekCount);

			QString text;
			if (firstDay.day() == 1)
				text = firstDay.toString(tr("d MMM"));
			else
				text = firstDay.toString(tr("d"));

			if (firstDay.month() != m_firstDate.month())
				pen.setColor(QColor(200, 200, 200));
			else
				pen.setColor(QColor(150, 150, 150));
			painter->setPen(pen);

			painter->drawText(r, Qt::AlignRight | Qt::AlignTop, text);
			firstDay = firstDay.addDays(1);
		}
}
