/***************************************************************************
    qgswcsdataitems.cpp
    ---------------------
    begin                : 2 July, 2012
    copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsapplication.h"
#include "qgswcsdataitems.h"
#include "qgswcsprovider.h"
#include "qgslogger.h"
#include "qgsdatasourceuri.h"
#include "qgswcssourceselect.h"
#include "qgsowsconnection.h"
#include "qgsnewhttpconnection.h"

#include <QFileInfo>
#include <QSettings>

QgsWCSConnectionItem::QgsWCSConnectionItem( QgsDataItem* parent, QString name, QString path, QString uri )
    : QgsDataCollectionItem( parent, name, path )
    , mUri( uri )
{
  mIconName = QStringLiteral( "mIconWcs.svg" );
}

QgsWCSConnectionItem::~QgsWCSConnectionItem()
{
}

QVector<QgsDataItem*> QgsWCSConnectionItem::createChildren()
{
  QVector<QgsDataItem*> children;

  QgsDataSourceUri uri;
  uri.setEncodedUri( mUri );
  QgsDebugMsg( "mUri = " + mUri );

  mCapabilities.setUri( uri );

  // Attention: supportedLayers() gives tree leafes, not top level
  if ( !mCapabilities.lastError().isEmpty() )
  {
    //children.append( new QgsErrorItem( this, tr( "Failed to retrieve layers" ), mPath + "/error" ) );
    // TODO: show the error without adding child
    return children;
  }

  Q_FOREACH ( const QgsWcsCoverageSummary& coverageSummary, mCapabilities.capabilities().contents.coverageSummary )
  {
    // Attention, the name may be empty
    QgsDebugMsg( QString::number( coverageSummary.orderId ) + ' ' + coverageSummary.identifier + ' ' + coverageSummary.title );
    QString pathName = coverageSummary.identifier.isEmpty() ? QString::number( coverageSummary.orderId ) : coverageSummary.identifier;

    QgsWCSLayerItem * layer = new QgsWCSLayerItem( this, coverageSummary.title, mPath + '/' + pathName, mCapabilities.capabilities(), uri, coverageSummary );

    children.append( layer );
  }
  return children;
}

bool QgsWCSConnectionItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
  {
    return false;
  }
  const QgsWCSConnectionItem *o = dynamic_cast<const QgsWCSConnectionItem *>( other );
  if ( !o )
  {
    return false;
  }

  return ( mPath == o->mPath && mName == o->mName );
}

QList<QAction*> QgsWCSConnectionItem::actions()
{
  QList<QAction*> lst;

  QAction* actionEdit = new QAction( tr( "Edit..." ), this );
  connect( actionEdit, SIGNAL( triggered() ), this, SLOT( editConnection() ) );
  lst.append( actionEdit );

  QAction* actionDelete = new QAction( tr( "Delete" ), this );
  connect( actionDelete, SIGNAL( triggered() ), this, SLOT( deleteConnection() ) );
  lst.append( actionDelete );

  return lst;
}

void QgsWCSConnectionItem::editConnection()
{
  QgsNewHttpConnection nc( nullptr, QStringLiteral( "/Qgis/connections-wcs/" ), mName );

  if ( nc.exec() )
  {
    // the parent should be updated
    mParent->refresh();
  }
}

void QgsWCSConnectionItem::deleteConnection()
{
  QgsOwsConnection::deleteConnection( QStringLiteral( "WCS" ), mName );
  // the parent should be updated
  mParent->refresh();
}


// ---------------------------------------------------------------------------

QgsWCSLayerItem::QgsWCSLayerItem( QgsDataItem* parent, QString name, QString path, const QgsWcsCapabilitiesProperty& capabilitiesProperty, const QgsDataSourceUri &dataSourceUri, const QgsWcsCoverageSummary& coverageSummary )
    : QgsLayerItem( parent, name, path, QString(), QgsLayerItem::Raster, QStringLiteral( "wcs" ) )
    , mCapabilities( capabilitiesProperty )
    , mDataSourceUri( dataSourceUri )
    , mCoverageSummary( coverageSummary )
{
  mSupportedCRS = mCoverageSummary.supportedCrs;
  QgsDebugMsg( "uri = " + mDataSourceUri.encodedUri() );
  mUri = createUri();
  // Populate everything, it costs nothing, all info about layers is collected
  Q_FOREACH ( const QgsWcsCoverageSummary& coverageSummary, mCoverageSummary.coverageSummary )
  {
    // Attention, the name may be empty
    QgsDebugMsg( QString::number( coverageSummary.orderId ) + ' ' + coverageSummary.identifier + ' ' + coverageSummary.title );
    QString pathName = coverageSummary.identifier.isEmpty() ? QString::number( coverageSummary.orderId ) : coverageSummary.identifier;
    QgsWCSLayerItem * layer = new QgsWCSLayerItem( this, coverageSummary.title, mPath + '/' + pathName, mCapabilities, mDataSourceUri, coverageSummary );
    mChildren.append( layer );
  }

  if ( mChildren.isEmpty() )
  {
    mIconName = QStringLiteral( "mIconWcs.svg" );
  }
  setState( Populated );
}

QgsWCSLayerItem::~QgsWCSLayerItem()
{
}

QString QgsWCSLayerItem::createUri()
{
  if ( mCoverageSummary.identifier.isEmpty() )
    return QLatin1String( "" ); // layer collection

  // Number of styles must match number of layers
  mDataSourceUri.setParam( QStringLiteral( "identifier" ), mCoverageSummary.identifier );

  // TODO(?): with WCS 1.0 GetCapabilities does not contain CRS and formats,
  // to get them we would need to call QgsWcsCapabilities::describeCoverage
  // but it is problematic to get QgsWcsCapabilities here (copy not allowed
  // by QObject, pointer is dangerous (OWS provider is changing parent))
  // We leave CRS and format default for now.

  QString format;
  // get first supported by GDAL and server
  // TODO
  //QStringList mimes = QgsGdalProvider::supportedMimes().keys();
  QStringList mimes;
  // prefer tiff
  if ( mimes.contains( QStringLiteral( "image/tiff" ) ) && mCoverageSummary.supportedFormat.contains( QStringLiteral( "image/tiff" ) ) )
  {
    format = QStringLiteral( "image/tiff" );
  }
  else
  {
    Q_FOREACH ( const QString& f, mimes )
    {
      if ( mCoverageSummary.supportedFormat.indexOf( f ) >= 0 )
      {
        format = f;
        break;
      }
    }
  }
  if ( !format.isEmpty() )
  {
    mDataSourceUri.setParam( QStringLiteral( "format" ), format );
  }

  QString crs;

  // TODO: prefer project CRS
  // get first known if possible
  QgsCoordinateReferenceSystem testCrs;
  Q_FOREACH ( const QString& c, mCoverageSummary.supportedCrs )
  {
    testCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( c );
    if ( testCrs.isValid() )
    {
      crs = c;
      break;
    }
  }
  if ( crs.isEmpty() && !mCoverageSummary.supportedCrs.isEmpty() )
  {
    crs = mCoverageSummary.supportedCrs.value( 0 );
  }
  if ( !crs.isEmpty() )
  {
    mDataSourceUri.setParam( QStringLiteral( "crs" ), crs );
  }

  return mDataSourceUri.encodedUri();
}

// ---------------------------------------------------------------------------

QgsWCSRootItem::QgsWCSRootItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mCapabilities |= Fast;
  mIconName = QStringLiteral( "mIconWcs.svg" );
  populate();
}

QgsWCSRootItem::~QgsWCSRootItem()
{
}

QVector<QgsDataItem*>QgsWCSRootItem::createChildren()
{
  QVector<QgsDataItem*> connections;
  Q_FOREACH ( const QString& connName, QgsOwsConnection::connectionList( "WCS" ) )
  {
    QgsOwsConnection connection( QStringLiteral( "WCS" ), connName );
    QgsDataItem * conn = new QgsWCSConnectionItem( this, connName, mPath + '/' + connName, connection.uri().encodedUri() );
    connections.append( conn );
  }
  return connections;
}

QList<QAction*> QgsWCSRootItem::actions()
{
  QList<QAction*> lst;

  QAction* actionNew = new QAction( tr( "New Connection..." ), this );
  connect( actionNew, SIGNAL( triggered() ), this, SLOT( newConnection() ) );
  lst.append( actionNew );

  return lst;
}


QWidget* QgsWCSRootItem::paramWidget()
{
  QgsWCSSourceSelect *select = new QgsWCSSourceSelect( nullptr, 0, true, true );
  connect( select, SIGNAL( connectionsChanged() ), this, SLOT( connectionsChanged() ) );
  return select;
}

void QgsWCSRootItem::connectionsChanged()
{
  refresh();
}

void QgsWCSRootItem::newConnection()
{
  QgsNewHttpConnection nc( nullptr, QStringLiteral( "/Qgis/connections-wcs/" ) );

  if ( nc.exec() )
  {
    refresh();
  }
}

// ---------------------------------------------------------------------------

QGISEXTERN int dataCapabilities()
{
  return  QgsDataProvider::Net;
}

QGISEXTERN QgsDataItem * dataItem( QString path, QgsDataItem* parentItem )
{
  QgsDebugMsg( "thePath = " + path );
  if ( path.isEmpty() )
  {
    // Top level WCS
    return new QgsWCSRootItem( parentItem, QStringLiteral( "WCS" ), QStringLiteral( "wcs:" ) );
  }

  // path schema: wcs:/connection name (used by OWS)
  if ( path.startsWith( QLatin1String( "wcs:/" ) ) )
  {
    QString connectionName = path.split( '/' ).last();
    if ( QgsOwsConnection::connectionList( QStringLiteral( "WCS" ) ).contains( connectionName ) )
    {
      QgsOwsConnection connection( QStringLiteral( "WCS" ), connectionName );
      return new QgsWCSConnectionItem( parentItem, QStringLiteral( "WCS" ), path, connection.uri().encodedUri() );
    }
  }

  return nullptr;
}

QGISEXTERN QgsWCSSourceSelect * selectWidget( QWidget * parent, Qt::WindowFlags fl )
{
  return new QgsWCSSourceSelect( parent, fl );
}

