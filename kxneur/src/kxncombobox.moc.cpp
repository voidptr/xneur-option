/****************************************************************************
** KXNComboBox meta object code from reading C++ file 'kxncombobox.h'
**
** Created: Fri Dec 14 00:09:43 2007
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.7   edited Oct 19 16:22 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "kxncombobox.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.7. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *KXNComboBox::className() const
{
    return "KXNComboBox";
}

QMetaObject *KXNComboBox::metaObj = 0;
static QMetaObjectCleanUp cleanUp_KXNComboBox( "KXNComboBox", &KXNComboBox::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString KXNComboBox::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "KXNComboBox", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString KXNComboBox::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "KXNComboBox", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* KXNComboBox::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QComboBox::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"enable", 1, param_slot_0 };
    static const QMetaData slot_tbl[] = {
	{ "enable(int)", &slot_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"KXNComboBox", parentObject,
	slot_tbl, 1,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_KXNComboBox.setMetaObject( metaObj );
    return metaObj;
}

void* KXNComboBox::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "KXNComboBox" ) )
	return this;
    return QComboBox::qt_cast( clname );
}

bool KXNComboBox::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: enable((int)static_QUType_int.get(_o+1)); break;
    default:
	return QComboBox::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool KXNComboBox::qt_emit( int _id, QUObject* _o )
{
    return QComboBox::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool KXNComboBox::qt_property( int id, int f, QVariant* v)
{
    return QComboBox::qt_property( id, f, v);
}

bool KXNComboBox::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
