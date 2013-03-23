/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2012 by Eric MAEKER, MD (France) <eric.maeker@gmail.com>      *
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
 *   Main developers: Eric MAEKER, <eric.maeker@gmail.com>                 *
 *   Contributors:                                                         *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 ***************************************************************************/
/*!
 * \class Tools::Internal::FspPrinter
 * \brief Prints a french FSP.
 */

#include "fspprinter.h"
#include "fspconstants.h"
#include "fsp.h"

#include <coreplugin/icore.h>
#include <coreplugin/isettings.h>

#include <utils/printaxishelper.h>
#include <translationutils/constants.h>

#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>
#include <QSystemLocale>
#include <QPixmap>

#include <QDebug>
#include <QFile>

using namespace Tools;
using namespace Internal;
using namespace Trans::ConstantTranslations;

static inline Core::ISettings *settings() { return Core::ICore::instance()->settings(); }

namespace Tools {
namespace Internal {
class FspPrinterPrivate
{
public:
    FspPrinterPrivate(FspPrinter *parent) :
        _drawRects(false),
        q(parent)
    {
    }
    
    ~FspPrinterPrivate()
    {
    }

    QRectF totalAmountRect(FspPrinter::Cerfa cerfa)
    {
        if (cerfa == FspPrinter::S12541_01)
            return QRectF(QPointF(109.2, 247.2), QSizeF(30.7, 5.3));
        else
            return QRectF(QPointF(109.2, 252.2), QSizeF(30.7, 5.3));
    }

    QRectF rectInMilliters(FspPrinter::Cerfa cerfa, int fspIndex)
    {
            switch (fspIndex) {
            // Bill
            case Fsp::Bill_Number:
                return QRectF(QPointF(157.5, 6.1), QSizeF(45.7, 5.3));
            case Fsp::Bill_Date:
                if (cerfa == FspPrinter::S12541_01)
                    return QRectF(QPointF(172.5, 14), QSizeF(30, 4));
                else
                    return QRectF(QPointF(162.6, 12.7), QSizeF(40.6, 4.3));
            // Patient
            case Fsp::Patient_FullName:
                return QRectF(QPointF(55.9,  26.6), QSizeF(140, 4));
            case Fsp::Patient_FullAddress:
                return QRectF(QPointF(55.9,  70.1), QSizeF(110, 4));
            case Fsp::Patient_DateOfBirth:
                return QRectF(QPointF(55.9,  43.), QSizeF(40,  4));
            case Fsp::Patient_Personal_NSS:
                return QRectF(QPointF(55.9,  34.4), QSizeF(66,  5));
            case Fsp::Patient_Personal_NSSKey:
                if (cerfa == FspPrinter::S12541_01)
                    return QRectF(QPointF(124.2, 34.4), QSizeF(10,  5));
                else
                    return QRectF(QPointF(124.5, 34.4), QSizeF(10,  5));
            case Fsp::Patient_Assure_FullName:
                return QRectF(QPointF(55.9,  52.5), QSizeF(140, 4));
            case Fsp::Patient_Assure_NSS:
                return QRectF(QPointF(55.9,  60.6), QSizeF(66,  5));
            case Fsp::Patient_Assure_NSSKey:
                if (cerfa == FspPrinter::S12541_01)
                    return QRectF(QPointF(124.2, 60.6), QSizeF(10,  5));
                else
                    return QRectF(QPointF(124.5, 60.6), QSizeF(10,  5));
            case Fsp::Patient_Assurance_Number:
                return QRectF(QPointF(157.5, 34.4), QSizeF(45.7, 5.4));
            // Conditions
            case Fsp::Condition_Maladie:
                return QRectF(QPointF(7.3, 128.), QSizeF(5.3, 3.8));
            case Fsp::Condition_Maladie_ETM :
                if (cerfa == FspPrinter::S12541_01) {
                    return QRectF(QPointF(116.7, 128.), QSizeF(5.3, 3.8)); // TRUE value
                }
                // TODO: manage value == false: return QRectF(QPointF(95.5, 128.), QSizeF(5.3, 3.8));
            case Fsp::Condition_Maladie_ETM_Ald :
                return QRectF(QPointF(55., 134.6) ,QSizeF(5.3, 3.8));
            case Fsp::Condition_Maladie_ETM_Autre :
                if (cerfa == FspPrinter::S12541_01)
                    return QRectF(QPointF(197.4, 134.6), QSizeF(5.3, 3.8));
                else
                    return QRectF(QPointF(124.5, 134.6), QSizeF(5.3, 3.8));
            case Fsp::Condition_Maladie_ETM_L115:
                if (cerfa == FspPrinter::S12541_01)
                    return QRectF(QPointF(166.9, 134.6), QSizeF(5.3, 3.8));
                else
                    return QRectF(QPointF(135.4, 128.), QSizeF(5.3, 3.8));
            case Fsp::Condition_Maladie_ETM_Prevention:
                if (cerfa == FspPrinter::S12541_01)
                    return QRectF(QPointF(105.7, 134.6), QSizeF(5.3, 3.8));
                else
                    return QRectF(QPointF(100.1, 134.6), QSizeF(5.3, 3.8));
            case Fsp::Condition_Maladie_ETM_AccidentParTiers_Oui :
                return QRectF(QPointF(64.3, 141.6), QSizeF(5.3, 3.8)); // NON
                // TODO: manage value == false: return QRectF(QPointF(87.4, 141.6), QSizeF(5.3, 3.8)); // OUI
            case Fsp::Condition_Maladie_ETM_AccidentParTiers_Date :
                return QRectF(QPointF(114.5, 141.2), QSizeF(40.6, 4.1));
            case Fsp::Condition_Maternite :
                return QRectF(QPointF(7.5, 148.7), QSizeF(5.3, 3.8));
            case Fsp::Condition_Maternite_Date :
                return QRectF(QPointF(162.5, 148.7), QSizeF(40.6, 4.1));
            case Fsp::Condition_ATMP :
                return QRectF(QPointF(7.5, 155.9), QSizeF(5.3, 3.8));
            case Fsp::Condition_ATMP_Number :
                return QRectF(QPointF(68.4, 155.9), QSizeF(45.7, 4.3));
            case Fsp::Condition_ATMP_Date :
                return QRectF(QPointF(162.5, 155.9), QSizeF(40.6, 4.1));
            case Fsp::Condition_NouveauMedTraitant:
                return QRectF(QPointF(91.1, 163.2), QSizeF(5.3, 3.8));
            case Fsp::Condition_MedecinEnvoyeur :
                if (cerfa == FspPrinter::S12541_01)
                    return QRectF(QPointF(62.1, 175.1), QSizeF(140., 5.3));
                else
                    return QRectF(QPointF(50.5, 173.2), QSizeF(140., 5.3));
            case Fsp::Condition_AccesSpecifique :
                return QRectF(QPointF(37.1, 183.2), QSizeF(5.3, 3.8));
            case Fsp::Condition_Urgence :
                return QRectF(QPointF(60.8, 183.2), QSizeF(5.3, 3.8));
            case Fsp::Condition_HorsResidence :
                return QRectF(QPointF(106.3, 183.2), QSizeF(5.3, 3.8));
            case Fsp::Condition_Remplace :
                return QRectF(QPointF(154.1, 183.2), QSizeF(5.3, 3.8));
            case Fsp::Condition_HorsCoordination :
                return QRectF(QPointF(197.4, 183.2), QSizeF(5.3, 3.8));
            case Fsp::Condition_AccordPrealableDate:
                if (cerfa == FspPrinter::S12541_02)
                    return QRectF(QPointF(144.8, 193.5), QSizeF(40.6, 4.1));
            case Fsp::Unpaid_PartObligatoire:
                if (cerfa == FspPrinter::S12541_01)
                    return QRectF(QPointF(86.2, 256.3), QSizeF(5.3, 3.8));
                else
                    return QRectF(QPointF(86.4, 261.6), QSizeF(5.3, 3.8));
            case Fsp::Unpaid_PartComplementaire:
                if (cerfa == FspPrinter::S12541_01)
                    return QRectF(QPointF(190.5, 256.3), QSizeF(5.3, 3.8));
                else
                    return QRectF(QPointF(190.5, 261.6), QSizeF(5.3, 3.8));
            }
        return QRectF();
    }

    QRectF amountRectInMilliters(FspPrinter::Cerfa cerfa, int line, int fspIndex)
    {
            int y;
            if (cerfa == FspPrinter::S12541_01)
                y = 209.4 + 8.6*line;
            else
                y = 214.1 + 8.6*line;
            switch (fspIndex) {
            case Fsp::Amount_Date:
                return QRectF(QPointF(6.8, y), QSizeF(30.7, 5.1));
            case Fsp::Amount_ActCode:
                return QRectF(QPointF(38.3, y), QSizeF(36.6, 5.1));
            case Fsp::Amount_Activity:
                return QRectF(QPointF(74.8, y), QSizeF(5.8, 5.1));
            case Fsp::Amount_CV:
                return QRectF(QPointF(80.6, y), QSizeF(10.7, 5.1));
            case Fsp::Amount_OtherAct1:
                return QRectF(QPointF(91.4, y), QSizeF(16.5, 5.1));
            case Fsp::Amount_OtherAct2:
                return QRectF(QPointF(108.2, y), QSizeF(16.5, 5.1));
            case Fsp::Amount_Amount:
                return QRectF(QPointF(124.6, y), QSizeF(30.2, 5.1));
            case Fsp::Amount_Depassement:
                return QRectF(QPointF(155.1, y), QSizeF(7.1, 5.1));
            case Fsp::Amount_Deplacement_IKMD:
                return QRectF(QPointF(162.9, y), QSizeF(9.7, 5.1));
            case Fsp::Amount_Deplacement_Nb:
                return QRectF(QPointF(173.0, y),QSizeF(9.7, 5.1));
            case Fsp::Amount_Deplacement_IKValue:
                return QRectF(QPointF(183.2, y), QSizeF(20.1, 5.1));
            return QRectF();
        }
    }

    QString printingValue(const Fsp &fsp, FspPrinter::Cerfa cerfa, int fspIndex)
    {
        if (!fsp.data(fspIndex).isValid() || fsp.data(fspIndex).isNull()) {
            qWarning() << "fspIndex" << fspIndex << "isNull or invalid"
                       << "data" << fsp.data(fspIndex);
            return QString::null;
        }

        switch (fspIndex) {
        // Bill
        case Fsp::Bill_Number:
            return fsp.data(Fsp::Bill_Number).toString().left(9);
        case Fsp::Bill_Date:
            if (cerfa == FspPrinter::S12541_01)
                return fsp.data(Fsp::Bill_Date).toDate().toString("ddMMyy");
            else
                return fsp.data(Fsp::Bill_Date).toDate().toString("ddMMyyyy");
            // Patient
        case Fsp::Patient_FullName:
        case Fsp::Patient_FullAddress:
        case Fsp::Patient_Personal_NSS:
        case Fsp::Patient_Personal_NSSKey:
        case Fsp::Patient_Assure_FullName:
        case Fsp::Patient_Assure_NSS:
        case Fsp::Patient_Assure_NSSKey:
        case Fsp::Condition_MedecinEnvoyeur:
            return fsp.data(fspIndex).toString();
        case Fsp::Patient_Assurance_Number:
        case Fsp::Condition_ATMP_Number:
            return fsp.data(fspIndex).toString().left(9).rightJustified(9, '*');
        case Fsp::Patient_DateOfBirth:
        case Fsp::Condition_Maternite_Date:
        case Fsp::Condition_Maladie_ETM_AccidentParTiers_Date:
        case Fsp::Condition_AccordPrealableDate:
        case Fsp::Condition_ATMP_Date:
            return fsp.data(Fsp::Patient_DateOfBirth).toDate().toString("ddMMyyyy");
            // Conditions
        case Fsp::Condition_Maladie:
        case Fsp::Condition_Maladie_ETM:
        case Fsp::Condition_Maladie_ETM_Ald:
        case Fsp::Condition_Maladie_ETM_Autre:
        case Fsp::Condition_Maladie_ETM_L115:
        case Fsp::Condition_Maladie_ETM_Prevention:
        case Fsp::Condition_Maladie_ETM_AccidentParTiers_Oui:
        case Fsp::Condition_Maternite:
        case Fsp::Condition_ATMP:
        case Fsp::Condition_NouveauMedTraitant:
        case Fsp::Condition_AccesSpecifique:
        case Fsp::Condition_Urgence:
        case Fsp::Condition_HorsResidence:
        case Fsp::Condition_Remplace:
        case Fsp::Condition_HorsCoordination:
        case Fsp::Unpaid_PartObligatoire:
        case Fsp::Unpaid_PartComplementaire:
            if (fsp.data(fspIndex).toBool())
                return "X";
        }
        return QString::null;
    }

    QString amountPrintingValue(const Fsp &fsp, FspPrinter::Cerfa cerfa, int line, int fspIndex)
    {
        switch (fspIndex) {
        case Fsp::Amount_Date:
            return fsp.amountLineData(line, fspIndex).toDate().toString("ddMMyyyy");
        case Fsp::Amount_ActCode:
            return fsp.amountLineData(line, fspIndex).toString().left(7);
        case Fsp::Amount_Activity:
            return fsp.amountLineData(line, fspIndex).toString().left(1);
        case Fsp::Amount_CV:
        case Fsp::Amount_OtherAct1:
        case Fsp::Amount_OtherAct2:
            return fsp.amountLineData(line, fspIndex).toString().left(5);
        case Fsp::Amount_Depassement:
        case Fsp::Amount_Deplacement_IKMD:
            return fsp.amountLineData(line, fspIndex).toString().left(2);
        case Fsp::Amount_Deplacement_Nb:
            return fsp.amountLineData(line, fspIndex).toString();
        case Fsp::Amount_Amount:
            return QString::number(fsp.amountLineData(line, fspIndex).toDouble(), 'f', 2).remove(".").rightJustified(6, '*');
        case Fsp::Amount_Deplacement_IKValue:
            return QString::number(fsp.amountLineData(line, fspIndex).toDouble(), 'f', 2).remove(".").rightJustified(4, '*');
        }
        return QString::null;
    }

    bool charSplitting(int fspIndex)
    {
        switch (fspIndex) {
        // Bill
        case Fsp::Bill_Number:
        case Fsp::Bill_Date:
        case Fsp::Patient_Personal_NSS:
        case Fsp::Patient_Personal_NSSKey:
        case Fsp::Patient_Assure_NSS:
        case Fsp::Patient_Assure_NSSKey:
        case Fsp::Patient_Assurance_Number:
        case Fsp::Condition_ATMP_Number:
        case Fsp::Patient_DateOfBirth:
        case Fsp::Condition_Maternite_Date:
        case Fsp::Condition_Maladie_ETM_AccidentParTiers_Date:
        case Fsp::Condition_Maladie:
        case Fsp::Condition_Maladie_ETM:
        case Fsp::Condition_Maladie_ETM_Ald:
        case Fsp::Condition_Maladie_ETM_Autre:
        case Fsp::Condition_Maladie_ETM_L115:
        case Fsp::Condition_Maladie_ETM_Prevention:
        case Fsp::Condition_Maladie_ETM_AccidentParTiers_Oui:
        case Fsp::Condition_Maternite:
        case Fsp::Condition_ATMP:
        case Fsp::Condition_ATMP_Date:
        case Fsp::Condition_NouveauMedTraitant:
        case Fsp::Condition_AccesSpecifique:
        case Fsp::Condition_Urgence:
        case Fsp::Condition_HorsResidence:
        case Fsp::Condition_Remplace:
        case Fsp::Condition_HorsCoordination:
        case Fsp::Condition_AccordPrealableDate:
        case Fsp::Unpaid_PartObligatoire:
        case Fsp::Unpaid_PartComplementaire:
        case Fsp::Amount_Date:
        case Fsp::Amount_ActCode:
        case Fsp::Amount_Activity:
        case Fsp::Amount_Amount:
        case Fsp::Amount_Depassement:
        case Fsp::Amount_Deplacement_Nb:
        case Fsp::Amount_Deplacement_IKValue:
            return true;
        case Fsp::Patient_FullName:
        case Fsp::Patient_FullAddress:
        case Fsp::Patient_Assure_FullName:
        case Fsp::Condition_MedecinEnvoyeur:
        case Fsp::Amount_CV:
        case Fsp::Amount_OtherAct1:
        case Fsp::Amount_OtherAct2:
        case Fsp::Amount_Deplacement_IKMD:
            return false;
        }
    }

    Qt::AlignmentFlag amountAlignement(int fspIndex)
    {
        // For splitting char this value is ignored
        switch (fspIndex) {
        case Fsp::Amount_ActCode:
        case Fsp::Amount_Activity:
        case Fsp::Amount_Depassement:
        case Fsp::Amount_CV:
        case Fsp::Amount_OtherAct1:
        case Fsp::Amount_OtherAct2:
        case Fsp::Amount_Deplacement_IKMD:
            return Qt::AlignCenter;
        }
        return Qt::AlignLeft;
    }

public:
    Utils::PrintAxisHelper _axisHelper;
    bool _drawRects;

private:
    FspPrinter *q;
};
} // namespace Internal
} // namespace Tools


/*! Constructor of the Tools::Internal::FspPrinter class */
FspPrinter::FspPrinter() :
    d(new FspPrinterPrivate(this))
{
}

/*! Destructor of the Tools::Internal::FspPrinter class */
FspPrinter::~FspPrinter()
{
    if (d)
        delete d;
    d = 0;
}

void FspPrinter::setDrawRects(bool drawRects)
{
    d->_drawRects = drawRects;
}

bool FspPrinter::print(const Fsp &fsp, Cerfa cerfa, bool printCerfaAsBackground)
{
    QPrintDialog dlg;
    if (dlg.exec()==QDialog::Rejected)
        return false;
    QPrinter *printer = dlg.printer();
    printer->setFullPage(true);
    printer->setPaperSize(QPrinter::A4);
    printer->setResolution(150);
    d->_axisHelper.setPageSize(printer->paperRect(), printer->paperSize(QPrinter::Millimeter));

    QPainter painter;
    if (!painter.begin(printer)) { // failed to open file
        qWarning("failed to open file, is it writable?");
        return false;
    }

    QFont font;
    font.setFamily("Arial Black");
    font.setPointSize(9);
    painter.setFont(font);

    QPixmap background;
    if (printCerfaAsBackground) {
        if (cerfa == S12541_01) {
            if (!background.load(settings()->path(Core::ISettings::ThemeRootPath) + "/pixmap/others/S3110.png", "PNG"))
                qWarning() << "ERROR: unable to load background pixmap";
        } else {
            if (!background.load(settings()->path(Core::ISettings::ThemeRootPath) + "/pixmap/others/S3110_02.png", "PNG"))
                qWarning() << "ERROR: unable to load background pixmap";
        }
        painter.drawPixmap(QRect(QPoint(0,0), printer->pageRect().size()), background);
    }

    // Include the printer correction
    painter.translate(d->_axisHelper.pointToPixels(settings()->value(Constants::S_HORIZ_CORRECTION_MM).toDouble(),
                                                   settings()->value(Constants::S_VERTIC_CORRECTION_MM).toDouble()));

    Utils::PrintString s;
    s.autoFontResize = true;
    s.minFontPixelSize = 10;
    s.drawBoundingRect = d->_drawRects;
    for(int i = 0; i < Fsp::MaxData; ++i) {
        const QRectF &rect = d->rectInMilliters(cerfa, i);
        if (!rect.isValid() || rect.isNull())
            continue;
        s.topMillimeters = rect.topLeft();
        s.contentSizeMillimeters = rect.size();
        s.splitChars = d->charSplitting(i);
        s.content = d->printingValue(fsp, cerfa, i);
        d->_axisHelper.printString(&painter, s);
    }

    // Acts and amount
    double totalAmount = 0.;
    for(int i=0; i < 4; ++i) {
        if (fsp.amountLineData(i, Fsp::Amount_Amount).isNull())
            break;
        for(int j = Fsp::Amount_Date; j < Fsp::Amount_MaxData; ++j) {
            const QRectF &rect = d->amountRectInMilliters(cerfa, i, j);
            if (!rect.isValid() || rect.isNull())
                continue;
            s.topMillimeters = rect.topLeft();
            s.contentSizeMillimeters = rect.size();
            s.splitChars = d->charSplitting(j);
            s.content = d->amountPrintingValue(fsp, cerfa, i, j);
            s.alignment = d->amountAlignement(j);
            d->_axisHelper.printString(&painter, s);
        }

        QRectF amountRect = d->amountRectInMilliters(cerfa, i, Fsp::Amount_Amount);
        s.topMillimeters = amountRect.topLeft() + QPointF(19.6, 0.3);
        s.contentSizeMillimeters =  QSizeF(1.8, 5.1);
        s.content = ",";
        d->_axisHelper.printString(&painter, s);

        if (!fsp.amountLineData(i, Fsp::Amount_Amount).isNull()) {
            totalAmount += fsp.amountLineData(i, Fsp::Amount_Amount).toDouble(); // add IK
        }
    }

    if (totalAmount > 0.) {
        const QRectF amountRect = d->totalAmountRect(cerfa);
        s.topMillimeters = amountRect.topLeft();
        s.contentSizeMillimeters =  amountRect.size();
        s.content = QString::number(totalAmount, 'f', 2).remove(".").rightJustified(6, '*');
        s.splitChars = true;
        d->_axisHelper.printString(&painter, s);

        s.topMillimeters = amountRect.topLeft() + QPointF(19.6, 0.3);
        s.contentSizeMillimeters =  QSizeF(1.8, 5.1);
        s.content = ",";
        d->_axisHelper.printString(&painter, s);
    }

    painter.end();
    return true;
}

///** Print the preview in a QPainter with the CERFA as background */
//bool FspPrinter::preview(const Fsp &fsp, Cerfa cerfa)
//{

//}
