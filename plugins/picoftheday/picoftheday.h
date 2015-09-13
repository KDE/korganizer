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

#include <EventViews/CalendarDecoration>
using namespace EventViews::CalendarDecoration;

#include <KIO/Job>
#include <QUrl>
class Picoftheday : public Decoration
{

public:
    Picoftheday();
    ~Picoftheday();

    Element::List createDayElements(const QDate &) Q_DECL_OVERRIDE;

    void configure(QWidget *parent) Q_DECL_OVERRIDE;

    QString info() const Q_DECL_OVERRIDE;

private:
    QSize mThumbSize;
};

class PicofthedayFactory : public DecorationFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.korganizer.Picoftheday")
public:
    Decoration *createPluginFactory() Q_DECL_OVERRIDE {
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
    QPixmap newPixmap(const QSize &size) Q_DECL_OVERRIDE;

    /**
      Returns the thumbnail URL for a given width corresponding to a full-size image URL. */
    QUrl thumbnailUrl(const QUrl &fullSizeUrl, const int width = 0) const;

Q_SIGNALS:
    void gotNewPixmap(const QPixmap &) const;
    void gotNewShortText(const QString &) const;
    void gotNewLongText(const QString &) const;
    void gotNewExtensiveText(const QString &) const;
    void gotNewUrl(const QUrl &) const;
    // The following three signals are only used internally
    void step1Success() const;
    void step2Success() const;
    void step3Success() const;

protected Q_SLOTS:
    void step1StartDownload();
    void step2GetImagePage();
    void step3GetThumbnail();

private:
    QDate mDate;
    QString mDescription;
    QSize mDlThumbSize;
    QString mFileName;
    QUrl mFullSizeImageUrl;
    float mHWRatio;
    QSize mThumbSize;
    QUrl mThumbUrl;
    bool mFirstStepCompleted;
    bool mSecondStepCompleted;
    KIO::SimpleJob *mFirstStepJob;
    KIO::SimpleJob *mSecondStepJob;
    KIO::SimpleJob *mThirdStepJob;
    QTimer *mTimer;

private Q_SLOTS:
    void step1Result(KJob *job);
    void step2Result(KJob *job);
    void step3Result(KJob *job);
};

#endif
