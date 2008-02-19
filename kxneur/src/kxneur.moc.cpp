/****************************************************************************
** KXNeurApp meta object code from reading C++ file 'kxneur.h'
**
** Created: Fri Dec 14 00:09:42 2007
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.7   edited Oct 19 16:22 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "kxneur.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.7. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *KXNeurApp::className() const
{
    return "KXNeurApp";
}

QMetaObject *KXNeurApp::metaObj = 0;
static QMetaObjectCleanUp cleanUp_KXNeurApp( "KXNeurApp", &KXNeurApp::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString KXNeurApp::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "KXNeurApp", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString KXNeurApp::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "KXNeurApp", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* KXNeurApp::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = KUniqueApplication::staticMetaObject();
    static const QUMethod slot_0 = {"slotUpdateMode", 0, 0 };
    static const QUMethod slot_1 = {"slotUpdateRun", 0, 0 };
    static const QUMethod slot_2 = {"slotPref", 0, 0 };
    static const QUMethod slot_3 = {"slotExit", 0, 0 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"groupChange", 1, param_slot_4 };
    static const QUMethod slot_5 = {"setNextLang", 0, 0 };
    static const QUParameter param_slot_6[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"setMenuLang", 1, param_slot_6 };
    static const QUMethod slot_7 = {"refreshLang", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "slotUpdateMode()", &slot_0, QMetaData::Public },
	{ "slotUpdateRun()", &slot_1, QMetaData::Public },
	{ "slotPref()", &slot_2, QMetaData::Public },
	{ "slotExit()", &slot_3, QMetaData::Private },
	{ "groupChange(int)", &slot_4, QMetaData::Private },
	{ "setNextLang()", &slot_5, QMetaData::Private },
	{ "setMenuLang(int)", &slot_6, QMetaData::Private },
	{ "refreshLang()", &slot_7, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"KXNeurApp", parentObject,
	slot_tbl, 8,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_KXNeurApp.setMetaObject( metaObj );
    return metaObj;
}

void* KXNeurApp::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "KXNeurApp" ) )
	return this;
    return KUniqueApplication::qt_cast( clname );
}

bool KXNeurApp::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: slotUpdateMode(); break;
    case 1: slotUpdateRun(); break;
    case 2: slotPref(); break;
    case 3: slotExit(); break;
    case 4: groupChange((int)static_QUType_int.get(_o+1)); break;
    case 5: setNextLang(); break;
    case 6: setMenuLang((int)static_QUType_int.get(_o+1)); break;
    case 7: refreshLang(); break;
    default:
	return KUniqueApplication::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool KXNeurApp::qt_emit( int _id, QUObject* _o )
{
    return KUniqueApplication::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool KXNeurApp::qt_property( int id, int f, QVariant* v)
{
    return KUniqueApplication::qt_property( id, f, v);
}

bool KXNeurApp::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
