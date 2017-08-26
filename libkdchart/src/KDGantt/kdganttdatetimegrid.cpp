/****************************************************************************
** Copyright (C) 2001-2016 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Chart library.
**
** Licensees holding valid commercial KD Chart licenses may use this file in
** accordance with the KD Chart Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.GPL.txt included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

#include "kdganttdatetimegrid.h"
#include "kdganttdatetimegrid_p.h"

#include "kdganttabstractrowcontroller.h"

#include <QApplication>
#include <QDateTime>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionHeader>
#include <QWidget>
#include <QString>
#include <QDebug>
#include <QList>

#include <cassert>

using namespace KDGantt;

QDebug operator<<( QDebug dbg, KDGantt::DateTimeScaleFormatter::Range range )
{
    switch ( range ) {
    case KDGantt::DateTimeScaleFormatter::Second: dbg << "KDGantt::DateTimeScaleFormatter::Second"; break;
    case KDGantt::DateTimeScaleFormatter::Minute: dbg << "KDGantt::DateTimeScaleFormatter::Minute"; break;
    case KDGantt::DateTimeScaleFormatter::Hour:   dbg << "KDGantt::DateTimeScaleFormatter::Hour"; break;
    case KDGantt::DateTimeScaleFormatter::Day:    dbg << "KDGantt::DateTimeScaleFormatter::Day"; break;
    case KDGantt::DateTimeScaleFormatter::Week:   dbg << "KDGantt::DateTimeScaleFormatter::Week"; break;
    case KDGantt::DateTimeScaleFormatter::Month:  dbg << "KDGantt::DateTimeScaleFormatter::Month"; break;
    case KDGantt::DateTimeScaleFormatter::Year:   dbg << "KDGantt::DateTimeScaleFormatter::Year"; break;
    }
    return dbg;
}


/*!\class KDGantt::DateTimeGrid
 * \ingroup KDGantt
 *
 * This implementation of AbstractGrid works with QDateTime
 * and shows days and week numbers in the header
 */

qreal DateTimeGrid::Private::dateTimeToChartX( const QDateTime& dt ) const
{
    assert( startDateTime.isValid() );
    qreal result = startDateTime.date().daysTo(dt.date())*24.*60.*60.;
    result += startDateTime.time().msecsTo(dt.time())/1000.;
    result *= dayWidth/( 24.*60.*60. );

    return result;
}

QDateTime DateTimeGrid::Private::chartXtoDateTime( qreal x ) const
{
    assert( startDateTime.isValid() );
    int days = static_cast<int>( x/dayWidth );
    qreal secs = x*( 24.*60.*60. )/dayWidth;
    QDateTime dt = startDateTime;
    QDateTime result = dt.addDays( days )
                       .addSecs( static_cast<int>(secs-(days*24.*60.*60.) ) )
                       .addMSecs( qRound( ( secs-static_cast<int>( secs ) )*1000. ) );
    return result;
}

#define d d_func()

/*!\class KDGantt::DateTimeScaleFormatter
 * \ingroup KDGantt
 *
 * This class formats dates and times used in DateTimeGrid follawing a given format.
 *
 * The format follows the format of QDateTime::toString(), with one addition:
 * "w" is replaced with the week number of the date as number without a leading zero (1-53)
 * "ww" is replaced with the week number of the date as number with a leading zero (01-53)
 *
 * For example:
 *
 * \code
 *  // formatter to print the complete date over the current week
 *  // This leads to the first day of the week being printed
 *  DateTimeScaleFormatter formatter = DateTimeScaleFormatter( DateTimeScaleFormatter::Week, "yyyy-MM-dd" );
 * \endcode
 *
 * Optionally, you can set an user defined text alignment flag. The default value is Qt::AlignCenter.
 * \sa DateTimeScaleFormatter::DateTimeScaleFormatter
 *
 * This class even controls the range of the grid sections.
 * \sa KDGanttDateTimeScaleFormatter::Range
 */

/*! Creates a DateTimeScaleFormatter using \a range and \a format.
 *  The text on the header is aligned following \a alignment.
 */
DateTimeScaleFormatter::DateTimeScaleFormatter( Range range, const QString& format,
                                                const QString& templ, Qt::Alignment alignment )
    : _d( new Private( range, format, templ, alignment ) )
{
}

DateTimeScaleFormatter::DateTimeScaleFormatter( Range range, const QString& format, Qt::Alignment alignment )
    : _d( new Private( range, format, QString::fromLatin1( "%1" ), alignment ) )
{
}

DateTimeScaleFormatter::DateTimeScaleFormatter( const DateTimeScaleFormatter& other )
    : _d( new Private( other.range(), other.format(), other.d->templ, other.alignment() ) )
{
}

DateTimeScaleFormatter::~DateTimeScaleFormatter()
{
    delete _d;
}

DateTimeScaleFormatter& DateTimeScaleFormatter::operator=( const DateTimeScaleFormatter& other )
{
    if ( this == &other )
        return *this;

    delete _d;
    _d = new Private( other.range(), other.format(), other.d->templ, other.alignment() );
    return *this;
}

/*! \returns The format being used for formatting dates and times.
 */
QString DateTimeScaleFormatter::format() const
{
    return d->format;
}

/*! \returns The \a datetime as string respecting the format.
 */
QString DateTimeScaleFormatter::format( const QDateTime& datetime ) const
{
    QString result = d->format;
    // additional feature: Weeknumber
    const QString shortWeekNumber = QString::number( datetime.date().weekNumber()) + QLatin1String("/")
                                                                                     + QString::number( datetime.date().year());
    const QString longWeekNumber = ( shortWeekNumber.length() == 1 ? QString::fromLatin1( "0" ) : QString() ) + shortWeekNumber;
    result.replace( QString::fromLatin1( "ww" ), longWeekNumber );
    result.replace( QString::fromLatin1( "w" ), shortWeekNumber );
    result = datetime.toLocalTime().toString( result );
    return result;
}

QString DateTimeScaleFormatter::text( const QDateTime& datetime ) const
{
    return d->templ.arg( format( datetime ) );
}

/*! \returns The range of each item on a DateTimeGrid header.
 * \sa DateTimeScaleFormatter::Range */
DateTimeScaleFormatter::Range DateTimeScaleFormatter::range() const
{
    return d->range;
}

Qt::Alignment DateTimeScaleFormatter::alignment() const
{
    return d->alignment;
}

/*! \returns the QDateTime being the begin of the range after the one containing \a datetime
 *  \sa currentRangeBegin
 */
QDateTime DateTimeScaleFormatter::nextRangeBegin( const QDateTime& datetime ) const
{
    QDateTime result = datetime;
    switch ( d->range )
    {
    case Second:
        result = result.addSecs( 60 );
        break;
    case Minute:
        // set it to the begin of the next minute
        result.setTime( QTime( result.time().hour(), result.time().minute() ) );
        result = result.addSecs( 60 );
        break;
    case Hour:
        // set it to the begin of the next hour
        result.setTime( QTime( result.time().hour(), 0 ) );
        result = result.addSecs( 60 * 60 );
        break;
    case Day:
        // set it to midnight the next day
        result.setTime( QTime( 0, 0 ) );
        result = result.addDays( 1 );
        break;
    case Week:
        // set it to midnight
        result.setTime( QTime( 0, 0 ) );
        // iterate day-wise, until weekNumber changes
        {
            const int weekNumber = result.date().weekNumber();
            while ( weekNumber == result.date().weekNumber() )
                result = result.addDays( 1 );
        }
        break;
    case Month:
        // set it to midnight
        result.setTime( QTime( 0, 0 ) );
        // set it to the first of the next month
        result.setDate( QDate( result.date().year(), result.date().month(), 1 ).addMonths( 1 ) );
        break;
    case Year:
        // set it to midnight
        result.setTime( QTime( 0, 0 ) );
        // set it to the first of the next year
        result.setDate( QDate( result.date().year(), 1, 1 ).addYears( 1 ) );
        break;
    }
    //result = result.toLocalTime();
    assert( result != datetime );
    //qDebug() << "DateTimeScaleFormatter::nextRangeBegin("<<datetime<<")="<<d->range<<result;
    return result;
}

/*! \returns the QDateTime being the begin of the range containing \a datetime
 *  \sa nextRangeBegin
 */
QDateTime DateTimeScaleFormatter::currentRangeBegin( const QDateTime& datetime ) const
{
    QDateTime result = datetime;
    switch ( d->range )
    {
    case Second:
        break; // nothing
    case Minute:
        // set it to the begin of the current minute
        result.setTime( QTime( result.time().hour(), result.time().minute() ) );
        break;
    case Hour:
        // set it to the begin of the current hour
        result.setTime( QTime( result.time().hour(), 0 ) );
        break;
    case Day:
        // set it to midnight the current day
        result.setTime( QTime( 0, 0 ) );
        break;
    case Week:
        // set it to midnight
        result.setTime( QTime( 0, 0 ) );
        // iterate day-wise, as long weekNumber is the same
        {
            const int weekNumber = result.date().weekNumber();
            while ( weekNumber == result.date().addDays( -1 ).weekNumber() )
                result = result.addDays( -1 );
        }
        break;
    case Month:
        // set it to midnight
        result.setTime( QTime( 0, 0 ) );
        // set it to the first of the current month
        result.setDate( QDate( result.date().year(), result.date().month(), 1 ) );
        break;
    case Year:
        // set it to midnight
        result.setTime( QTime( 0, 0 ) );
        // set it to the first of the current year
        result.setDate( QDate( result.date().year(), 1, 1 ) );
        break;
    }
    return result;
}

DateTimeGrid::DateTimeGrid() : AbstractGrid( new Private )
{
}

DateTimeGrid::~DateTimeGrid()
{
}

/*! \returns The QDateTime used as start date for the grid.
 *
 * The default is three days before the current date.
 */
QDateTime DateTimeGrid::startDateTime() const
{
    return d->startDateTime;
}

/*! \param dt The start date of the grid. It is used as the beginning of the
 * horizontal scrollbar in the view.
 *
 * Emits gridChanged() after the start date has changed.
 */
void DateTimeGrid::setStartDateTime( const QDateTime& dt )
{
    d->startDateTime = dt;
    emit gridChanged();
}

/*! \returns The width in pixels for each day in the grid.
 *
 * The default is 100 pixels.
 */
qreal DateTimeGrid::dayWidth() const
{
    return d->dayWidth;
}

/*! Maps a given point in time \a dt to an X value in the scene.
 */
qreal DateTimeGrid::mapFromDateTime( const QDateTime& dt) const
{
    return d->dateTimeToChartX( dt );
}

/*! Maps a given X value \a x in scene coordinates to a point in time.
 */
QDateTime DateTimeGrid::mapToDateTime( qreal x ) const
{
    return d->chartXtoDateTime( x );
}

/*! \param w The width in pixels for each day in the grid.
 *
 * The signal gridChanged() is emitted after the day width is changed.
 */
void DateTimeGrid::setDayWidth( qreal w )
{
    assert( w>0 );
    d->dayWidth = w;
    emit gridChanged();
}

/*! \param s The scale to be used to paint the grid.
 *
 * The signal gridChanged() is emitted after the scale has changed.
 * \sa Scale
 *
 * Following example demonstrates how to change the format of the header to use
 * a date-scaling with the header-label displayed with the ISO date-notation.
 * \code
 * DateTimeScaleFormatter* formatter = new DateTimeScaleFormatter(DateTimeScaleFormatter::Day, QString::fromLatin1("yyyy-MMMM-dddd"));
 * grid->setUserDefinedUpperScale( formatter );
 * grid->setUserDefinedLowerScale( formatter );
 * grid->setScale( DateTimeGrid::ScaleUserDefined );
 * \endcode
 */
void DateTimeGrid::setScale( Scale s )
{
    d->scale = s;
    emit gridChanged();
}

/*! \returns The scale used to paint the grid.
 *
 * The default is ScaleAuto, which means the day scale will be used
 * as long as the day width is less or equal to 500.
 * \sa Scale
 */
DateTimeGrid::Scale DateTimeGrid::scale() const
{
    return d->scale;
}

/*! Sets the scale formatter for the lower part of the header to the
 *  user defined formatter to \a lower. The DateTimeGrid object takes
 *  ownership of the formatter, which has to be allocated with new.
 *
 * You have to set the scale to ScaleUserDefined for this setting to take effect.
 * \sa DateTimeScaleFormatter
 */
void DateTimeGrid::setUserDefinedLowerScale( DateTimeScaleFormatter* lower )
{
    delete d->lower;
    d->lower = lower;
    emit gridChanged();
}

/*! Sets the scale formatter for the upper part of the header to the
 *  user defined formatter to \a upper. The DateTimeGrid object takes
 *  ownership of the formatter, which has to be allocated with new.
 *
 * You have to set the scale to ScaleUserDefined for this setting to take effect.
 * \sa DateTimeScaleFormatter
 */
void DateTimeGrid::setUserDefinedUpperScale( DateTimeScaleFormatter* upper )
{
    delete d->upper;
    d->upper = upper;
    emit gridChanged();
}

/*! \return The DateTimeScaleFormatter being used to render the lower scale.
 */
DateTimeScaleFormatter* DateTimeGrid::userDefinedLowerScale() const
{
    return d->lower;
}

/*! \return The DateTimeScaleFormatter being used to render the upper scale.
 */
DateTimeScaleFormatter* DateTimeGrid::userDefinedUpperScale() const
{
    return d->upper;
}

/*! \param ws The start day of the week.
 *
 * A solid line is drawn on the grid to mark the beginning of a new week.
 * Emits gridChanged() after the start day has changed.
 */
void DateTimeGrid::setWeekStart( Qt::DayOfWeek ws )
{
    d->weekStart = ws;
    emit gridChanged();
}

/*! \returns The start day of the week */
Qt::DayOfWeek DateTimeGrid::weekStart() const
{
    return d->weekStart;
}

/*! \param fd A set of days to mark as free in the grid.
 *
 * Free days are filled with the alternate base brush of the
 * palette used by the view.
 * The signal gridChanged() is emitted after the free days are changed.
 */
void DateTimeGrid::setFreeDays( const QSet<Qt::DayOfWeek>& fd )
{
    d->freeDays = fd;
    emit gridChanged();
}

/*! \returns The days marked as free in the grid. */
QSet<Qt::DayOfWeek> DateTimeGrid::freeDays() const
{
    return d->freeDays;
}

/*! Sets the brush to use to paint free days.
*/
void DateTimeGrid::setFreeDaysBrush(const QBrush brush)
{
    d->freeDaysBrush = brush;
}

/*!
  \returns The brush used to paint free days.
*/
QBrush DateTimeGrid::freeDaysBrush() const
{
    return d->freeDaysBrush;
}

/*! \returns true if row separators are used. */
bool DateTimeGrid::rowSeparators() const
{
    return d->rowSeparators;
}
/*! \param enable Whether to use row separators or not. */
void DateTimeGrid::setRowSeparators( bool enable )
{
    d->rowSeparators = enable;
}

/*! Sets the brush used to display rows where no data is found.
 * Default is a red pattern. If set to QBrush() rows with no
 * information will not be marked.
 */
void DateTimeGrid::setNoInformationBrush( const QBrush& brush )
{
    d->noInformationBrush = brush;
    emit gridChanged();
}

/*! \returns the brush used to mark rows with no information.
 */
QBrush DateTimeGrid::noInformationBrush() const
{
    return d->noInformationBrush;
}

/*!
 * \param value The datetime to get the x value for.
 * \returns The x value corresponding to \a value or -1.0 if \a value is not a datetime variant.
 */
qreal DateTimeGrid::mapToChart( const QVariant& value ) const
{
    if ( ! value.canConvert( QVariant::DateTime ) ||
         ( value.type() == QVariant::String && value.toString().isEmpty() ) )
    {
        return -1.0;
    }
    return d->dateTimeToChartX( value.toDateTime() );
}

/*!
 * \param x The x value get the datetime for.
 * \returns The datetime corresponding to \a x or an invalid datetime if x cannot be mapped.
 */
QVariant DateTimeGrid::mapFromChart( qreal x ) const
{
    return d->chartXtoDateTime( x );
}

/*! \param idx The index to get the Span for.
 * \returns The start and end pixels, in a Span, of the specified index.
 */
Span DateTimeGrid::mapToChart( const QModelIndex& idx ) const
{
    assert( model() );
    if ( !idx.isValid() ) return Span();
    assert( idx.model()==model() );
    const QVariant sv = model()->data( idx, StartTimeRole );
    const QVariant ev = model()->data( idx, EndTimeRole );
    if ( sv.canConvert( QVariant::DateTime ) &&
        ev.canConvert( QVariant::DateTime ) &&
    !(sv.type() == QVariant::String && sv.toString().isEmpty()) &&
    !(ev.type() == QVariant::String && ev.toString().isEmpty())
    ) {
      QDateTime st = sv.toDateTime();
      QDateTime et = ev.toDateTime();
      if ( et.isValid() && st.isValid() ) {
        qreal sx = d->dateTimeToChartX( st );
        qreal ex = d->dateTimeToChartX( et )-sx;
        //qDebug() << "DateTimeGrid::mapToChart("<<st<<et<<") => "<< Span( sx, ex );
        return Span( sx, ex);
      }
    }
    // Special case for Events with only a start date
    if ( sv.canConvert( QVariant::DateTime ) && !(sv.type() == QVariant::String && sv.toString().isEmpty()) ) {
      QDateTime st = sv.toDateTime();
      if ( st.isValid() ) {
        qreal sx = d->dateTimeToChartX( st );
        return Span( sx, 0 );
      }
    }
    return Span();
}

#if 0
static void debug_print_idx( const QModelIndex& idx )
{
    if ( !idx.isValid() ) {
        qDebug() << "[Invalid]";
        return;
    }
    QDateTime st = idx.data( StartTimeRole ).toDateTime();
    QDateTime et = idx.data( EndTimeRole ).toDateTime();
    qDebug() << idx << "["<<st<<et<<"]";
}
#endif

/*! Maps the supplied Span to QDateTimes, and puts them as start time and
 * end time for the supplied index.
 *
 * \param span The span used to map from.
 * \param idx The index used for setting the start time and end time in the model.
 * \param constraints A list of hard constraints to match against the start time and
 * end time mapped from the span.
 *
 * \returns true if the start time and time was successfully added to the model, or false
 * if unsucessful.
 * Also returns false if any of the constraints isn't satisfied. That is, if the start time of
 * the constrained index is before the end time of the dependency index, or the end time of the
 * constrained index is before the start time of the dependency index.
 */
bool DateTimeGrid::mapFromChart( const Span& span, const QModelIndex& idx,
    const QList<Constraint>& constraints ) const
{
    assert( model() );
    if ( !idx.isValid() ) return false;
    assert( idx.model()==model() );

    QDateTime st = d->chartXtoDateTime(span.start());
    QDateTime et = d->chartXtoDateTime(span.start()+span.length());
    //qDebug() << "DateTimeGrid::mapFromChart("<<span<<") => "<< st << et;
    Q_FOREACH( const Constraint& c, constraints ) {
        if ( c.type() != Constraint::TypeHard || !isSatisfiedConstraint( c )) continue;
        if ( c.startIndex() == idx ) {
            QDateTime tmpst = model()->data( c.endIndex(), StartTimeRole ).toDateTime();
            //qDebug() << tmpst << "<" << et <<"?";
            if ( tmpst<et ) return false;
        } else if ( c.endIndex() == idx ) {
            QDateTime tmpet = model()->data( c.startIndex(), EndTimeRole ).toDateTime();
            //qDebug() << tmpet << ">" << st <<"?";
            if ( tmpet>st ) return false;
        }
    }

    return model()->setData( idx, qVariantFromValue(st), StartTimeRole )
        && model()->setData( idx, qVariantFromValue(et), EndTimeRole );
}

Qt::PenStyle DateTimeGrid::Private::gridLinePenStyle( QDateTime dt, Private::HeaderType headerType ) const
{
    switch ( headerType ) {
        case Private::HeaderHour:
            // Midnight
            if ( dt.time().hour() == 0 )
                return Qt::SolidLine;
            return Qt::DashLine;
        case Private::HeaderDay:
            // First day of the week
            if ( dt.date().dayOfWeek() == weekStart )
                return Qt::SolidLine;
            return Qt::DashLine;
        case Private::HeaderWeek:
            // First day of the month
            if ( dt.date().day() == 1 )
                return Qt::SolidLine;
            // First day of the week
            if ( dt.date().dayOfWeek() == weekStart )
                return Qt::DashLine;
            return Qt::NoPen;
        case Private::HeaderMonth:
            // First day of the year
            if ( dt.date().dayOfYear() == 1 )
                return Qt::SolidLine;
            // First day of the month
            if ( dt.date().day() == 1 )
                return Qt::DashLine;
            return Qt::NoPen;
        default:
            // Nothing to do here
            break;
   }

    // Default
    return Qt::NoPen;
}

QDateTime DateTimeGrid::Private::adjustDateTimeForHeader( QDateTime dt, Private::HeaderType headerType ) const
{
    // In any case, set time to 00:00:00:00
    dt.setTime( QTime( 0, 0, 0, 0 ) );

    switch ( headerType ) {
        case Private::HeaderWeek:
            // Set day to beginning of the week
            while ( dt.date().dayOfWeek() != weekStart )
                dt = dt.addDays( -1 );
            break;
        case Private::HeaderMonth:
            // Set day to beginning of the month
            dt = dt.addDays( 1 - dt.date().day() );
            break;
        case Private::HeaderYear:
            // Set day to first day of the year
            dt = dt.addDays( 1 - dt.date().dayOfYear() );
            break;
        default:
            // In any other case, we don't need to adjust the date time
            break;
    }

    return dt;
}

void DateTimeGrid::Private::paintVerticalLines( QPainter* painter,
                                                const QRectF& sceneRect,
                                                const QRectF& exposedRect,
                                                QWidget* widget,
                                                Private::HeaderType headerType )
{
        QDateTime dt = chartXtoDateTime( exposedRect.left() );
        dt = adjustDateTimeForHeader( dt, headerType );

        int offsetSeconds = 0;
        int offsetDays = 0;
        // Determine the time step per grid line
        if ( headerType == Private::HeaderHour )
            offsetSeconds = 60*60;
        else
            offsetDays = 1;

        for ( qreal x = dateTimeToChartX( dt ); x < exposedRect.right();
              dt = dt.addSecs( offsetSeconds ), dt = dt.addDays( offsetDays ), x = dateTimeToChartX( dt ) ) {
                  //TODO not the best solution as it might be one paint too much, but i don't know what
                  //causes the test to fail yet, i think it might be a rounding error
            //if ( x >= exposedRect.left() ) {
                QPen pen = painter->pen();
                pen.setBrush( QApplication::palette().dark() );
                pen.setStyle( gridLinePenStyle( dt, headerType ) );
                painter->setPen( pen );
                if ( freeDays.contains( static_cast<Qt::DayOfWeek>( dt.date().dayOfWeek() ) ) ) {
                    if (freeDaysBrush.style() == Qt::NoBrush)
                        painter->setBrush( widget?widget->palette().midlight()
                                           :QApplication::palette().midlight() );
                    else
                        painter->setBrush(freeDaysBrush);

                    painter->fillRect( QRectF( x, exposedRect.top(), dayWidth, exposedRect.height() ), painter->brush() );
                }
                painter->drawLine( QPointF( x, sceneRect.top() ), QPointF( x, sceneRect.bottom() ) );
            //}
        }
}

void DateTimeGrid::Private::paintVerticalUserDefinedLines( QPainter* painter,
                                                           const QRectF& sceneRect,
                                                           const QRectF& exposedRect,
                                                           const DateTimeScaleFormatter* formatter,
                                                           QWidget* widget )
{
    Q_UNUSED( widget );
    QDateTime dt = chartXtoDateTime( exposedRect.left() );
    dt = formatter->currentRangeBegin( dt );
    QPen pen = painter->pen();
    pen.setBrush( QApplication::palette().dark() );
    pen.setStyle( Qt::DashLine );
    painter->setPen( pen );
    for ( qreal x = dateTimeToChartX( dt ); x < exposedRect.right();
        dt = formatter->nextRangeBegin( dt ),x=dateTimeToChartX( dt ) ) {
        if ( freeDays.contains( static_cast<Qt::DayOfWeek>( dt.date().dayOfWeek() ) ) ) {
            QBrush oldBrush = painter->brush();
            if (freeDaysBrush.style() == Qt::NoBrush)
                painter->setBrush( widget?widget->palette().midlight()
                                 :QApplication::palette().midlight() );
            else
                painter->setBrush(freeDaysBrush);

          painter->fillRect( QRectF( x, exposedRect.top(), dayWidth, exposedRect.height() ), painter->brush() );
          painter->setBrush( oldBrush );
        }
              //TODO not the best solution as it might be one paint too much, but i don't know what
              //causes the test to fail yet, i think it might be a rounding error
        //if ( x >= exposedRect.left() ) {
            // FIXME: Also fill area between this and the next vertical line to indicate free days? (Johannes)
    painter->drawLine( QPointF( x, sceneRect.top() ), QPointF( x, sceneRect.bottom() ) );
        //}
    }
}

DateTimeGrid::Private::HeaderType DateTimeGrid::Private::headerTypeForScale( DateTimeGrid::Scale scale )
{
    switch ( scale ) {
        case ScaleHour:
            return Private::HeaderHour;
        case ScaleDay:
            return Private::HeaderDay;
        case ScaleWeek:
            return Private::HeaderWeek;
        case ScaleMonth:
            return Private::HeaderMonth;
        default:
            // There are no specific header types for any other scale!
            assert( false );
            break;
    }
        return Private::HeaderDay;
}

void DateTimeGrid::paintGrid( QPainter* painter,
                              const QRectF& sceneRect,
                              const QRectF& exposedRect,
                              AbstractRowController* rowController,
                              QWidget* widget )
{
    // TODO: Support hours and weeks
    switch ( scale() ) {
    case ScaleHour:
    case ScaleDay:
    case ScaleWeek:
    case ScaleMonth:
        d->paintVerticalLines( painter, sceneRect, exposedRect, widget, d->headerTypeForScale( scale() ) );
        break;
    case ScaleAuto: {
        const qreal tabw = QApplication::fontMetrics().width( QLatin1String( "XXXXX" ) );
        const qreal dayw = dayWidth();
        if ( dayw > 24*60*60*tabw ) {

            d->paintVerticalUserDefinedLines( painter, sceneRect, exposedRect, &d->minute_lower, widget );
        } else if ( dayw > 24*60*tabw ) {
            d->paintVerticalLines( painter, sceneRect, exposedRect, widget, Private::HeaderHour );
        } else if ( dayw > 24*tabw ) {
        d->paintVerticalLines( painter, sceneRect, exposedRect, widget, Private::HeaderDay );
        } else if ( dayw > tabw ) {
            d->paintVerticalUserDefinedLines( painter, sceneRect, exposedRect, &d->week_lower, widget );
        } else if ( 4*dayw > tabw ) {
            d->paintVerticalUserDefinedLines( painter, sceneRect, exposedRect, &d->month_lower, widget );
        } else {
            d->paintVerticalUserDefinedLines( painter, sceneRect, exposedRect, &d->year_lower, widget );
        }
        break;
    }
    case ScaleUserDefined:
        d->paintVerticalUserDefinedLines( painter, sceneRect, exposedRect, d->lower, widget );
        break;
    }
    if ( rowController ) {
        // First draw the rows
        QPen pen = painter->pen();
        pen.setBrush( QApplication::palette().dark() );
        pen.setStyle( Qt::DashLine );
        painter->setPen( pen );
        QModelIndex idx = rowController->indexAt( qRound( exposedRect.top() ) );
        if ( rowController->indexAbove( idx ).isValid() ) idx = rowController->indexAbove( idx );
        qreal y = 0;
        while ( y < exposedRect.bottom() && idx.isValid() ) {
            const Span s = rowController->rowGeometry( idx );
            y = s.start()+s.length();
            if ( d->rowSeparators ) {
                painter->drawLine( QPointF( sceneRect.left(), y ),
                                   QPointF( sceneRect.right(), y ) );
            }
            if ( !idx.data( ItemTypeRole ).isValid() && d->noInformationBrush.style() != Qt::NoBrush ) {
                painter->fillRect( QRectF( exposedRect.left(), s.start(), exposedRect.width(), s.length() ), d->noInformationBrush );
            }
            // Is alternating background better?
            //if ( idx.row()%2 ) painter->fillRect( QRectF( exposedRect.x(), s.start(), exposedRect.width(), s.length() ), QApplication::palette().alternateBase() );
            idx =  rowController->indexBelow( idx );
        }
    }
}

int DateTimeGrid::Private::tabHeight( const QString& txt, QWidget* widget ) const
{
    QStyleOptionHeader opt;
    if ( widget ) opt.initFrom( widget );
    opt.text = txt;
    QStyle* style;
    if ( widget ) style = widget->style();
    else style = QApplication::style();
    QSize s = style->sizeFromContents(QStyle::CT_HeaderSection, &opt, QSize(), widget);
    return s.height();
}

void DateTimeGrid::Private::getAutomaticFormatters( DateTimeScaleFormatter** lower, DateTimeScaleFormatter** upper)
{
    const qreal tabw = QApplication::fontMetrics().width( QLatin1String( "XXXXX" ) );
    const qreal dayw = dayWidth;
    if ( dayw > 24*60*60*tabw ) {
        *lower = &minute_lower;
        *upper = &minute_upper;
    } else if ( dayw > 24*60*tabw ) {
        *lower = &hour_lower;
        *upper = &hour_upper;
    } else if ( dayw > 24*tabw ) {
        *lower = &day_lower;
        *upper = &day_upper;
    } else if ( dayw > tabw ) {
        *lower = &week_lower;
        *upper = &week_upper;
    } else if ( 4*dayw > tabw ) {
        *lower = &month_lower;
        *upper = &month_upper;
    } else {
        *lower = &year_lower;
        *upper = &year_upper;
    }
}


void DateTimeGrid::paintHeader( QPainter* painter,  const QRectF& headerRect, const QRectF& exposedRect,
                                qreal offset, QWidget* widget )
{
    painter->save();
    QPainterPath clipPath;
    clipPath.addRect( headerRect );
    painter->setClipPath( clipPath, Qt::IntersectClip );
    switch ( scale() )
    {
    case ScaleHour:
        paintHourScaleHeader( painter, headerRect, exposedRect, offset, widget );
        break;
    case ScaleDay:
        paintDayScaleHeader( painter, headerRect, exposedRect, offset, widget );
        break;
    case ScaleWeek:
        paintWeekScaleHeader( painter, headerRect, exposedRect, offset, widget );
        break;
    case ScaleMonth:
        paintMonthScaleHeader( painter, headerRect, exposedRect, offset, widget );
        break;
    case ScaleAuto:
        {
            DateTimeScaleFormatter *lower, *upper;
            d->getAutomaticFormatters( &lower, &upper );
            const qreal lowerHeight = d->tabHeight( lower->text( startDateTime() ) );
            const qreal upperHeight = d->tabHeight( upper->text( startDateTime() ) );
            const qreal upperRatio = upperHeight/( lowerHeight+upperHeight );

            const QRectF upperHeaderRect( headerRect.x(), headerRect.top(), headerRect.width()-1, headerRect.height() * upperRatio );
            const QRectF lowerHeaderRect( headerRect.x(), upperHeaderRect.bottom()+1, headerRect.width()-1,  headerRect.height()-upperHeaderRect.height()-1 );

            paintUserDefinedHeader( painter, lowerHeaderRect, exposedRect, offset, lower, widget );
            paintUserDefinedHeader( painter, upperHeaderRect, exposedRect, offset, upper, widget );
            break;
        }
    case ScaleUserDefined:
        {
            const qreal lowerHeight = d->tabHeight( d->lower->text( startDateTime() ) );
            const qreal upperHeight = d->tabHeight( d->upper->text( startDateTime() ) );
            const qreal upperRatio = upperHeight/( lowerHeight+upperHeight );

            const QRectF upperHeaderRect( headerRect.x(), headerRect.top(), headerRect.width()-1, headerRect.height() * upperRatio );
            const QRectF lowerHeaderRect( headerRect.x(), upperHeaderRect.bottom()+1, headerRect.width()-1,  headerRect.height()-upperHeaderRect.height()-1 );

            paintUserDefinedHeader( painter, lowerHeaderRect, exposedRect, offset, d->lower, widget );
            paintUserDefinedHeader( painter, upperHeaderRect, exposedRect, offset, d->upper, widget );
        }
        break;
    }
    painter->restore();
}

void DateTimeGrid::paintUserDefinedHeader( QPainter* painter,
                                           const QRectF& headerRect, const QRectF& exposedRect,
                                           qreal offset, const DateTimeScaleFormatter* formatter,
                                           QWidget* widget )
{
    const QStyle* const style = widget ? widget->style() : QApplication::style();

    QDateTime dt = formatter->currentRangeBegin( d->chartXtoDateTime( offset + exposedRect.left() ));
    qreal x = d->dateTimeToChartX( dt );

    while ( x < exposedRect.right() + offset ) {
        const QDateTime next = formatter->nextRangeBegin( dt );
        const qreal nextx = d->dateTimeToChartX( next );

        QStyleOptionHeader opt;
        if ( widget ) opt.init( widget );
        opt.rect = QRectF( x - offset+1, headerRect.top(), qMax<qreal>( 1., nextx-x-1 ), headerRect.height() ).toAlignedRect();
        opt.textAlignment = formatter->alignment();
        opt.text = formatter->text( dt );
        style->drawControl( QStyle::CE_Header, &opt, painter, widget );

        dt = next;
        x = nextx;
    }
}

void DateTimeGrid::Private::paintHeader( QPainter* painter,
                                         const QRectF& headerRect, const QRectF& exposedRect,
                                         qreal offset, QWidget* widget,
                                         Private::HeaderType headerType,
                                         DateTextFormatter *formatter )
{
    QStyle* style = widget?widget->style():QApplication::style();

    const qreal left = exposedRect.left() + offset;
    const qreal right = exposedRect.right() + offset;

    // Paint a section for each hour
    QDateTime dt = chartXtoDateTime( left );
    dt = adjustDateTimeForHeader( dt, headerType );
    // Determine the time step per grid line
    int offsetSeconds = 0;
    int offsetDays = 0;
    int offsetMonths = 0;

    switch ( headerType ) {
        case Private::HeaderHour:
            offsetSeconds = 60*60;
            break;
        case Private::HeaderDay:
            offsetDays = 1;
            break;
        case Private::HeaderWeek:
            offsetDays = 7;
            break;
        case Private::HeaderMonth:
            offsetMonths = 1;
            break;
        case Private::HeaderYear:
            offsetMonths = 12;
            break;
        default:
            // Other scales cannot be painted with this method!
            assert( false );
            break;
    }

    for ( qreal x = dateTimeToChartX( dt ); x < right;
          dt = dt.addSecs( offsetSeconds ), dt = dt.addDays( offsetDays ), dt = dt.addMonths( offsetMonths ),
          x = dateTimeToChartX( dt ) ) {
        QStyleOptionHeader opt;
        if ( widget ) opt.init( widget );
        opt.rect = formatter->textRect( x, offset, dayWidth, headerRect, dt );
        opt.text = formatter->format( dt );
        opt.textAlignment = Qt::AlignCenter;
        style->drawControl(QStyle::CE_Header, &opt, painter, widget);
    }
}

/*! Paints the hour scale header.
 * \sa paintHeader()
 */
void DateTimeGrid::paintHourScaleHeader( QPainter* painter,
                                         const QRectF& headerRect, const QRectF& exposedRect,
                                         qreal offset, QWidget* widget )
{
    class HourFormatter : public Private::DateTextFormatter {
    public:
        virtual ~HourFormatter() {}

        QString format( const QDateTime& dt ) {
            return dt.time().toString( QString::fromLatin1( "hh" ) );
        }
        QRect textRect( qreal x, qreal offset, qreal dayWidth, const QRectF& headerRect, const QDateTime& dt ) {
            Q_UNUSED(dt);

            return QRectF( QPointF( x, headerRect.top() ) + QPointF( -offset + 1.0, headerRect.height() / 2.0 ),
                           QSizeF( dayWidth / 24.0, headerRect.height() / 2.0 ) ).toAlignedRect();
        }
    };
    d->paintHeader( painter, headerRect, exposedRect, offset, widget, // General parameters
                    Private::HeaderHour, new HourFormatter ); // Custom parameters

    class DayFormatter : public Private::DateTextFormatter {
    public:
        virtual ~DayFormatter() {}
        QString format( const QDateTime& dt ) {
            return dt.date().toString();
        }
        QRect textRect( qreal x, qreal offset, qreal dayWidth, const QRectF& headerRect, const QDateTime& dt ) {
            Q_UNUSED(dt);

            return QRectF( QPointF( x, headerRect.top() ) + QPointF( -offset, 0.0 ),
                           QSizeF( dayWidth, headerRect.height() / 2.0 ) ).toRect();
        }
    };
    d->paintHeader( painter, headerRect, exposedRect, offset, widget, // General parameters
                    Private::HeaderDay, new DayFormatter ); // Custom parameters
}

/*! Paints the day scale header.
 * \sa paintHeader()
 */
void DateTimeGrid::paintDayScaleHeader( QPainter* painter,  const QRectF& headerRect, const QRectF& exposedRect,
                                qreal offset, QWidget* widget )
{
    class DayFormatter : public Private::DateTextFormatter {
    public:
        virtual ~DayFormatter() {}

        QString format( const QDateTime& dt ) {
            return dt.toString( QString::fromLatin1( "ddd" ) ).left( 1 );
        }
        QRect textRect( qreal x, qreal offset, qreal dayWidth, const QRectF& headerRect, const QDateTime& dt ) {
            Q_UNUSED(dt);

            return QRectF( QPointF( x, headerRect.top() ) + QPointF( -offset + 1.0, headerRect.height() / 2.0 ),
                           QSizeF( dayWidth, headerRect.height() / 2.0 ) ).toAlignedRect();
        }
    };
    d->paintHeader( painter, headerRect, exposedRect, offset, widget, // General parameters
                    Private::HeaderDay, new DayFormatter ); // Custom parameters

    class WeekFormatter : public Private::DateTextFormatter {
    public:
        virtual ~WeekFormatter() {}
        QString format( const QDateTime& dt ) {
            return QString::number(dt.date().weekNumber()) + QLatin1String("/") + QString::number(dt.date().year());
        }
        QRect textRect( qreal x, qreal offset, qreal dayWidth, const QRectF& headerRect, const QDateTime& dt ) {
            Q_UNUSED(dt);

            return QRectF( QPointF( x, headerRect.top() ) + QPointF( -offset, 0.0 ),
                           QSizeF( dayWidth * 7, headerRect.height() / 2.0 ) ).toRect();
        }
    };
    d->paintHeader( painter, headerRect, exposedRect, offset, widget, // General parameters
                    Private::HeaderWeek, new WeekFormatter ); // Custom parameters
}

/*! Paints the week scale header.
 * \sa paintHeader()
 */
void DateTimeGrid::paintWeekScaleHeader( QPainter* painter,  const QRectF& headerRect, const QRectF& exposedRect,
                                        qreal offset, QWidget* widget )
{
    class WeekFormatter : public Private::DateTextFormatter {
    public:
        virtual ~WeekFormatter() {}

        QString format( const QDateTime& dt ) {
            return QString::number( dt.date().weekNumber() );
        }
        QRect textRect( qreal x, qreal offset, qreal dayWidth, const QRectF& headerRect, const QDateTime& dt ) {
            Q_UNUSED(dt);

            return QRectF( QPointF( x, headerRect.top() ) + QPointF( -offset, headerRect.height() / 2.0 ),
                           QSizeF( dayWidth * 7, headerRect.height() / 2.0 ) ).toRect();
        }
    };
    d->paintHeader( painter, headerRect, exposedRect, offset, widget, // General parameters
                    Private::HeaderWeek, new WeekFormatter ); // Custom parameters

    class MonthFormatter : public Private::DateTextFormatter {
    public:
        virtual ~MonthFormatter() {}

        QString format( const QDateTime& dt ) {
            return QLocale().monthName(dt.date().month(), QLocale::LongFormat) + QLatin1String("/") + QString::number(dt.date().year());
        }
        QRect textRect( qreal x, qreal offset, qreal dayWidth, const QRectF& headerRect, const QDateTime& dt ) {
            return QRectF( QPointF( x, headerRect.top() ) + QPointF( -offset, 0.0 ),
                           QSizeF( dayWidth * dt.date().daysInMonth(), headerRect.height() / 2.0 ) ).toRect();
        }
    };
    d->paintHeader( painter, headerRect, exposedRect, offset, widget, // General parameters
                    Private::HeaderMonth, new MonthFormatter ); // Custom parameters
}

/*! Paints the week scale header.
 * \sa paintHeader()
 */
void DateTimeGrid::paintMonthScaleHeader( QPainter* painter,  const QRectF& headerRect, const QRectF& exposedRect,
                                        qreal offset, QWidget* widget )
{
    class MonthFormatter : public Private::DateTextFormatter {
    public:
        virtual ~MonthFormatter() {}

        QString format( const QDateTime& dt ) {
            return QLocale().monthName(dt.date().month(), QLocale::ShortFormat) + QLatin1String("/") + QString::number(dt.date().year());
        }
        QRect textRect( qreal x, qreal offset, qreal dayWidth, const QRectF& headerRect, const QDateTime& dt ) {
            return QRectF( QPointF( x, headerRect.top() ) + QPointF( -offset, headerRect.height() / 2.0 ),
                           QSizeF( dayWidth * dt.date().daysInMonth(), headerRect.height() / 2.0 ) ).toRect();
        }
    };
    d->paintHeader( painter, headerRect, exposedRect, offset, widget, // General parameters
                    Private::HeaderMonth, new MonthFormatter ); // Custom parameters

    class YearFormatter : public Private::DateTextFormatter {
    public:
        virtual ~YearFormatter() {}

        QString format( const QDateTime& dt ) {
            return QString::number( dt.date().year() );
        }
        QRect textRect( qreal x, qreal offset, qreal dayWidth, const QRectF& headerRect, const QDateTime& dt ) {
            return QRectF( QPointF( x, headerRect.top() ) + QPointF( -offset, 0.0 ),
                           QSizeF( dayWidth * dt.date().daysInYear(), headerRect.height() / 2.0 ) ).toRect();
        }
    };
    d->paintHeader( painter, headerRect, exposedRect, offset, widget, // General parameters
                    Private::HeaderYear, new YearFormatter ); // Custom parameters
}

/*! Draw the background for a day.
*/
void DateTimeGrid::drawDayBackground(QPainter* painter, const QRectF& rect, const QDate& date)
{
    Q_UNUSED(painter);
    Q_UNUSED(rect);
    Q_UNUSED(date);
}

/*! Draw the foreground for a day.
*/
void DateTimeGrid::drawDayForeground(QPainter* painter, const QRectF& rect, const QDate& date)
{
    Q_UNUSED(painter);
    Q_UNUSED(rect);
    Q_UNUSED(date);
}

/** Return the rectangle that represents the date-range.
*/
QRectF DateTimeGrid::computeRect(const QDateTime& from, const QDateTime& to, const QRectF& rect) const
{
    qreal topLeft = d->dateTimeToChartX(from);
    qreal topRight = d->dateTimeToChartX(to);

    return QRectF(topLeft, rect.top(), topRight - topLeft, rect.height());
}

/** Return a date-range represented by the rectangle.
*/
QPair<QDateTime, QDateTime> DateTimeGrid::dateTimeRange(const QRectF& rect) const
{
    QDateTime start;
    QDateTime end;

    start = d->chartXtoDateTime(rect.left());
    end = d->chartXtoDateTime(rect.right());

    return qMakePair(start, end);
}

void DateTimeGrid::drawBackground(QPainter* paint, const QRectF& rect)
{
    int offset = (int)dayWidth();

    // Figure out the date at the extreme left
    QDate date = d->chartXtoDateTime(rect.left()).date();

    // We need to paint from one end to the other
    int startx = rect.left();
    int endx = rect.right();

    // Save the painter state
    paint->save();

    // Paint the first date column
    while (1)
    {
        QDate nextDate = d->chartXtoDateTime(startx+1).date();
        if (date != nextDate)
        {
            QRectF dayRect(startx-dayWidth(), rect.top(), dayWidth(), rect.height());
            dayRect = dayRect.adjusted(1, 0, 0, 0);
            drawDayBackground(paint, dayRect, date);
            break;
        }

        ++startx;
    }

    // Paint the remaining dates
    for (int i=startx; i<endx; i+=offset)
    {
        date = d->chartXtoDateTime(i+1).date();

        QRectF dayRect(i, rect.top(), dayWidth(), rect.height());
        dayRect = dayRect.adjusted(1, 0, 0, 0);
        drawDayBackground(paint, dayRect, date);
    }

    // Restore the painter state
    paint->restore();
}

void DateTimeGrid::drawForeground(QPainter* paint, const QRectF& rect)
{
    int offset = (int)dayWidth();

    // Figure out the date at the extreme left
    QDate date = d->chartXtoDateTime(rect.left()).date();

    // We need to paint from one end to the other
    int startx = rect.left();
    int endx = rect.right();

    // Save the painter state
    paint->save();

    // Paint the first date column
    while (1)
    {
        QDate nextDate = d->chartXtoDateTime(startx+1).date();
        if (date != nextDate)
        {
            QRectF dayRect(startx-dayWidth(), rect.top(), dayWidth(), rect.height());
            dayRect = dayRect.adjusted(1, 0, 0, 0);
            drawDayForeground(paint, dayRect, date);
            break;
        }

        ++startx;
    }

    // Paint the remaining dates
    for (int i=startx; i<endx; i+=offset)
    {
        date = d->chartXtoDateTime(i+1).date();

        QRectF dayRect(i, rect.top(), dayWidth(), rect.height());
        dayRect = dayRect.adjusted(1, 0, 0, 0);
        drawDayForeground(paint, dayRect, date);
    }

    // Restore the painter state
    paint->restore();
}

#undef d

#ifndef KDAB_NO_UNIT_TESTS

#include <QStandardItemModel>
#include "unittest/test.h"

static std::ostream& operator<<( std::ostream& os, const QDateTime& dt )
{
#ifdef QT_NO_STL
    os << dt.toString().toLatin1().constData();
#else
    os << dt.toString().toStdString();
#endif
    return os;
}

KDAB_SCOPED_UNITTEST_SIMPLE( KDGantt, DateTimeGrid, "test" ) {
    QStandardItemModel model( 3, 2 );
    DateTimeGrid grid;
    QDateTime dt = QDateTime::currentDateTime();
    grid.setModel( &model );
    QDateTime startdt = dt.addDays( -10 );
    grid.setStartDateTime( startdt );

    model.setData( model.index( 0, 0 ), dt,               StartTimeRole );
    model.setData( model.index( 0, 0 ), dt.addDays( 17 ), EndTimeRole );

    model.setData( model.index( 2, 0 ), dt.addDays( 18 ), StartTimeRole );
    model.setData( model.index( 2, 0 ), dt.addDays( 19 ), EndTimeRole );

    Span s = grid.mapToChart( model.index( 0, 0 ) );
    //qDebug() << "span="<<s;

    assertTrue( s.start()>0 );
    assertTrue( s.length()>0 );

    assertTrue( startdt == grid.mapToDateTime( grid.mapFromDateTime( startdt ) ) );

    grid.mapFromChart( s, model.index( 1, 0 ) );

    QDateTime s1 = model.data( model.index( 0, 0 ), StartTimeRole ).toDateTime();
    QDateTime e1 = model.data( model.index( 0, 0 ), EndTimeRole ).toDateTime();
    QDateTime s2 = model.data( model.index( 1, 0 ), StartTimeRole ).toDateTime();
    QDateTime e2 = model.data( model.index( 1, 0 ), EndTimeRole ).toDateTime();

    assertTrue( s1.isValid() );
    assertTrue( e1.isValid() );
    assertTrue( s2.isValid() );
    assertTrue( e2.isValid() );

    assertEqual( s1, s2 );
    assertEqual( e1, e2 );

    assertTrue( grid.isSatisfiedConstraint( Constraint( model.index( 0, 0 ), model.index( 2, 0 ) ) ) );
    assertFalse( grid.isSatisfiedConstraint( Constraint( model.index( 2, 0 ), model.index( 0, 0 ) ) ) );

    s = grid.mapToChart( model.index( 0, 0 ) );
    s.setEnd( s.end()+100000. );
    bool rc = grid.mapFromChart( s, model.index( 0, 0 ) );
    assertTrue( rc );
    assertEqual( s1, model.data( model.index( 0, 0 ), StartTimeRole ).toDateTime() );
    Span newspan = grid.mapToChart( model.index( 0, 0 ) );
    assertEqual( newspan.start(), s.start() );
    assertEqual( newspan.length(), s.length() );

    {
        QDateTime startDateTime = QDateTime::currentDateTime();
        qreal dayWidth = 100;
        QDate currentDate = QDate::currentDate();
        QDateTime dt( QDate(currentDate.year(), 1, 1),  QTime( 0, 0, 0, 0 ) );
        assert( dt.isValid() );
        qreal result = startDateTime.date().daysTo(dt.date())*24.*60.*60.;
        result += startDateTime.time().msecsTo(dt.time())/1000.;
        result *= dayWidth/( 24.*60.*60. );

        int days = static_cast<int>( result/dayWidth );
        qreal secs = result*( 24.*60.*60. )/dayWidth;
        QDateTime dt2 = startDateTime;
        QDateTime result2 = dt2.addDays( days ).addSecs( static_cast<int>(secs-(days*24.*60.*60.) ) ).addMSecs( qRound( ( secs-static_cast<int>( secs ) )*1000. ) );

        assertEqual( dt, result2 );
    }
}

#endif /* KDAB_NO_UNIT_TESTS */

#include "moc_kdganttdatetimegrid.cpp"
