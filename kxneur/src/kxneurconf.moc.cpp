/****************************************************************************
** KXNPushButton meta object code from reading C++ file 'kxneurconf.h'
**
** Created: Fri Dec 14 00:09:44 2007
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.7   edited Oct 19 16:22 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "kxneurconf.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.7. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *KXNPushButton::className() const
{
    return "KXNPushButton";
}

QMetaObject *KXNPushButton::metaObj = 0;
static QMetaObjectCleanUp cleanUp_KXNPushButton( "KXNPushButton", &KXNPushButton::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString KXNPushButton::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "KXNPushButton", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString KXNPushButton::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "KXNPushButton", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* KXNPushButton::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QPushButton::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "nn", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"update", 1, param_slot_0 };
    static const QUMethod slot_1 = {"click1", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "update(int)", &slot_0, QMetaData::Public },
	{ "click1()", &slot_1, QMetaData::Public }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_charstar, 0, QUParameter::In }
    };
    static const QUMethod signal_0 = {"set_file", 1, param_signal_0 };
    static const QMetaData signal_tbl[] = {
	{ "set_file(char*)", &signal_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"KXNPushButton", parentObject,
	slot_tbl, 2,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_KXNPushButton.setMetaObject( metaObj );
    return metaObj;
}

void* KXNPushButton::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "KXNPushButton" ) )
	return this;
    return QPushButton::qt_cast( clname );
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL set_file
void KXNPushButton::set_file( char* t0 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 0 );
    if ( !clist )
	return;
    QUObject o[2];
    static_QUType_charstar.set(o+1,t0);
    activate_signal( clist, o );
}

bool KXNPushButton::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: update((int)static_QUType_int.get(_o+1)); break;
    case 1: click1(); break;
    default:
	return QPushButton::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool KXNPushButton::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: set_file((char*)static_QUType_charstar.get(_o+1)); break;
    default:
	return QPushButton::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool KXNPushButton::qt_property( int id, int f, QVariant* v)
{
    return QPushButton::qt_property( id, f, v);
}

bool KXNPushButton::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES


const char *KXNLineEdit::className() const
{
    return "KXNLineEdit";
}

QMetaObject *KXNLineEdit::metaObj = 0;
static QMetaObjectCleanUp cleanUp_KXNLineEdit( "KXNLineEdit", &KXNLineEdit::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString KXNLineEdit::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "KXNLineEdit", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString KXNLineEdit::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "KXNLineEdit", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* KXNLineEdit::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QLineEdit::staticMetaObject();
    static const QUMethod slot_0 = {"openDlg", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "openDlg()", &slot_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"KXNLineEdit", parentObject,
	slot_tbl, 1,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_KXNLineEdit.setMetaObject( metaObj );
    return metaObj;
}

void* KXNLineEdit::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "KXNLineEdit" ) )
	return this;
    return QLineEdit::qt_cast( clname );
}

bool KXNLineEdit::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: openDlg(); break;
    default:
	return QLineEdit::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool KXNLineEdit::qt_emit( int _id, QUObject* _o )
{
    return QLineEdit::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool KXNLineEdit::qt_property( int id, int f, QVariant* v)
{
    return QLineEdit::qt_property( id, f, v);
}

bool KXNLineEdit::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES


const char *XNeurPage::className() const
{
    return "XNeurPage";
}

QMetaObject *XNeurPage::metaObj = 0;
static QMetaObjectCleanUp cleanUp_XNeurPage( "XNeurPage", &XNeurPage::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString XNeurPage::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "XNeurPage", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString XNeurPage::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "XNeurPage", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* XNeurPage::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "path", &static_QUType_charstar, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"open_file", 1, param_slot_0 };
    static const QMetaData slot_tbl[] = {
	{ "open_file(char*)", &slot_0, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"XNeurPage", parentObject,
	slot_tbl, 1,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_XNeurPage.setMetaObject( metaObj );
    return metaObj;
}

void* XNeurPage::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "XNeurPage" ) )
	return this;
    return QWidget::qt_cast( clname );
}

bool XNeurPage::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: open_file((char*)static_QUType_charstar.get(_o+1)); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool XNeurPage::qt_emit( int _id, QUObject* _o )
{
    return QWidget::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool XNeurPage::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool XNeurPage::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
