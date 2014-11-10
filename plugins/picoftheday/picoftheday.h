/*
  This file is part of KOrganizer.
  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2007 Loïc Corbasson <loic.corbasson@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef KORG_PLUGINS_PICOFTHEDAY_PICOFTHEDAY_H
#define KORG_PLUGINS_PICOFTHEDAY_PICOFTHEDAY_H

#include <calendarviews/agenda/calendardecoration.h>
using namespace EventViews::CalendarDecoration;

#include <KIO/Job>
#include <KUrl>
class Picoftheday : public Decoration
{

public:
    Picoftheday();
    ~Picoftheday();

    Element::List createDayElements(const QDate &);

    void configure(QWidget *parent);

    QString info() const;

private:
    QSize mThumbSize;
};

class PicofthedayFactory : public DecorationFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.korganizer.Picoftheday");
public:
    Decoration *createPluginFactory()
    {
        return new Picoftheday;
    }
};

class POTDElement : public StoredElement
{
    Q_OBJECT

public:
    POTDElement(const QString &id, const QDate &date,
                const QSize &initialThumbSize);
    ~POTDElement() {}

    void setDate(const QDate &date);
    void setThumbnailSize(const QSize &size);
    /** @reimp from Element */
    QPixmap newPixmap(const QSize &size);

    /**
      Returns the thumbnail URL for a given width corresponding to a full-size image URL. */
    KUrl thumbnailUrl(const KUrl &fullSizeUrl, const int width = 0) const;

signals:
    void gotNewPixmap(const QPixmap &) const;
    void gotNewShortText(const QString &) const;
    void gotNewLongText(const QString &) const;
    void gotNewExtensiveText(const QString &) const;
    void gotNewUrl(const KUrl &) const;
    // The following three signals are only used internally
    void step1Success() const;
    void step2Success() const;
    void step3Success() const;

protected slots:
    void step1StartDownload();
    void step2GetImagePage();
    void step3GetThumbnail();

private:
    QDate mDate;
    QString mDescription;
    QSize mDlThumbSize;
    QString mFileName;
    KUrl mFullSizeImageUrl;
    float mHWRatio;
    QSize mThumbSize;
    KUrl mThumbUrl;
    bool mFirstStepCompleted;
    bool mSecondStepCompleted;
    KIO::SimpleJob *mFirstStepJob;
    KIO::SimpleJob *mSecondStepJob;
    KIO::SimpleJob *mThirdStepJob;
    QTimer *mTimer;

private slots:
    void step1Result(KJob *job);
    void step2Result(KJob *job);
    void step3Result(KJob *job);
};

#endif
