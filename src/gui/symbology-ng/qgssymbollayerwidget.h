/***************************************************************************
 qgssymbollayerwidget.h - symbol layer widgets

 ---------------------
 begin                : November 2009
 copyright            : (C) 2009 by Martin Dobias
 email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSYMBOLLAYERWIDGET_H
#define QGSSYMBOLLAYERWIDGET_H

#include "qgspropertyoverridebutton.h"
#include "qgssymbolwidgetcontext.h"
#include "qgssymbollayer.h"
#include <QWidget>
#include <QStandardItemModel>

class QgsVectorLayer;
class QgsMapCanvas;

/** \ingroup gui
 * \class QgsSymbolLayerWidget
 */
class GUI_EXPORT QgsSymbolLayerWidget : public QWidget, protected QgsExpressionContextGenerator
{
    Q_OBJECT

  public:
    QgsSymbolLayerWidget( QWidget* parent, const QgsVectorLayer* vl = nullptr )
        : QWidget( parent )
        , mVectorLayer( vl )
        , mMapCanvas( nullptr )
    {}

    virtual void setSymbolLayer( QgsSymbolLayer* layer ) = 0;
    virtual QgsSymbolLayer* symbolLayer() = 0;

    /** Sets the context in which the symbol widget is shown, e.g., the associated map canvas and expression contexts.
     * @param context symbol widget context
     * @see context()
     * @note added in QGIS 3.0
     */
    void setContext( const QgsSymbolWidgetContext& context );

    /** Returns the context in which the symbol widget is shown, e.g., the associated map canvas and expression contexts.
     * @see setContext()
     * @note added in QGIS 3.0
     */
    QgsSymbolWidgetContext context() const;

    /** Returns the vector layer associated with the widget.
     * @note added in QGIS 2.12
     */
    const QgsVectorLayer* vectorLayer() const { return mVectorLayer; }

  protected:

    /**
     * Registers a data defined override button. Handles setting up connections
     * for the button and initializing the button to show the correct descriptions
     * and help text for the associated property.
     * @note added in QGIS 3.0
     */
    void registerDataDefinedButton( QgsPropertyOverrideButton* button, QgsSymbolLayer::Property key );

    QgsExpressionContext createExpressionContext() const override;

  private:
    const QgsVectorLayer* mVectorLayer = nullptr;

    QgsMapCanvas* mMapCanvas = nullptr;

  signals:

    /**
     * Should be emitted whenever configuration changes happened on this symbol layer configuration.
     * If the subsymbol is changed, {@link symbolChanged()} should be emitted instead.
     */
    void changed();

    /**
     * Should be emitted whenever the sub symbol changed on this symbol layer configuration.
     * Normally {@link changed()} should be preferred.
     *
     * @see {@link changed()}
     */
    void symbolChanged();

  protected slots:
    void updateDataDefinedProperty();

  private:
    QgsSymbolWidgetContext mContext;
};

///////////

#include "ui_widget_simpleline.h"

class QgsSimpleLineSymbolLayer;

/** \ingroup gui
 * \class QgsSimpleLineSymbolLayerWidget
 */
class GUI_EXPORT QgsSimpleLineSymbolLayerWidget : public QgsSymbolLayerWidget, private Ui::WidgetSimpleLine
{
    Q_OBJECT

  public:
    QgsSimpleLineSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent = nullptr );

    static QgsSymbolLayerWidget* create( const QgsVectorLayer* vl ) { return new QgsSimpleLineSymbolLayerWidget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayer* layer ) override;
    virtual QgsSymbolLayer* symbolLayer() override;

  public slots:
    void penWidthChanged();
    void colorChanged( const QColor& color );
    void penStyleChanged();
    void offsetChanged();
    void on_mCustomCheckBox_stateChanged( int state );
    void on_mChangePatternButton_clicked();
    void on_mPenWidthUnitWidget_changed();
    void on_mOffsetUnitWidget_changed();
    void on_mDashPatternUnitWidget_changed();
    void on_mDrawInsideCheckBox_stateChanged( int state );

  protected:
    QgsSimpleLineSymbolLayer* mLayer = nullptr;

    //creates a new icon for the 'change pattern' button
    void updatePatternIcon();

  private slots:

    void updateAssistantSymbol();

  private:

    std::shared_ptr< QgsLineSymbol > mAssistantPreviewSymbol;

};

///////////

#include "ui_widget_simplemarker.h"

class QgsSimpleMarkerSymbolLayer;

/** \ingroup gui
 * \class QgsSimpleMarkerSymbolLayerWidget
 */
class GUI_EXPORT QgsSimpleMarkerSymbolLayerWidget : public QgsSymbolLayerWidget, private Ui::WidgetSimpleMarker
{
    Q_OBJECT

  public:
    QgsSimpleMarkerSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent = nullptr );

    static QgsSymbolLayerWidget* create( const QgsVectorLayer* vl ) { return new QgsSimpleMarkerSymbolLayerWidget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayer* layer ) override;
    virtual QgsSymbolLayer* symbolLayer() override;

  public slots:

    void setColorStroke( const QColor& color );
    void setColorFill( const QColor& color );
    void setSize();
    void setAngle();
    void setOffset();
    void on_mSizeUnitWidget_changed();
    void on_mOffsetUnitWidget_changed();
    void on_mStrokeWidthUnitWidget_changed();
    void on_mStrokeStyleComboBox_currentIndexChanged( int index );
    void on_mStrokeWidthSpinBox_valueChanged( double d );
    void on_mHorizontalAnchorComboBox_currentIndexChanged( int index );
    void on_mVerticalAnchorComboBox_currentIndexChanged( int index );

  protected:
    QgsSimpleMarkerSymbolLayer* mLayer = nullptr;

  private slots:
    void setShape();
    void updateAssistantSymbol();
    void penJoinStyleChanged();

  private:

    std::shared_ptr< QgsMarkerSymbol > mAssistantPreviewSymbol;
};

///////////

#include "ui_widget_simplefill.h"

class QgsSimpleFillSymbolLayer;

/** \ingroup gui
 * \class QgsSimpleFillSymbolLayerWidget
 */
class GUI_EXPORT QgsSimpleFillSymbolLayerWidget : public QgsSymbolLayerWidget, private Ui::WidgetSimpleFill
{
    Q_OBJECT

  public:
    QgsSimpleFillSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent = nullptr );

    static QgsSymbolLayerWidget* create( const QgsVectorLayer* vl ) { return new QgsSimpleFillSymbolLayerWidget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayer* layer ) override;
    virtual QgsSymbolLayer* symbolLayer() override;

  public slots:
    void setColor( const QColor& color );
    void setStrokeColor( const QColor& color );
    void setBrushStyle();
    void strokeWidthChanged();
    void strokeStyleChanged();
    void offsetChanged();
    void on_mStrokeWidthUnitWidget_changed();
    void on_mOffsetUnitWidget_changed();

  protected:
    QgsSimpleFillSymbolLayer* mLayer = nullptr;
};


///////////

#include "ui_widget_filledmarker.h"

class QgsFilledMarkerSymbolLayer;

/** \ingroup gui
 * \class QgsFilledMarkerSymbolLayerWidget
 * \brief Widget for configuring QgsFilledMarkerSymbolLayer symbol layers.
 * \note Added in version 2.16
 */
class GUI_EXPORT QgsFilledMarkerSymbolLayerWidget : public QgsSymbolLayerWidget, private Ui::WidgetFilledMarker
{
    Q_OBJECT

  public:

    /** Constructor for QgsFilledMarkerSymbolLayerWidget.
     * @param vl associated vector layer
     * @param parent parent widget
     */
    QgsFilledMarkerSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent = nullptr );

    /** Creates a new QgsFilledMarkerSymbolLayerWidget.
     * @param vl associated vector layer
     */
    static QgsSymbolLayerWidget* create( const QgsVectorLayer* vl ) { return new QgsFilledMarkerSymbolLayerWidget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayer* layer ) override;
    virtual QgsSymbolLayer* symbolLayer() override;

  protected:
    QgsFilledMarkerSymbolLayer* mLayer = nullptr;

  private slots:

    void updateAssistantSymbol();
    void setShape();
    void setSize();
    void setAngle();
    void setOffset();
    void on_mSizeUnitWidget_changed();
    void on_mOffsetUnitWidget_changed();
    void on_mHorizontalAnchorComboBox_currentIndexChanged( int index );
    void on_mVerticalAnchorComboBox_currentIndexChanged( int index );

  private:

    std::shared_ptr< QgsMarkerSymbol > mAssistantPreviewSymbol;
};

///////////

#include "ui_widget_gradientfill.h"

class QgsGradientFillSymbolLayer;

/** \ingroup gui
 * \class QgsGradientFillSymbolLayerWidget
 */
class GUI_EXPORT QgsGradientFillSymbolLayerWidget : public QgsSymbolLayerWidget, private Ui::WidgetGradientFill
{
    Q_OBJECT

  public:
    QgsGradientFillSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent = nullptr );

    static QgsSymbolLayerWidget* create( const QgsVectorLayer* vl ) { return new QgsGradientFillSymbolLayerWidget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayer* layer ) override;
    virtual QgsSymbolLayer* symbolLayer() override;

  public slots:
    void setColor( const QColor& color );
    void setColor2( const QColor& color );

    /** Applies the color ramp passed on by the color ramp button
     */
    void applyColorRamp();

    void setGradientType( int index );
    void setCoordinateMode( int index );
    void setGradientSpread( int index );
    void offsetChanged();
    void referencePointChanged();
    void on_mOffsetUnitWidget_changed();
    void colorModeChanged();
    void on_mSpinAngle_valueChanged( double value );

  protected:
    QgsGradientFillSymbolLayer* mLayer = nullptr;
};

///////////

#include "ui_widget_shapeburstfill.h"

class QgsShapeburstFillSymbolLayer;

/** \ingroup gui
 * \class QgsShapeburstFillSymbolLayerWidget
 */
class GUI_EXPORT QgsShapeburstFillSymbolLayerWidget : public QgsSymbolLayerWidget, private Ui::WidgetShapeburstFill
{
    Q_OBJECT

  public:
    QgsShapeburstFillSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent = nullptr );

    static QgsSymbolLayerWidget* create( const QgsVectorLayer* vl ) { return new QgsShapeburstFillSymbolLayerWidget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayer* layer ) override;
    virtual QgsSymbolLayer* symbolLayer() override;

  public slots:
    void setColor( const QColor& color );
    void setColor2( const QColor& color );
    void colorModeChanged();
    void on_mSpinBlurRadius_valueChanged( int value );
    void on_mSpinMaxDistance_valueChanged( double value );
    void on_mDistanceUnitWidget_changed();
    void on_mRadioUseWholeShape_toggled( bool value );
    void applyColorRamp();
    void offsetChanged();
    void on_mOffsetUnitWidget_changed();
    void on_mIgnoreRingsCheckBox_stateChanged( int state );

  protected:
    QgsShapeburstFillSymbolLayer* mLayer = nullptr;
};

///////////

#include "ui_widget_markerline.h"

class QgsMarkerLineSymbolLayer;

/** \ingroup gui
 * \class QgsMarkerLineSymbolLayerWidget
 */
class GUI_EXPORT QgsMarkerLineSymbolLayerWidget : public QgsSymbolLayerWidget, private Ui::WidgetMarkerLine
{
    Q_OBJECT

  public:
    QgsMarkerLineSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent = nullptr );

    static QgsSymbolLayerWidget* create( const QgsVectorLayer* vl ) { return new QgsMarkerLineSymbolLayerWidget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayer* layer ) override;
    virtual QgsSymbolLayer* symbolLayer() override;

  public slots:

    void setInterval( double val );
    void setOffsetAlongLine( double val );
    void setRotate();
    void setOffset();
    void setPlacement();
    void on_mIntervalUnitWidget_changed();
    void on_mOffsetUnitWidget_changed();
    void on_mOffsetAlongLineUnitWidget_changed();

  protected:

    QgsMarkerLineSymbolLayer* mLayer = nullptr;
};


///////////

#include "ui_widget_svgmarker.h"

class QgsSvgMarkerSymbolLayer;

/** \ingroup gui
 * \class QgsSvgMarkerSymbolLayerWidget
 */
class GUI_EXPORT QgsSvgMarkerSymbolLayerWidget : public QgsSymbolLayerWidget, private Ui::WidgetSvgMarker
{
    Q_OBJECT

  public:
    QgsSvgMarkerSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent = nullptr );

    static QgsSymbolLayerWidget* create( const QgsVectorLayer* vl ) { return new QgsSvgMarkerSymbolLayerWidget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayer* layer ) override;
    virtual QgsSymbolLayer* symbolLayer() override;

  public slots:
    void setName( const QModelIndex& idx );
    void populateIcons( const QModelIndex& idx );
    void setSize();
    void setAngle();
    void setOffset();
    void on_mFileToolButton_clicked();
    void on_mFileLineEdit_textEdited( const QString& text );
    void on_mFileLineEdit_editingFinished();
    void on_mChangeColorButton_colorChanged( const QColor& color );
    void on_mChangeStrokeColorButton_colorChanged( const QColor& color );
    void on_mStrokeWidthSpinBox_valueChanged( double d );
    void on_mSizeUnitWidget_changed();
    void on_mStrokeWidthUnitWidget_changed();
    void on_mOffsetUnitWidget_changed();
    void on_mHorizontalAnchorComboBox_currentIndexChanged( int index );
    void on_mVerticalAnchorComboBox_currentIndexChanged( int index );

  protected:

    void populateList();
    //update gui for svg file (insert new path, update activation of gui elements for svg params)
    void setGuiForSvg( const QgsSvgMarkerSymbolLayer* layer );

    QgsSvgMarkerSymbolLayer* mLayer = nullptr;

  private slots:

    void updateAssistantSymbol();

  private:

    std::shared_ptr< QgsMarkerSymbol > mAssistantPreviewSymbol;

};

///////////

#include "ui_widget_rasterfill.h"

class QgsRasterFillSymbolLayer;

/** \ingroup gui
 * \class QgsRasterFillSymbolLayerWidget
 */
class GUI_EXPORT QgsRasterFillSymbolLayerWidget : public QgsSymbolLayerWidget, private Ui::WidgetRasterFill
{
    Q_OBJECT

  public:
    QgsRasterFillSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent = nullptr );

    static QgsSymbolLayerWidget* create( const QgsVectorLayer* vl ) { return new QgsRasterFillSymbolLayerWidget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayer* layer ) override;
    virtual QgsSymbolLayer* symbolLayer() override;

  protected:
    QgsRasterFillSymbolLayer* mLayer = nullptr;

  private slots:
    void on_mBrowseToolButton_clicked();
    void on_mImageLineEdit_editingFinished();
    void setCoordinateMode( int index );
    void on_mSpinTransparency_valueChanged( int value );
    void offsetChanged();
    void on_mOffsetUnitWidget_changed();
    void on_mRotationSpinBox_valueChanged( double d );
    void on_mWidthUnitWidget_changed();
    void on_mWidthSpinBox_valueChanged( double d );

  private:
    void updatePreviewImage();
};

///////////

#include "ui_widget_svgfill.h"

class QgsSVGFillSymbolLayer;

/** \ingroup gui
 * \class QgsSVGFillSymbolLayerWidget
 */
class GUI_EXPORT QgsSVGFillSymbolLayerWidget : public QgsSymbolLayerWidget, private Ui::WidgetSVGFill
{
    Q_OBJECT

  public:
    QgsSVGFillSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent = nullptr );

    static QgsSymbolLayerWidget* create( const QgsVectorLayer* vl ) { return new QgsSVGFillSymbolLayerWidget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayer* layer ) override;
    virtual QgsSymbolLayer* symbolLayer() override;

  protected:
    QgsSVGFillSymbolLayer* mLayer = nullptr;
    void insertIcons();

    /** Enables or disables svg fill color, stroke color and stroke width based on whether the
     * svg file supports custom parameters.
     * @param resetValues set to true to overwrite existing layer fill color, stroke color and stroke width
     * with default values from svg file
     */
    void updateParamGui( bool resetValues = true );

  private slots:
    void on_mBrowseToolButton_clicked();
    void on_mTextureWidthSpinBox_valueChanged( double d );
    void on_mSVGLineEdit_textEdited( const QString & text );
    void on_mSVGLineEdit_editingFinished();
    void setFile( const QModelIndex& item );
    void populateIcons( const QModelIndex& item );
    void on_mRotationSpinBox_valueChanged( double d );
    void on_mChangeColorButton_colorChanged( const QColor& color );
    void on_mChangeStrokeColorButton_colorChanged( const QColor& color );
    void on_mStrokeWidthSpinBox_valueChanged( double d );
    void on_mTextureWidthUnitWidget_changed();
    void on_mSvgStrokeWidthUnitWidget_changed();
};

//////////

#include "ui_widget_linepatternfill.h"

class QgsLinePatternFillSymbolLayer;

/** \ingroup gui
 * \class QgsLinePatternFillSymbolLayerWidget
 */
class GUI_EXPORT QgsLinePatternFillSymbolLayerWidget : public QgsSymbolLayerWidget, private Ui::WidgetLinePatternFill
{
    Q_OBJECT

  public:

    QgsLinePatternFillSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent = nullptr );
    static QgsSymbolLayerWidget* create( const QgsVectorLayer* vl ) { return new QgsLinePatternFillSymbolLayerWidget( vl ); }

    virtual void setSymbolLayer( QgsSymbolLayer* layer ) override;
    virtual QgsSymbolLayer* symbolLayer() override;

  protected:
    QgsLinePatternFillSymbolLayer* mLayer = nullptr;

  private slots:
    void on_mAngleSpinBox_valueChanged( double d );
    void on_mDistanceSpinBox_valueChanged( double d );
    void on_mOffsetSpinBox_valueChanged( double d );
    void on_mDistanceUnitWidget_changed();
    void on_mOffsetUnitWidget_changed();
};

//////////

#include "ui_widget_pointpatternfill.h"

class QgsPointPatternFillSymbolLayer;

/** \ingroup gui
 * \class QgsPointPatternFillSymbolLayerWidget
 */
class GUI_EXPORT QgsPointPatternFillSymbolLayerWidget: public QgsSymbolLayerWidget, private Ui::WidgetPointPatternFill
{
    Q_OBJECT

  public:
    QgsPointPatternFillSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent = nullptr );
    static QgsSymbolLayerWidget* create( const QgsVectorLayer* vl ) { return new QgsPointPatternFillSymbolLayerWidget( vl ); }

    virtual void setSymbolLayer( QgsSymbolLayer* layer ) override;
    virtual QgsSymbolLayer* symbolLayer() override;

  protected:
    QgsPointPatternFillSymbolLayer* mLayer = nullptr;

  private slots:
    void on_mHorizontalDistanceSpinBox_valueChanged( double d );
    void on_mVerticalDistanceSpinBox_valueChanged( double d );
    void on_mHorizontalDisplacementSpinBox_valueChanged( double d );
    void on_mVerticalDisplacementSpinBox_valueChanged( double d );
    void on_mHorizontalDistanceUnitWidget_changed();
    void on_mVerticalDistanceUnitWidget_changed();
    void on_mHorizontalDisplacementUnitWidget_changed();
    void on_mVerticalDisplacementUnitWidget_changed();
};

/////////

#include "ui_widget_fontmarker.h"

class QgsFontMarkerSymbolLayer;
class CharacterWidget;

/** \ingroup gui
 * \class QgsFontMarkerSymbolLayerWidget
 */
class GUI_EXPORT QgsFontMarkerSymbolLayerWidget : public QgsSymbolLayerWidget, private Ui::WidgetFontMarker
{
    Q_OBJECT

  public:
    QgsFontMarkerSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent = nullptr );

    static QgsSymbolLayerWidget* create( const QgsVectorLayer* vl ) { return new QgsFontMarkerSymbolLayerWidget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayer* layer ) override;
    virtual QgsSymbolLayer* symbolLayer() override;

  public slots:
    void setFontFamily( const QFont& font );
    void setColor( const QColor& color );

    /** Set stroke color.
     * @note added in 2.16 */
    void setColorStroke( const QColor& color );
    void setSize( double size );
    void setAngle( double angle );
    void setCharacter( QChar chr );
    void setOffset();
    void on_mSizeUnitWidget_changed();
    void on_mOffsetUnitWidget_changed();
    void on_mStrokeWidthUnitWidget_changed();
    void on_mStrokeWidthSpinBox_valueChanged( double d );
    void on_mHorizontalAnchorComboBox_currentIndexChanged( int index );
    void on_mVerticalAnchorComboBox_currentIndexChanged( int index );

  protected:
    QgsFontMarkerSymbolLayer* mLayer = nullptr;
    CharacterWidget* widgetChar = nullptr;

  private slots:

    void penJoinStyleChanged();
    void updateAssistantSymbol();

  private:

    std::shared_ptr< QgsMarkerSymbol > mAssistantPreviewSymbol;

};

//////////


#include "ui_widget_centroidfill.h"

class QgsCentroidFillSymbolLayer;

/** \ingroup gui
 * \class QgsCentroidFillSymbolLayerWidget
 */
class GUI_EXPORT QgsCentroidFillSymbolLayerWidget : public QgsSymbolLayerWidget, private Ui::WidgetCentroidFill
{
    Q_OBJECT

  public:
    QgsCentroidFillSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent = nullptr );

    static QgsSymbolLayerWidget* create( const QgsVectorLayer* vl ) { return new QgsCentroidFillSymbolLayerWidget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayer* layer ) override;
    virtual QgsSymbolLayer* symbolLayer() override;

  protected:
    QgsCentroidFillSymbolLayer* mLayer = nullptr;

  private slots:
    void on_mDrawInsideCheckBox_stateChanged( int state );
    void on_mDrawAllPartsCheckBox_stateChanged( int state );

};


#include "ui_qgsgeometrygeneratorwidgetbase.h"
#include "qgis_gui.h"

class QgsGeometryGeneratorSymbolLayer;

/** \ingroup gui
 * \class QgsGeometryGeneratorSymbolLayerWidget
 */
class GUI_EXPORT QgsGeometryGeneratorSymbolLayerWidget : public QgsSymbolLayerWidget, private Ui::GeometryGeneratorWidgetBase
{
    Q_OBJECT

  public:
    QgsGeometryGeneratorSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent = nullptr );

    /**
     * Will be registered as factory
     */
    static QgsSymbolLayerWidget* create( const QgsVectorLayer* vl ) { return new QgsGeometryGeneratorSymbolLayerWidget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayer* layer ) override;
    virtual QgsSymbolLayer* symbolLayer() override;

  private:
    QgsGeometryGeneratorSymbolLayer* mLayer = nullptr;

  private slots:
    void updateExpression( const QString& string );
    void updateSymbolType();
};

#endif
