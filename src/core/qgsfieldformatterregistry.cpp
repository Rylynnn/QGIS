/***************************************************************************
  qgsfieldformatterregistry.cpp - QgsFieldFormatterRegistry

 ---------------------
 begin                : 2.12.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsfieldformatterregistry.h"
#include "qgsfieldformatter.h"

#include "qgsvaluerelationfieldformatter.h"
#include "qgsvaluemapfieldformatter.h"
#include "qgsdatetimefieldformatter.h"
#include "qgsrelationreferencefieldformatter.h"
#include "qgskeyvaluefieldformatter.h"
#include "qgslistfieldformatter.h"
#include "qgsfallbackfieldformatter.h"


QgsFieldFormatterRegistry::QgsFieldFormatterRegistry( QObject* parent )
    : QObject( parent )
{
  addFieldFormatter( new QgsValueRelationFieldFormatter() );
  addFieldFormatter( new QgsValueMapFieldFormatter() );
  addFieldFormatter( new QgsRelationReferenceFieldFormatter() );
  addFieldFormatter( new QgsKeyValueFieldFormatter() );
  addFieldFormatter( new QgsListFieldFormatter() );
  addFieldFormatter( new QgsDateTimeFieldFormatter() );

  mFallbackFieldFormatter = new QgsFallbackFieldFormatter();
}

QgsFieldFormatterRegistry::~QgsFieldFormatterRegistry()
{
  qDeleteAll( mFieldFormatters );
  delete mFallbackFieldFormatter;
}

void QgsFieldFormatterRegistry::addFieldFormatter( QgsFieldFormatter* formatter )
{
  mFieldFormatters.insert( formatter->id(), formatter );
  emit fieldFormatterAdded( formatter );
}

void QgsFieldFormatterRegistry::removeFieldFormatter( QgsFieldFormatter* formatter )
{
  removeFieldFormatter( formatter->id() );
}

void QgsFieldFormatterRegistry::removeFieldFormatter( const QString& id )
{
  if ( QgsFieldFormatter* formatter = mFieldFormatters.take( id ) )
  {
    emit fieldFormatterRemoved( formatter );
    delete formatter;
  }
}

QgsFieldFormatter* QgsFieldFormatterRegistry::fieldFormatter( const QString& id ) const
{
  return mFieldFormatters.value( id, mFallbackFieldFormatter );
}

QgsFieldFormatter* QgsFieldFormatterRegistry::fallbackFieldFormatter() const
{
  return mFallbackFieldFormatter;
}
