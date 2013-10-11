/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2013 by Eric MAEKER, MD (France) <eric.maeker@gmail.com>      *
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
 *  Main Developer: Eric MAEKER, MD <eric.maeker@gmail.com>                *
 *  Contributors:                                                          *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 ***************************************************************************/
#include "druginteractor.h"

#include <utils/log.h>
#include <utils/global.h>

#include <QDomElement>
#include <QString>

#include <QDebug>

using namespace DDI;
using namespace Internal;

DrugInteractor::DrugInteractor()
{
    setData(IsValid, false);
    setData(IsDuplicated, false);
    setData(DoNotWarnDuplicated, false);
}

DrugInteractor::DrugInteractor(const QDomElement &element)
{
    setData(IsDuplicated, false);
    if (element.tagName()!="I") {
        setData(IsValid, false);
        return;
    }
    // get attribs
    setData(InitialLabel, element.attribute("id"));
    setData(IsValid, element.attribute("v").toInt());
    bool isClass = element.attribute("c").toInt();
    setData(IsClass, isClass);
    setData(DoNotWarnDuplicated, element.attribute("nwd").toInt());
    setData(Reference, element.attribute("r"));
    setData(PMIDsStringList, element.attribute("p").split(";"));
    setData(DateOfCreation, QDate::fromString(element.attribute("dc"), Qt::ISODate));
    setData(DateLastUpdate, QDate::fromString(element.attribute("lu"), Qt::ISODate));

    // get class information
    if (isClass) {
        QDomElement ci = element.firstChildElement("CI");
        while (!ci.isNull()) {
            const QString &l = ci.attribute("l");
            if (l == "fr")
                setData(ClassInformationFr, ci.attribute("t"));
            else if (l == "en")
                setData(ClassInformationEn, ci.attribute("t"));
            else if (l == "de")
                setData(ClassInformationDe, ci.attribute("t"));
//            else if (l == "es")
//                setData(ClassInformationEs, ci.attribute("t"));
            ci = ci.nextSiblingElement("CI");
        }
    }

    // get Labels
    QDomElement label = element.firstChildElement("L");
    while (!label.isNull()) {
        const QString &l = label.attribute("l");
        if (l == "fr")
            setData(FrLabel, label.attribute("n"));
        else if (l == "en")
            setData(EnLabel, label.attribute("n"));
        else if (l == "de")
            setData(DeLabel, label.attribute("n"));
        else if (l == "es")
            setData(EsLabel, label.attribute("n"));
        label = label.nextSiblingElement("L");
    }

    // get Review info
    QDomElement rev = element.firstChildElement("O");
    setData(Comment, rev.attribute("c"));
    setData(IsReviewed, rev.attribute("r"));
    setData(ReviewersStringList, rev.attribute("rv").split(";"));
    setData(DateOfReview, QDate::fromString(element.attribute("d"), Qt::ISODate));
    setData(IsAutoFound, rev.attribute("af"));

    // get children item
    QDomElement child = element.firstChildElement("M");
    while (!child.isNull()) {
        addChildId(child.attribute("n"));
        if (!child.attribute("p").isEmpty()) {
            addChildClassificationPMIDs(child.attribute("n"), child.attribute("p").split(";"));
        }
        child = child.nextSiblingElement("M");
    }

    // get ATC codes
    child = element.firstChildElement("ATC");
    while (!child.isNull()) {
        m_AtcLinks.append(child.attribute("a").split(";"));
        child = child.nextSiblingElement("ATC");
    }

}

DrugInteractor::~DrugInteractor()
{}

QVariant DrugInteractor::data(const int reference, const QString &lang) const
{
    QString l = lang;
    if (lang.isEmpty())
        l = Trans::Constants::ALL_LANGUAGE;
    if (reference == TranslatedLabel) {
        QString loc = QLocale().name().left(2).toLower();
        if (loc=="en")
            return m_TrData.value(EnLabel).value(l);
        else if (loc=="fr")
            return m_TrData.value(FrLabel).value(l);
        else if (loc=="de")
            return m_TrData.value(DeLabel).value(l);
        else if (loc=="es")
            return m_TrData.value(EsLabel).value(l);
    } else if (reference==ATCCodeStringList) {
        return m_AtcLinks;
    }
    return m_TrData.value(reference).value(l);
}

bool DrugInteractor::setData(const int reference, const QVariant &value, const QString &lang)
{
    QString l = lang;
    if (lang.isEmpty())
        l = Trans::Constants::ALL_LANGUAGE;
    if (reference==ATCCodeStringList) {
        m_AtcLinks = value.toStringList();
        return true;
    }
    QHash<QString, QVariant> &data = m_TrData[reference];
    data.insert(l, value);
    return true;
}

void DrugInteractor::addAtcLink(const QString &atcCode)
{
    if (m_AtcLinks.contains(atcCode))
        return;
    m_AtcLinks.append(atcCode);
    m_AtcLinks.sort();
}

void DrugInteractor::addChildClassificationPMIDs(const QString &childId, const QStringList &pmids)
{
    foreach(const QString &pmid, pmids)
        m_ChildClassifPMIDs.insertMulti(childId, pmid);
}

QStringList DrugInteractor::childClassificationPMIDs(const QString &childId) const
{
    return m_ChildClassifPMIDs.values(childId);
}


QStringList DrugInteractor::allNeededPMIDs() const
{
    return m_ChildClassifPMIDs.values();
}

QString DrugInteractor::toXml() const
{
    QString xml = QString("\n  <I id=\"%1\" v=\"%2\" c=\"%3\" r=\"%4\" p=\"%5\" dc=\"%6\" lu=\"%7\" nwd=\"%8\">")
            .arg(data(InitialLabel).toString())
            .arg(data(IsValid).toBool())
            .arg(data(IsClass).toBool())
            .arg(data(Reference).toString())
            .arg(data(PMIDsStringList).toStringList().join(";"))
            .arg(data(DateOfCreation).toDate().toString(Qt::ISODate))
            .arg(data(DateLastUpdate).toDate().toString(Qt::ISODate))
            .arg(data(DoNotWarnDuplicated).toInt())
            ;

    // add class information
    if (isClass()) {
        if (!data(ClassInformationFr).isNull()) {
            xml += QString("\n    <CI l=\"fr\" t=\"%1\"/>").arg(data(ClassInformationFr).toString());
        }
        if (!data(ClassInformationEn).isNull()) {
            xml += QString("\n    <CI l=\"en\" t=\"%1\"/>").arg(data(ClassInformationEn).toString());
        }
        if (!data(ClassInformationDe).isNull()) {
            xml += QString("\n    <CI l=\"de\" t=\"%1\"/>").arg(data(ClassInformationDe).toString());
        }
    }

    // Add labels
    QHash<int, QString> langsForLabels;
    langsForLabels.insert(FrLabel, "fr");
    langsForLabels.insert(EnLabel, "en");
    langsForLabels.insert(DeLabel, "de");
    langsForLabels.insert(EsLabel, "es");
    foreach(const QString &lang, langsForLabels) {
        xml += QString("\n    <L l=\"%1\" n=\"%2\"/>")
                .arg(lang).arg(data(langsForLabels.key(lang)).toString())
                ;
    }

    // Add ATC Codes
    const QStringList &atc = data(ATCCodeStringList).toStringList();
    if (atc.count() > 0) {
        if (!atc.at(0).isEmpty()) {
                // TODO: add to each linked ATC codes: comment, reviewState, reviewers */
//                xml += QString("\n    <ATC a=\"%1\" c=\"%2\" rv=\"\" r=\"\">")
//                        .arg(code).arg("");
            xml += QString("\n    <ATC a=\"%1\"/>")
                    .arg(atc.join(";"));
        }
    }

    // Add children
    QStringList children = childrenIds();
    children.removeDuplicates();
    foreach(const QString &c, children) {
        QStringList pmids = m_ChildClassifPMIDs.values(c);
        pmids.removeAll("");
        pmids.removeDuplicates();
        if (pmids.count()) {
            xml += QString("\n    <M n=\"%1\" p=\"%2\"/>").arg(c).arg(pmids.join(";"));
        } else {
            xml += QString("\n    <M n=\"%1\"/>").arg(c);
        }
    }

    // Add Review info
    xml += QString("\n    <O c=\"%1\" r=\"%2\" rv=\"%3\" d=\"%4\" af=\"%5\"/>")
            .arg(data(Comment).toString())
            .arg(data(IsReviewed).toInt())
            .arg(data(ReviewersStringList).toStringList().join(";"))
            .arg(data(DateOfReview).toStringList().join(";"))
            .arg(data(IsAutoFound).toInt())
            ;

    xml += "\n  </I>";
    return xml;
}

bool DrugInteractor::operator==(const DrugInteractor &other) const
{
    return (other.data(Id).toString()==m_TrData.value(Id).value(Trans::Constants::ALL_LANGUAGE).toString());
}

bool DrugInteractor::lowerThan(const DrugInteractor &d1, const DrugInteractor &d2)
{
    bool d1IsClass, d2IsClass;
    d1IsClass = d1.isClass();
    d2IsClass = d2.isClass();
    if (d1IsClass && !d2IsClass)
        return true;
    if (d2IsClass && !d1IsClass)
        return false;
    return d1.data(DrugInteractor::TranslatedLabel).toString() < d2.data(DrugInteractor::TranslatedLabel).toString();
}