/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2010 by Eric MAEKER, MD (France) <eric.maeker@free.fr>        *
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
 *   Main Developper : Eric MAEKER, <eric.maeker@free.fr>                  *
 *   Contributors :                                                        *
 *       NAME <MAIL@ADRESS>                                                *
 ***************************************************************************/
#include "icdmodel.h"
#include "icddatabase.h"
#include "constants.h"

#include <translationutils/constanttranslations.h>

#include <QString>
#include <QLocale>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSqlError>

#include <QDebug>

using namespace ICD;

using namespace Trans::ConstantTranslations;

static inline ICD::IcdDatabase *icdBase() {return ICD::IcdDatabase::instance();}

namespace ICD {
namespace Internal {

    enum QueryFields {
        Query_SID = 0,
        Query_LID
    };

class IcdModelPrivate
{
public:
    IcdModelPrivate(IcdModel *parent) : m_IcdMaster(0), m_SearchMode(IcdModel::SearchByLabel), q(parent)
    {
        m_IcdMaster = new QSqlQueryModel(q);
        // Master -> valid=1, level=4, code like '%search%'
        // Libelle -> XX_OMS like 'search', valid=1
        // †  -->  &#134;
    }

    QString searchQuery()
    {
        QString req;
        QString fields;
        if (m_SearchMode == IcdModel::SearchByLabel) {
            fields = QString("`%1`.`%2`, `%3`.`%4` ")
                     .arg(icdBase()->table(Constants::Table_System))
                     .arg(icdBase()->field(Constants::Table_System, Constants::SYSTEM_SID))
                     .arg(icdBase()->table(Constants::Table_System))
                     .arg(icdBase()->field(Constants::Table_System, Constants::SYSTEM_LID));

            req = QString("SELECT %1 \nFROM %2 \n"
                          "LEFT JOIN %3 ON %2.%4=%3.%5 \n")
                    .arg(fields)
                    .arg(icdBase()->table(Constants::Table_System))
                    // join libelle
                    .arg(icdBase()->table(Constants::Table_Libelle))
                    .arg(icdBase()->field(Constants::Table_System, Constants::SYSTEM_LID))
                    .arg(icdBase()->field(Constants::Table_Libelle, Constants::LIBELLE_LID));
        } else {
            // Search by ICD Codes
//            SELECT `master`.`SID`, system.LID
//            FROM master, system
//            WHERE  (`master`.`code` like 'B5%') AND (system.SID=master.SID)
            fields = QString("`%1`.`%2`, `%3`.`%4` ")
                     .arg(icdBase()->table(Constants::Table_Master))
                     .arg(icdBase()->field(Constants::Table_Master, Constants::MASTER_SID))
                     .arg(icdBase()->table(Constants::Table_System))
                     .arg(icdBase()->field(Constants::Table_System, Constants::SYSTEM_LID));
            req = QString("SELECT %1 \n"
                          "FROM %2, %3  \n"
                          "WHERE %2.%5 = %3.%6 \n")
                    .arg(fields)
                    .arg(icdBase()->table(Constants::Table_Master))
                    .arg(icdBase()->table(Constants::Table_System))
                    // Where clause
                    .arg(icdBase()->field(Constants::Table_Master, Constants::MASTER_SID))
                    .arg(icdBase()->field(Constants::Table_System, Constants::SYSTEM_SID));
        }
        return req;
    }

    ~IcdModelPrivate () {}

public:
    QSqlQueryModel *m_IcdMaster;
    IcdModel::SearchModes m_SearchMode;
    QString m_LastFilterRequiered;

private:
    IcdModel *q;
};
}
}

IcdModel::IcdModel(QObject *parent) :
        QAbstractTableModel(parent), d(new Internal::IcdModelPrivate(this))
{
    // connect lanquage change
    d->m_IcdMaster->setQuery(d->searchQuery(), icdBase()->database());

    connect(d->m_IcdMaster,SIGNAL(layoutChanged()), this, SIGNAL(layoutChanged()));
    connect(d->m_IcdMaster,SIGNAL(layoutAboutToBeChanged()), this, SIGNAL(layoutAboutToBeChanged()));
    connect(d->m_IcdMaster,SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)), this, SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)));
    connect(d->m_IcdMaster,SIGNAL(rowsInserted(QModelIndex, int, int)), this, SIGNAL(rowsInserted(QModelIndex, int, int)));
    connect(d->m_IcdMaster,SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)), this, SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)));
    connect(d->m_IcdMaster,SIGNAL(rowsRemoved(QModelIndex, int, int)), this, SIGNAL(rowsRemoved(QModelIndex, int, int)));
    connect(d->m_IcdMaster,SIGNAL(modelAboutToBeReset()), this, SIGNAL(modelAboutToBeReset()));
    connect(d->m_IcdMaster,SIGNAL(modelReset()), this, SIGNAL(modelReset()));
}

IcdModel::~IcdModel()
{
    if (d) {
        delete d;
        d = 0;
    }
}

void IcdModel::setSearchMethod(SearchModes mode)
{
    if (mode==d->m_SearchMode)
        return;
    d->m_SearchMode = mode;
    // modify the query
    // update model ?
}

int IcdModel::rowCount(const QModelIndex &parent) const
{
    return d->m_IcdMaster->rowCount();
}

int IcdModel::columnCount(const QModelIndex &parent) const
{
    return ColumnCount;
}

void IcdModel::fetchMore(const QModelIndex &parent)
{
    d->m_IcdMaster->fetchMore(parent);
}

bool IcdModel::canFetchMore(const QModelIndex &parent) const
{
    return d->m_IcdMaster->canFetchMore(parent);
}

QVariant IcdModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role==Qt::DisplayRole || role==Qt::EditRole) {
        switch (index.column()) {
        case SID_Code: return d->m_IcdMaster->index(index.row(), Internal::Query_SID).data();
        case ICD_Code: return icdBase()->getIcdCode(d->m_IcdMaster->index(index.row(), Internal::Query_SID).data());
        case ICD_CodeWithDagetAndStar: return icdBase()->getIcdCodeWithDagStar(d->m_IcdMaster->index(index.row(), Internal::Query_SID).data());
        case Daget: return icdBase()->getHumanReadableIcdDaget(d->m_IcdMaster->index(index.row(), Internal::Query_SID).data());
        case Label: return icdBase()->getLabel(d->m_IcdMaster->index(index.row(), Internal::Query_LID).data());
        }
    }
    return QVariant();
}

bool IcdModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return false;
}


QVariant IcdModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QVariant();
}

bool IcdModel::insertRows(int row, int count, const QModelIndex &parent)
{
    return false;
}

bool IcdModel::removeRows(int row, int count, const QModelIndex &parent)
{
    return false;
}

void IcdModel::setFilter(const QString &searchLabel)
{
    d->m_LastFilterRequiered = searchLabel;
    QHash<int, QString> where;
    QString req;
    if (d->m_SearchMode==SearchByLabel) {
        where.insert(Constants::LIBELLE_FR, QString("like '%1%'").arg(searchLabel));
        req = d->searchQuery() + " WHERE " + icdBase()->getWhereClause(Constants::Table_Libelle, where);
    } else {
        where.insert(Constants::MASTER_CODE, QString("like '%1%'").arg(searchLabel));
        req = d->searchQuery() + " AND " + icdBase()->getWhereClause(Constants::Table_Master, where);
    }
    qWarning() << req;

    d->m_IcdMaster->setQuery(req, icdBase()->database());

    qWarning()<<d->m_IcdMaster->query().lastError().text();
    reset();
}
