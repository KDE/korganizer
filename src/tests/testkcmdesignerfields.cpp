/*
    SPDX-FileCopyrightText: 2010 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "../kcmdesignerfields.h"

#include <KAboutData>

#include <QApplication>
#include <QCommandLineParser>

class MyDesignerFields : public KCMDesignerFields
{
    Q_OBJECT
public:
    MyDesignerFields()
        : KCMDesignerFields(nullptr)
    {
    }

    QString localUiDir() override
    {
        return QStringLiteral(TESTKCMDESIGNERCURRENTDIR);
    }

    QString uiPath() override
    {
        return QStringLiteral(TESTKCMDESIGNERCURRENTDIR);
    }

    void writeActivePages(const QStringList &) override
    {
    }

    QStringList readActivePages() override
    {
        return QStringList();
    }

    QString applicationName() override
    {
        return QStringLiteral("textkcmdesignerfields");
    }
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    KAboutData aboutData(QStringLiteral("testkcmdesignerfields"), QString(), QStringLiteral("0.1"));
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    auto kcm = new MyDesignerFields();
    kcm->show();

    app.exec();
    delete kcm;
}

#include "testkcmdesignerfields.moc"
