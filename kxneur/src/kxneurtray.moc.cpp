/****************************************************************************
** KXNeurTray meta object code from reading C++ file 'kxneurtray.h'
**
** Created: Fri Dec 14 00:09:42 2007
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.7   edited Oct 19 16:22 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "kxneurtray.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.7. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *KXNeurTray::className() const
{
    return "KXNeurTray";
}

QMetaObject *KXNeurTray::metaObj = 0;
static QMetaObjectCleanUp cleanUp_KXNeurTray( "KXNeurTray", &KXNeurTray::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString KXNeurTray::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "KXNeurTray", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString KXNeurTray::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "KXNeurTray", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* KXNeurTray::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = KSystemTray::staticMetaObject();
    static const QUMethod slot_0 = {"slotHelp", 0, 0 };
    static const QUMethod slot_1 = {"slotAbout", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "slotHelp()", &slot_0, QMetaData::Public },
	{ "slotAbout()", &slot_1, QMetaData::Public }
    };
    static const QUMethod signal_0 = {"clicked", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "clicked()", &signal_0, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"KXNeurTray", parentObject,
	slot_tbl, 2,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_KXNeurTray.setMetaObject( metaObj );
    return metaObj;
}

void* KXNeurTray::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "KXNeurTray" ) )
	return this;
    return KSystemTray::qt_cast( clname );
}

// SIGNAL clicked
void KXNeurTray::clicked()
{
    activate_signal( staticMetaObject()->signalOffset() + 0 );
}

bool KXNeurTray::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: slotHelp(); break;
    case 1: slotAbout(); break;
    default:
	return KSystemTray::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool KXNeurTray::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: clicked(); break;
    default:
	return KSystemTray::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool KXNeurTray::qt_property( int id, int f, QVariant* v)
{
    return KSystemTray::qt_property( id, f, v);
}

bool KXNeurTray::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
