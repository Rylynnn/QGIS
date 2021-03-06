/***************************************************************************
    qgsrulebasedrenderer.h - Rule-based renderer (symbology-ng)
    ---------------------
    begin                : May 2010
    copyright            : (C) 2010 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRULEBASEDRENDERERV2_H
#define QGSRULEBASEDRENDERERV2_H

#include "qgis_core.h"
#include "qgsfields.h"
#include "qgsfeature.h"
#include "qgis.h"

#include "qgsrenderer.h"

class QgsExpression;

class QgsCategorizedSymbolRenderer;
class QgsGraduatedSymbolRenderer;

/** \ingroup core
When drawing a vector layer with rule-based renderer, it goes through
the rules and draws features with symbols from rules that match.
 */
class CORE_EXPORT QgsRuleBasedRenderer : public QgsFeatureRenderer
{
  public:
    // TODO: use QVarLengthArray instead of QList

    enum FeatureFlags { FeatIsSelected = 1, FeatDrawMarkers = 2 };

    // feature for rendering: QgsFeature and some flags
    struct FeatureToRender
    {
      FeatureToRender( QgsFeature& _f, int _flags )
          : feat( _f )
          , flags( _flags )
      {}
      QgsFeature feat;
      int flags; // selected and/or draw markers
    };

    // rendering job: a feature to be rendered with a particular symbol
    // (both f, symbol are _not_ owned by this class)
    struct RenderJob
    {
      RenderJob( FeatureToRender& _ftr, QgsSymbol* _s )
          : ftr( _ftr )
          , symbol( _s )
      {}
      FeatureToRender& ftr;
      QgsSymbol* symbol = nullptr;
    };

    // render level: a list of jobs to be drawn at particular level
    // (jobs are owned by this class)
    struct RenderLevel
    {
      explicit RenderLevel( int z ): zIndex( z ) {}
      ~RenderLevel() { Q_FOREACH ( RenderJob* j, jobs ) delete j; }
      int zIndex;
      QList<RenderJob*> jobs;

      RenderLevel& operator=( const RenderLevel& rh )
      {
        zIndex = rh.zIndex;
        qDeleteAll( jobs );
        jobs.clear();
        Q_FOREACH ( RenderJob* job, rh.jobs )
        {
          jobs << new RenderJob( *job );
        }
        return *this;
      }

      RenderLevel( const RenderLevel& other )
          : zIndex( other.zIndex )
      {
        Q_FOREACH ( RenderJob* job, other.jobs )
        {
          jobs << new RenderJob( *job );
        }
      }

    };

    // rendering queue: a list of rendering levels
    typedef QList<RenderLevel> RenderQueue;

    class Rule;
    typedef QList<Rule*> RuleList;

    /** \ingroup core
      This class keeps data about a rules for rule-based renderer.
      A rule consists of a symbol, filter expression and range of scales.
      If filter is empty, it matches all features.
      If scale range has both values zero, it matches all scales.
      If one of the min/max scale denominators is zero, there is no lower/upper bound for scales.
      A rule matches if both filter and scale range match.
     */
    class CORE_EXPORT Rule
    {
      public:
        //! The result of rendering a rule
        enum RenderResult
        {
          Filtered = 0, //!< The rule does not apply
          Inactive,     //!< The rule is inactive
          Rendered      //!< Something was rendered
        };

        //! Constructor takes ownership of the symbol
        Rule( QgsSymbol* symbol, int scaleMinDenom = 0, int scaleMaxDenom = 0, const QString& filterExp = QString(),
              const QString& label = QString(), const QString& description = QString(), bool elseRule = false );
        ~Rule();

        //! Rules cannot be copied
        Rule( const Rule& rh ) = delete;
        //! Rules cannot be copied
        Rule& operator=( const Rule& rh ) = delete;

        /**
         * Dump for debug purpose
         * @param indent How many characters to indent. Will increase by two with every of the recursive calls
         * @return A string representing this rule
         */
        QString dump( int indent = 0 ) const;

        /**
         * Return the attributes used to evaluate the expression of this rule
         * @return A set of attribute names
         */
        QSet<QString> usedAttributes( const QgsRenderContext& context ) const;

        /**
         * Returns true if this rule or one of its chilren needs the geometry to be applied.
         */
        bool needsGeometry() const;

        //! @note available in python bindings as symbol2
        QgsSymbolList symbols( const QgsRenderContext& context = QgsRenderContext() ) const;

        //! @note not available in python bindings
        QgsLegendSymbolList legendSymbolItems( double scaleDenominator = -1, const QString& rule = "" ) const;

        //! @note added in 2.6
        QgsLegendSymbolListV2 legendSymbolItemsV2( int currentLevel = -1 ) const;

        /**
         * Check if a given feature shall be rendered by this rule
         *
         * @param f         The feature to test
         * @param context   The context in which the rendering happens
         * @return          True if the feature shall be rendered
         */
        bool isFilterOK( QgsFeature& f, QgsRenderContext *context = nullptr ) const;

        /**
         * Check if this rule applies for a given scale
         * @param scale The scale to check. If set to 0, it will always return true.
         *
         * @return If the rule will be evaluated at this scale
         */
        bool isScaleOK( double scale ) const;

        QgsSymbol* symbol() { return mSymbol; }
        QString label() const { return mLabel; }
        bool dependsOnScale() const { return mScaleMinDenom != 0 || mScaleMaxDenom != 0; }
        int scaleMinDenom() const { return mScaleMinDenom; }
        int scaleMaxDenom() const { return mScaleMaxDenom; }

        /**
         * A filter that will check if this rule applies
         * @return An expression
         */
        QgsExpression* filter() const { return mFilter; }

        /**
         * A filter that will check if this rule applies
         * @return An expression
         */
        QString filterExpression() const { return mFilterExp; }

        /**
         * A human readable description for this rule
         *
         * @return Description
         */
        QString description() const { return mDescription; }

        /**
         * Returns if this rule is active
         *
         * @return True if the rule is active
         */
        bool active() const { return mIsActive; }

        //! Unique rule identifier (for identification of rule within renderer)
        //! @note added in 2.6
        QString ruleKey() const { return mRuleKey; }
        //! Override the assigned rule key (should be used just internally by rule-based renderer)
        //! @note added in 2.6
        void setRuleKey( const QString& key ) { mRuleKey = key; }

        //! set a new symbol (or NULL). Deletes old symbol.
        void setSymbol( QgsSymbol* sym );
        void setLabel( const QString& label ) { mLabel = label; }

        /**
         * Set the minimum denominator for which this rule shall apply.
         * E.g. 1000 if it shall be evaluated between 1:1000 and 1:100'000
         * Set to 0 to disable the minimum check
         * @param scaleMinDenom The minimum scale denominator for this rule
         */
        void setScaleMinDenom( int scaleMinDenom ) { mScaleMinDenom = scaleMinDenom; }

        /**
         * Set the maximum denominator for which this rule shall apply.
         * E.g. 100'000 if it shall be evaluated between 1:1000 and 1:100'000
         * Set to 0 to disable the maximum check
         * @param scaleMaxDenom maximum scale denominator for this rule
         */
        void setScaleMaxDenom( int scaleMaxDenom ) { mScaleMaxDenom = scaleMaxDenom; }

        /**
         * Set the expression used to check if a given feature shall be rendered with this rule
         *
         * @param filterExp An expression
         */
        void setFilterExpression( const QString& filterExp );

        /**
         * Set a human readable description for this rule
         *
         * @param description Description
         */
        void setDescription( const QString& description ) { mDescription = description; }

        /**
         * Sets if this rule is active
         * @param state Determines if the rule should be activated or deactivated
         */
        void setActive( bool state ) { mIsActive = state; }

        //! clone this rule, return new instance
        Rule* clone() const;

        void toSld( QDomDocument& doc, QDomElement &element, QgsStringMap props ) const;

        /**
         * Create a rule from the SLD provided in element and for the specified geometry type.
         */
        static Rule* createFromSld( QDomElement& element, QgsWkbTypes::GeometryType geomType );

        QDomElement save( QDomDocument& doc, QgsSymbolMap& symbolMap ) const;

        //! prepare the rule for rendering and its children (build active children array)
        bool startRender( QgsRenderContext& context, const QgsFields& fields, QString& filter );

        //! get all used z-levels from this rule and children
        QSet<int> collectZLevels();

        //! assign normalized z-levels [0..N-1] for this rule's symbol for quick access during rendering
        //! @note not available in python bindings
        void setNormZLevels( const QMap<int, int>& zLevelsToNormLevels );

        /**
         * Render a given feature, will recursively call subclasses and only render if the constraints apply.
         *
         * @param featToRender The feature to render
         * @param context      The rendering context
         * @param renderQueue  The rendering queue to which the feature should be added
         * @return             The result of the rendering. In explicit if the feature is added to the queue or
         *                     the reason for not rendering the feature.
         */
        RenderResult renderFeature( FeatureToRender& featToRender, QgsRenderContext& context, RenderQueue& renderQueue );

        //! only tell whether a feature will be rendered without actually rendering it
        bool willRenderFeature( QgsFeature& feat, QgsRenderContext* context = nullptr );

        //! tell which symbols will be used to render the feature
        QgsSymbolList symbolsForFeature( QgsFeature& feat, QgsRenderContext* context = nullptr );

        /** Returns which legend keys match the feature
         * @note added in QGIS 2.14
         */
        QSet< QString > legendKeysForFeature( QgsFeature& feat, QgsRenderContext* context = nullptr );

        //! tell which rules will be used to render the feature
        RuleList rulesForFeature( QgsFeature& feat, QgsRenderContext* context = nullptr );

        /**
         * Stop a rendering process. Used to clean up the internal state of this rule
         *
         * @param context The rendering context
         */
        void stopRender( QgsRenderContext& context );

        /**
         * Create a rule from an XML definition
         *
         * @param ruleElem  The XML rule element
         * @param symbolMap Symbol map
         *
         * @return A new rule
         */
        static Rule* create( QDomElement& ruleElem, QgsSymbolMap& symbolMap );

        /**
         * Return all children rules of this rule
         *
         * @return A list of rules
         */
        RuleList& children() { return mChildren; }

        /**
         * Returns all children, grand-children, grand-grand-children, grand-gra... you get it
         *
         * @return A list of descendant rules
         */
        RuleList descendants() const { RuleList l; Q_FOREACH ( Rule *c, mChildren ) { l += c; l += c->descendants(); } return l; }

        /**
         * The parent rule
         *
         * @return Parent rule
         */
        Rule* parent() { return mParent; }

        //! add child rule, take ownership, sets this as parent
        void appendChild( Rule* rule );

        //! add child rule, take ownership, sets this as parent
        void insertChild( int i, Rule* rule );

        //! delete child rule
        void removeChild( Rule* rule );

        //! delete child rule
        void removeChildAt( int i );

        //! take child rule out, set parent as null
        Rule* takeChild( Rule* rule );

        //! take child rule out, set parent as null
        Rule* takeChildAt( int i );

        //! Try to find a rule given its unique key
        //! @note added in 2.6
        Rule* findRuleByKey( const QString& key );

        /**
         * Sets if this rule is an ELSE rule
         *
         * @param iselse If true, this rule is an ELSE rule
         */
        void setIsElse( bool iselse );

        /**
         * Check if this rule is an ELSE rule
         *
         * @return True if this rule is an else rule
         */
        bool isElse() { return mElseRule; }

      protected:
        void initFilter();

        Rule* mParent; // parent rule (NULL only for root rule)
        QgsSymbol* mSymbol = nullptr;
        int mScaleMinDenom, mScaleMaxDenom;
        QString mFilterExp, mLabel, mDescription;
        bool mElseRule;
        RuleList mChildren;
        RuleList mElseRules;
        bool mIsActive; // whether it is enabled or not

        QString mRuleKey; // string used for unique identification of rule within renderer

        // temporary
        QgsExpression* mFilter = nullptr;
        // temporary while rendering
        QSet<int> mSymbolNormZLevels;
        RuleList mActiveChildren;

      private:

        /**
         * Check which child rules are else rules and update the internal list of else rules
         *
         */
        void updateElseRules();
    };

    /////

    static QgsFeatureRenderer* create( QDomElement& element );

    //! Constructs the renderer from given tree of rules (takes ownership)
    QgsRuleBasedRenderer( QgsRuleBasedRenderer::Rule* root );
    //! Constructor for convenience. Creates a root rule and adds a default rule with symbol (takes ownership)
    QgsRuleBasedRenderer( QgsSymbol* defaultSymbol );

    ~QgsRuleBasedRenderer();

    //! return symbol for current feature. Should not be used individually: there could be more symbols for a feature
    virtual QgsSymbol* symbolForFeature( QgsFeature& feature, QgsRenderContext& context ) override;

    virtual bool renderFeature( QgsFeature& feature, QgsRenderContext& context, int layer = -1, bool selected = false, bool drawVertexMarker = false ) override;

    virtual void startRender( QgsRenderContext& context, const QgsFields& fields ) override;

    virtual void stopRender( QgsRenderContext& context ) override;

    virtual QString filter( const QgsFields& fields = QgsFields() ) override;

    virtual QSet<QString> usedAttributes( const QgsRenderContext& context ) const override;

    virtual bool filterNeedsGeometry() const override;

    virtual QgsRuleBasedRenderer* clone() const override;

    virtual void toSld( QDomDocument& doc, QDomElement &element, const QgsStringMap& props = QgsStringMap() ) const override;

    static QgsFeatureRenderer* createFromSld( QDomElement& element, QgsWkbTypes::GeometryType geomType );

    virtual QgsSymbolList symbols( QgsRenderContext& context ) override;

    virtual QDomElement save( QDomDocument& doc ) override;
    virtual QgsLegendSymbologyList legendSymbologyItems( QSize iconSize ) override;
    virtual bool legendSymbolItemsCheckable() const override;
    virtual bool legendSymbolItemChecked( const QString& key ) override;
    virtual void checkLegendSymbolItem( const QString& key, bool state = true ) override;

    virtual void setLegendSymbolItem( const QString& key, QgsSymbol* symbol ) override;
    virtual QgsLegendSymbolList legendSymbolItems( double scaleDenominator = -1, const QString& rule = "" ) override;
    virtual QgsLegendSymbolListV2 legendSymbolItemsV2() const override;
    virtual QString dump() const override;
    virtual bool willRenderFeature( QgsFeature& feat, QgsRenderContext& context ) override;
    virtual QgsSymbolList symbolsForFeature( QgsFeature& feat, QgsRenderContext& context ) override;
    virtual QgsSymbolList originalSymbolsForFeature( QgsFeature& feat, QgsRenderContext& context ) override;
    virtual QSet<QString> legendKeysForFeature( QgsFeature& feature, QgsRenderContext& context ) override;
    virtual Capabilities capabilities() override { return MoreSymbolsPerFeature | Filter | ScaleDependent; }

    /////

    Rule* rootRule() { return mRootRule; }

    //////

    //! take a rule and create a list of new rules based on the categories from categorized symbol renderer
    static void refineRuleCategories( Rule* initialRule, QgsCategorizedSymbolRenderer* r );
    //! take a rule and create a list of new rules based on the ranges from graduated symbol renderer
    static void refineRuleRanges( Rule* initialRule, QgsGraduatedSymbolRenderer* r );
    //! take a rule and create a list of new rules with intervals of scales given by the passed scale denominators
    static void refineRuleScales( Rule* initialRule, QList<int> scales );

    //! creates a QgsRuleBasedRenderer from an existing renderer.
    //! @note added in 2.5
    //! @returns a new renderer if the conversion was possible, otherwise 0.
    static QgsRuleBasedRenderer* convertFromRenderer( const QgsFeatureRenderer *renderer );

    //! helper function to convert the size scale and rotation fields present in some other renderers to data defined symbology
    static void convertToDataDefinedSymbology( QgsSymbol* symbol, const QString& sizeScaleField, const QString& rotationField = QString() );

  protected:
    //! the root node with hierarchical list of rules
    Rule* mRootRule = nullptr;

    // temporary
    RenderQueue mRenderQueue;
    QList<FeatureToRender> mCurrentFeatures;

    QString mFilter;
};

#endif // QGSRULEBASEDRENDERERV2_H
