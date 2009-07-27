/***************************************************************************
 *   FreeMedicalForms                                                      *
 *   Copyright (C) 2008-2009 by Eric MAEKER                                *
 *   eric.maeker@free.fr                                                   *
 *   All rights reserved.                                                  *
 *                                                                         *
 *   This program is a free and open source software.                      *
 *   It is released under the terms of the new BSD License.                *
 *                                                                         *
 *   Redistribution and use in source and binary forms, with or without    *
 *   modification, are permitted provided that the following conditions    *
 *   are met:                                                              *
 *   - Redistributions of source code must retain the above copyright      *
 *   notice, this list of conditions and the following disclaimer.         *
 *   - Redistributions in binary form must reproduce the above copyright   *
 *   notice, this list of conditions and the following disclaimer in the   *
 *   documentation and/or other materials provided with the distribution.  *
 *   - Neither the name of the FreeMedForms' organization nor the names of *
 *   its contributors may be used to endorse or promote products derived   *
 *   from this software without specific prior written permission.         *
 *                                                                         *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS   *
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT     *
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS     *
 *   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE        *
 *   COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,  *
 *   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,  *
 *   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;      *
 *   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER      *
 *   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT    *
 *   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN     *
 *   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE       *
 *   POSSIBILITY OF SUCH DAMAGE.                                           *
 ***************************************************************************/
/***************************************************************************
 *   Main Developper : Eric MAEKER, <eric.maeker@free.fr>                  *
 *   Contributors :                                                        *
 *       NAME <MAIL@ADRESS>                                                *
 ***************************************************************************/
#ifndef TKTHEME_H
#define TKTHEME_H

#include <tkExporter.h>
#include <tkGlobal.h>
#include <tkConstants.h>
class tkThemePrivate;
class tkSettings;

#include <QObject>
#include <QIcon>
#include <QPointer>

/**
 * \file tkTheme.h
 * \author Eric MAEKER <eric.maeker@free.fr>
 * \version 0.0.7
 * \date 16 July 2009
*/

class Q_TK_EXPORT tkTheme : public QObject
{
    Q_OBJECT
    friend class tkSettings;
    friend void tkGlobal::initLib();
public:
    enum IconSize {
        SmallIcon = 0, // 16x16 is the default
        MediumIcon,    // 32x32
        BigIcon        // 64x64
    };

    static tkTheme *instance();
    ~tkTheme();

    void setThemeRootPath( const QString & absPath );
    void setThemeRelativeRootPath( const QString & relativePathFromAppBinary );

    void refrehCache();
    void setCacheMaxCost( const int max );

    static QIcon icon( const QString & fileName, IconSize size = SmallIcon );
    static QString iconFullPath( const QString &fileName, IconSize size = SmallIcon );
    static QPixmap splashScreen( const QString &fileName );

protected:
    tkTheme( QObject *parent = 0, const int cacheSize = 100 );

    void setSmallIconPath( const QString &absPath );
    void setMediumIconPath( const QString &absPath );
    void setBigIconPath( const QString &absPath );

private:
    tkThemePrivate *d;
    static QPointer<tkTheme> m_Instance;
};

#endif // TKTHEME_H
