/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2011 by Eric MAEKER, MD (France) <eric.maeker@free.fr>        *
 *  All rights reserved.                                                   *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program (COPYING.FREEMEDFORMS file).                   *
 *  If not, see <http://www.gnu.org/licenses/>.                            *
 ***************************************************************************/
/***************************************************************************
 *   Main Developpers :                                                    *
 *       Guillaume Denry <guillaume.denry@gmail.com>                       *
 *       Eric MAEKER, MD <eric.maeker@gmail.com>                           *
 *   Contributors :                                                        *
 *       NAME <MAIL@ADRESS>                                                *
 ***************************************************************************/
#include "calendar_navbar.h"
#include "calendar_theme.h"

#include <translationutils/constanttranslations.h>

#include <QHBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <QLabel>
#include <QDate>
#include <QMenu>
#include <QComboBox>

using namespace Calendar;
using namespace Trans::ConstantTranslations;

static inline Calendar::CalendarTheme *theme() {return Calendar::CalendarTheme::instance();}

/**
  \class Calendar::CalendarNavbar
  Assume the navigation in the calendar view.
*/

/**
  \fn void Calendar::CalendarNavbar::granularityChanged(int);
  Signal sent when user select a new day granularity. Time is sent in minutes.
*/

CalendarNavbar::CalendarNavbar(QWidget *parent) :
        QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setAutoFillBackground(true);
    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, QColor(180, 180, 255));
    this->setPalette(palette);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(createNavigationButtons());
    layout->addWidget(m_dateLabel = new QLabel);
    QFont font = m_dateLabel->font();
    font.setBold(true);
    m_dateLabel->setFont(font);
    layout->addStretch();
    layout->addWidget(createNavigationModeButton());

    // signal/slot connections
    connect(m_todayButton, SIGNAL(clicked()), this, SLOT(todayPage()));
    connect(m_previousPageButton, SIGNAL(clicked()), this, SLOT(previousPage()));
    connect(m_nextPageButton, SIGNAL(clicked()), this, SLOT(nextPage()));
    connect(m_viewModeNav, SIGNAL(triggered(QAction*)), this, SLOT(changeViewMode(QAction*)));
}

QWidget *CalendarNavbar::createNavigationButtons() {
    QWidget *w = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(w);
    layout->setSpacing(0);
    layout->setMargin(0);

    // Add prev/next
    m_previousPageButton = new QToolButton(this);
    QString icon = theme()->iconFileName(CalendarTheme::NavigationPrevious);
    if (icon.isEmpty())
        m_previousPageButton->setText("<<");
    else
        m_previousPageButton->setIcon(QIcon(icon));

    m_nextPageButton = new QToolButton(this);
    icon = theme()->iconFileName(CalendarTheme::NavigationNext);
    if (icon.isEmpty())
        m_nextPageButton->setText(">>");
    else
        m_nextPageButton->setIcon(QIcon(icon));

    layout->addWidget(m_previousPageButton);
    layout->addWidget(m_nextPageButton);

    // Add day nav
    layout->addWidget(m_todayButton = createTodayButton());
    w->setLayout(layout);
    return w;
}

QToolButton *CalendarNavbar::createNavigationModeButton() {
    QString icon;
    m_viewModeNav = new QToolButton(this);
    icon = theme()->iconFileName(CalendarTheme::NavigationViewMode);
    if (!icon.isEmpty())
        m_viewModeNav->setIcon(QIcon(icon));
    m_viewModeNav->setPopupMode(QToolButton::InstantPopup);
    m_dayView = new QAction(tkTr(Trans::Constants::DAY), this);
    icon = theme()->iconFileName(CalendarTheme::NavigationDayViewMode);
    if (!icon.isEmpty())
        m_dayView->setIcon(QIcon(icon));
    m_weekView = new QAction(tkTr(Trans::Constants::WEEK), this);
    icon = theme()->iconFileName(CalendarTheme::NavigationDayViewMode);
    if (!icon.isEmpty())
        m_weekView->setIcon(QIcon(icon));
    m_monthView = new QAction(tkTr(Trans::Constants::MONTH), this);
    icon = theme()->iconFileName(CalendarTheme::NavigationDayViewMode);
    if (!icon.isEmpty())
        m_monthView->setIcon(QIcon(icon));
    m_viewModeNav->addAction(m_dayView);
    m_viewModeNav->addAction(m_weekView);
    m_viewModeNav->addAction(m_monthView);
    return m_viewModeNav;
}

QToolButton *CalendarNavbar::createTodayButton() {
    QToolButton *button = new QToolButton(this);
    QString icon = theme()->iconFileName(CalendarTheme::NavigationBookmarks);
    if (icon.isEmpty())
        button->setText("^");
    else
        button->setIcon(QIcon(icon));

    QMenu *menu = new QMenu(this);
    menu->addAction(tr("Today"), this, SLOT(todayPage()));
    menu->addAction(tr("Yesterday"), this, SLOT(yesterdayPage()));
    menu->addAction(tr("Tomorrow"), this, SLOT(tomorrowPage()));
    QMenu *viewMenu = menu->addMenu(tr("View range"));
    for(int i = 1; i < 19; ++i) {
        QAction *action = viewMenu->addAction(QString("%1 %2").arg(i*5).arg(tkTr(Trans::Constants::MINUTES)));
        action->setData(i);
    }
    connect(viewMenu, SIGNAL(triggered(QAction*)), this, SLOT(changeGranularity(QAction*)));

    button->setMenu(menu);
    button->setPopupMode(QToolButton::InstantPopup);
//    button->setDefaultAction(action);
    return button;
}

void CalendarNavbar::setViewType(ViewType viewType) {
	if (viewType == m_viewType)
		return;

	m_viewType = viewType;

	refreshInfos();

	emit viewTypeChanged();
}

void CalendarNavbar::setDate(const QDate &date) {
	QDate firstDate = getFirstDateByRandomDate(m_viewType, date);
	if (firstDate == m_firstDate)
		return;

	m_firstDate = firstDate;
	refreshInfos();
	emit firstDateChanged();
}

void CalendarNavbar::setDayGranularity(const int durationInMinutes) {
    Q_UNUSED(durationInMinutes);
//    // Find the index (every 5 minutes)
//    int index = -1;
//    if (durationInMinutes%5)
//        index = (durationInMinutes/5);
//    else
//        index = (durationInMinutes/5 - 1);
//    m_granularity->setCurrentIndex(index);
}

void CalendarNavbar::refreshInfos() {
	// TODO (refresh label, etc...)
	switch (m_viewType){
	case View_Day:
		m_dateLabel->setText(m_firstDate.toString());
		break;
	case View_Week:
		m_dateLabel->setText(getDateIntervalString());
		break;
	case View_Month:
		m_dateLabel->setText(m_firstDate.toString("MMMM yyyy"));
		break;
	default:; // should never happends
	}
}

void CalendarNavbar::todayPage() {
	setDate(QDate::currentDate());
}

void CalendarNavbar::yesterdayPage() {
	setDate(QDate::currentDate().addDays(-1));
}

void CalendarNavbar::tomorrowPage() {
	setDate(QDate::currentDate().addDays(1));
}

void CalendarNavbar::previousPage() {
	switch (m_viewType) {
	case View_Day:
		setDate(m_firstDate.addDays(-1));
		break;
	case View_Week:
		setDate(m_firstDate.addDays(-7));
		break;
	case View_Month:
		setDate(m_firstDate.addMonths(-1));
		break;
	default: // should never happend
		break;
	}
}

void CalendarNavbar::nextPage() {
	switch (m_viewType) {
	case View_Day:
		setDate(m_firstDate.addDays(1));
		break;
	case View_Week:
		setDate(m_firstDate.addDays(7));
		break;
	case View_Month:
		setDate(m_firstDate.addMonths(1));
		break;
	default: // should never happend
		break;
	}
}

void CalendarNavbar::changeViewMode(QAction *action) {
    if (action==m_dayView)
        dayMode();
    else if (action==m_weekView)
        weekMode();
    else if (action==m_monthView)
        monthMode();
}

void CalendarNavbar::dayMode() {
	setViewType(Calendar::View_Day);
}

void CalendarNavbar::weekMode() {
	setViewType(Calendar::View_Week);
}

void CalendarNavbar::monthMode() {
	setViewType(Calendar::View_Month);
}

void CalendarNavbar::changeGranularity(QAction *action) {
    Q_EMIT granularityChanged(action->data().toInt()*5);
}

QString CalendarNavbar::getDateIntervalString() {
	QDate lastDate;
	switch (m_viewType) {
	case View_Day:
		return m_firstDate.toString();
	case View_Week:
		lastDate = m_firstDate.addDays(6);
		if (m_firstDate.month() != lastDate.month() && m_firstDate.year() != lastDate.year())
			return QString("%1 - %2").arg(m_firstDate.toString(tr("d MMM yyyy"))).arg(lastDate.toString(tr("d MMM yyyy")));
		else if (m_firstDate.month() != lastDate.month())
			return QString("%1 - %2").arg(m_firstDate.toString(tr("d MMM"))).arg(lastDate.toString(tr("d MMM yyyy")));
		else
			return QString("%1 - %2").arg(m_firstDate.toString(tr("d"))).arg(lastDate.toString(tr("d MMM yyyy")));
	case View_Month:
//			return randomDate.addDays(- randomDate.day() + 1);
	default: // should never happend
		return "";
	}
}
