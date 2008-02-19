/****************************************************************************
** KXNKeyboard meta object code from reading C++ file 'kxnkeyboard.h'
**
** Created: Fri Dec 14 00:09:45 2007
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.7   edited Oct 19 16:22 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "kxnkeyboard.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.7. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *KXNKeyboard::className() const
{
    return "KXNKeyboard";
}

QMetaObject *KXNKeyboard::metaObj = 0;
static QMetaObjectCleanUp cleanUp_KXNKeyboard( "KXNKeyboard", &KXNKeyboard::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString KXNKeyboard::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "KXNKeyboard", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString KXNKeyboard::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "KXNKeyboard", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* KXNKeyboard::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QObject::staticMetaObject();
    static const QUParameter param_signal_0[] = {
	{ "groupno", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod signal_0 = {"groupChanged", 1, param_signal_0 };
    static const QUMethod signal_1 = {"layoutChanged", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "groupChanged(int)", &signal_0, QMetaData::Private },
	{ "layoutChanged()", &signal_1, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"KXNKeyboard", parentObject,
	0, 0,
	signal_tbl, 2,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_KXNKeyboard.setMetaObject( metaObj );
    return metaObj;
}

void* KXNKeyboard::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "KXNKeyboard" ) )
	return this;
    return QObject::qt_cast( clname );
}

// SIGNAL groupChanged
void KXNKeyboard::groupChanged( int t0 )
{
    activate_signal( staticMetaObject()->signalOffset() + 0, t0 );
}

// SIGNAL layoutChanged
void KXNKeyboard::layoutChanged()
{
    activate_signal( staticMetaObject()->signalOffset() + 1 );
}

bool KXNKeyboard::qt_invoke( int _id, QUObject* _o )
{
    return QObject::qt_invoke(_id,_o);
}

bool KXNKeyboard::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: groupChanged((int)static_QUType_int.get(_o+1)); break;
    case 1: layoutChanged(); break;
    default:
	return QObject::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool KXNKeyboard::qt_property( int id, int f, QVariant* v)
{
    return QObject::qt_property( id, f, v);
}

bool KXNKeyboard::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
