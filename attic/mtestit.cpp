/****************************************************************************
** Foo meta object code from reading C++ file 'testit.h'
**
** Created: Sun Nov 9 17:07:48 1997
**      by: The Qt Meta Object Compiler ($Revision$)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 2
#elif Q_MOC_OUTPUT_REVISION != 2
#error Moc format conflict - please regenerate all moc files
#endif

#include "testit.h"
#include <qmetaobj.h>


const char *Foo::className() const
{
    return "Foo";
}

QMetaObject *Foo::metaObj = 0;

void Foo::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QObject::className(), "QObject") != 0 )
	badSuperclassWarning("Foo","QObject");
    if ( !QObject::metaObject() )
	QObject::initMetaObject();
    typedef void(Foo::*m1_t0)(int);
    m1_t0 v1_0 = &Foo::setValue;
    QMetaData *slot_tbl = new QMetaData[1];
    slot_tbl[0].name = "setValue(int)";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    typedef void(Foo::*m2_t0)(int);
    m2_t0 v2_0 = &Foo::valueChanged;
    QMetaData *signal_tbl = new QMetaData[1];
    signal_tbl[0].name = "valueChanged(int)";
    signal_tbl[0].ptr = *((QMember*)&v2_0);
    metaObj = new QMetaObject( "Foo", "QObject",
	slot_tbl, 1,
	signal_tbl, 1 );
}

// SIGNAL valueChanged
void Foo::valueChanged( int t0 )
{
    activate_signal( "valueChanged(int)", t0 );
}
