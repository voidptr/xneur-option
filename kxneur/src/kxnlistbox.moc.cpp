/****************************************************************************
** KXNListBox meta object code from reading C++ file 'kxnlistbox.h'
**
** Created: Fri Dec 14 00:09:44 2007
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.7   edited Oct 19 16:22 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "kxnlistbox.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.7. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *KXNListBox::className() const
{
    return "KXNListBox";
}

QMetaObject *KXNListBox::metaObj = 0;
static QMetaObjectCleanUp cleanUp_KXNListBox( "KXNListBox", &KXNListBox::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString KXNListBox::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "KXNListBox", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString KXNListBox::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "KXNListBox", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* KXNListBox::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QListBox::staticMetaObject();
    static const QUMethod slot_0 = {"deleteSelected", 0, 0 };
    static const QUMethod slot_1 = {"getProgList", 0, 0 };
    static const QUMethod slot_2 = {"getPointedProg", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "deleteSelected()", &slot_0, QMetaData::Public },
	{ "getProgList()", &slot_1, QMetaData::Public },
	{ "getPointedProg()", &slot_2, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"KXNListBox", parentObject,
	slot_tbl, 3,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_KXNListBox.setMetaObject( metaObj );
    return metaObj;
}

void* KXNListBox::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "KXNListBox" ) )
	return this;
    return QListBox::qt_cast( clname );
}

bool KXNListBox::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: deleteSelected(); break;
    case 1: getProgList(); break;
    case 2: getPointedProg(); break;
    default:
	return QListBox::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool KXNListBox::qt_emit( int _id, QUObject* _o )
{
    return QListBox::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool KXNListBox::qt_property( int id, int f, QVariant* v)
{
    return QListBox::qt_property( id, f, v);
}

bool KXNListBox::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
