/***************************************************************************
                          kreportchartview.cpp
                             -------------------
    begin                : Sun Aug 14 2005
    copyright            : (C) 2004-2005 by Ace Jones
    email                : <ace.j@hotpop.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// ----------------------------------------------------------------------------
// QT Includes
#include <QMouseEvent>
#include <QLabel>
#include <QFrame>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kreportchartview.h"
//#include <KDChartDataRegion.h>
//Added by qt3to4:

using namespace reports;

KReportChartView::KReportChartView( QWidget* parent): KDChart::Widget(parent)
{
    // ********************************************************************
    // Set KMyMoney's Chart Parameter Defaults
    // ********************************************************************
//     this->setPaletteBackgroundColor( Qt::white );

//     KDChartParams* _params = new KDChartParams();
//     _params->setChartType( KDChartParams::Line );
//     _params->setAxisLabelStringParams( KDChartAxisParams::AxisPosBottom,&m_abscissaNames,0);
//     _params->setDataSubduedColors();

    /**
    // use line marker, but only circles.
    _params->setLineMarker( true );
    _params->setLineMarkerSize( QSize(8,8) );
    _params->setLineMarkerStyle( 0, KDChartParams::LineMarkerCircle );
    _params->setLineMarkerStyle( 1, KDChartParams::LineMarkerCircle );
    _params->setLineMarkerStyle( 2, KDChartParams::LineMarkerCircle );
    **/

    // initialize parameters
//     this->setParams(_params);

    // initialize data
//     KDChartTableData* _data = new KDChartTableData();
//     this->setData(_data);

    // ********************************************************************
    // Some Examplatory Chart Table Data
    // ********************************************************************

    /**
    // 1st series
    this->data()->setCell( 0, 0,    17.5   );
    this->data()->setCell( 0, 1,   125     );  // highest value
    this->data()->setCell( 0, 2,     6.67  );  // lowest value
    this->data()->setCell( 0, 3,    33.333 );
    this->data()->setCell( 0, 4,    30     );
    // 2nd series
    this->data()->setCell( 1, 0,    40     );
    this->data()->setCell( 1, 1,    40     );
    this->data()->setCell( 1, 2,    45.5   );
    this->data()->setCell( 1, 3,    45     );
    this->data()->setCell( 1, 4,    35     );
    // 3rd series
    this->data()->setCell( 2, 0,    25     );
    // missing value: setCell( 2, 1,   25 );
    this->data()->setCell( 2, 2,    30     );
    this->data()->setCell( 2, 3,    45     );
    this->data()->setCell( 2, 4,    40     );
    **/

    // ********************************************************************
    // Tooltip Setup
    // ********************************************************************
/*    label = new QLabel( this );
    label->hide();*/
    // mouse tracking on will force the mouseMoveEvent() method to be called from Qt
/*    label->setMouseTracking( true );
    label->setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
    label->setAlignment( Qt::AlignRight );
    label->setAutoResize( true );*/
}

/**
 * This function implements mouseMoveEvents
  */
void KReportChartView::mouseMoveEvent( QMouseEvent* event )
{
/*    QPoint translate, pos; // some movement helpers
    uint dataset;          // the current dataset (eg. category)
    double value;          // the value of the region
    double pivot_sum;      // the sum over all categories in the current pivot point

    // the data region in which the cursor was last time
    static uint previous;

    if ( !this->hasMouseTracking() )
       return ;

    // find the data region under the current mouse location
    KDChartDataRegion* current = 0;
    Q3PtrListIterator < KDChartDataRegion > it( *(this->dataRegions()) );
    while ( ( current = it.current() ) ) {
        ++it;
        if ( current->contains( event->pos() ) )
        {
            // we found the data region
            value = this->data()->cellVal(current->row, current->col).toDouble();
            if ( this->getAccountSeries() )
            {
              dataset = current->row;
              pivot_sum = value * 100.0 / this->data()->colSum(current->col);
            }
            else
            {
              dataset = current->col;
              pivot_sum = value * 100.0 / this->data()->rowSum(current->row);
            }

            // if we enter a new data region
            if ( !label->isVisible() || previous != dataset )
            {
                // set the tooltip text
                label->setText(QString("<h2>%1</h2><strong>%2</strong><br>(%3\%)")
                    .arg(this->params()->legendText( dataset ))
                    .arg(value, 0, 'f', 2)
                    .arg(pivot_sum, 0, 'f', 2)
                    );

                previous = dataset;
            }

            translate.setX( -10 - label->width());
            translate.setY( 20);

            // display the label near the cursor
            pos = event->pos() + translate;

            // but don't let the label move outside the visible area
            if( pos.x() < 0 )
                pos.setX(0);
            if( pos.y() < 0 )
                pos.setY(0);
            if( pos.x() + label->width() > this->width() )
                pos.setX( this->width() - label->width() );
            if( pos.y() + label->height() > this->height() )
                pos.setY( this->height() - label->height() );

            // now move the label
            label->move( pos );
            label->show();

            //
            // In a more abstract class, we would emit a dateMouseMove event:
            //emit this->dataMouseMove( event->pos(), current->row, current->col );

            return ;
        }
    }
    // mouse cursor not found in any data region
    label->hide();*/
}

void KReportChartView::setProperty(int row, int col, int id)
{
  //this->data()->cell(row, col).setPropertySet(id);
}
