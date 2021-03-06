/*
 *  Copyright (C) 2010 - 2015 Leonid Kostrykin
 *
 *  Chair of Medical Engineering (mediTEC)
 *  RWTH Aachen University
 *  Pauwelsstr. 20
 *  52074 Aachen
 *  Germany
 *
 */

#include <Carna/dicom/CarnaDICOM.h>
#if !CARNAQT_DISABLED

#include <Carna/dicom/Patient.h>
#include <Carna/dicom/Study.h>
#include <Carna/dicom/Series.h>
#include <Carna/dicom/SeriesView.h>
#include <Carna/dicom/ToggleSeriesPreview.h>
#include <Carna/dicom/FlowLayout.h>
#include <Carna/qt/ExpandableGroupBox.h>
#include <Carna/base/Log.h>
#include <QTimer>
#include <QLabel>
#include <QVBoxLayout>
#include <QScrollArea>

namespace Carna
{

namespace dicom
{

using Carna::qt::ExpandableGroupBox;



// ----------------------------------------------------------------------------------
// SeriesView
// ----------------------------------------------------------------------------------

SeriesView::SeriesView( QWidget* parent )
    : QFrame( parent )
    , rebuildRequired( false )
    , container( new QVBoxLayout() )
    , containerWidget( new QWidget() )
{
    this->setLayout( new QVBoxLayout() );
    this->layout()->setContentsMargins( 0, 0, 0, 0 );
    this->setMinimumWidth( 540 );
    this->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Expanding );

    const static QString BACKGROUND_COLOR = "#808080";

    containerWidget->setLayout( container );
    containerWidget->setStyleSheet( "QWidget{ background-color: " + BACKGROUND_COLOR + "; } QLabel{ color: #FFFFFF; }" );
    
    QScrollArea* const scrollable_container = new QScrollArea();
    scrollable_container->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    scrollable_container->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
    scrollable_container->setWidgetResizable( true );
    scrollable_container->setWidget( containerWidget );
    scrollable_container->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    scrollable_container->setLineWidth( 1 );
    scrollable_container->setStyleSheet( "QScrollArea{ background-color: " + BACKGROUND_COLOR + "; }" );

    this->layout()->addWidget( scrollable_container );
}


SeriesView::~SeriesView()
{
}


void SeriesView::addPatient( const Patient& patient )
{
    patients.push_back( &patient );
    scheduleRebuild();
}


void SeriesView::clear()
{
    selectedSeriesPreviews.clear();

 // ----------------------------------------------------------------------------------

    std::set< const Series* > selected_series = selectedSeries;
    selectedSeries.clear();

    for( auto series_itr = selected_series.begin(); series_itr != selected_series.end(); ++series_itr )
    {
        emit seriesUnselected( **series_itr );
    }

    emit selectionChanged();

    patients.clear();
    scheduleRebuild();
}


void SeriesView::scheduleRebuild()
{
    if( rebuildRequired == false )
    {
        rebuildRequired = true;
        QTimer::singleShot( 0, this, SLOT( rebuild() ) );
    }
}


void SeriesView::rebuild()
{
    if( !rebuildRequired )
    {
        return;
    }

    QSizePolicy headingSizePolicy( QSizePolicy::Ignored, QSizePolicy::Fixed );
    headingSizePolicy.setHeightForWidth( false );

 // ----------------------------------------------------------------------------------

    delete this->container;
    this->container = new QVBoxLayout();
    qDeleteAll( containerWidget->children() );
    
    for( auto patient_itr = patients.begin(); patient_itr != patients.end(); ++patient_itr )
    {
        const Patient& patient = **patient_itr;

     // create and add patient name

        QLabel* const heading = new QLabel( QString::fromStdString( patient.name ) );
        heading->setStyleSheet( "font-size: 14pt; font-weight: bold;" );
        heading->setSizePolicy( headingSizePolicy );
        container->addWidget( heading );

     // process studies

        for( auto study_itr = patient.studies().begin(); study_itr != patient.studies().end(); ++study_itr )
        {
            const Study& study = **study_itr;

         // create expendable group box for the series

            ExpandableGroupBox* const gbStudy = new ExpandableGroupBox( QString::fromStdString( study.name ), true );
            FlowLayout* const series = new FlowLayout();
            gbStudy->child()->setLayout( series );
            series->setContentsMargins( 0, 0, 0, 0 );
            container->addWidget( gbStudy );

         // process series

            for( auto series_itr = study.series().begin(); series_itr != study.series().end(); ++series_itr )
            {
                const Series& currentSeries = **series_itr;

                if( currentSeries.elements().size() < Series::MINIMUM_ELEMENTS_COUNT )
                {
                    base::Log::instance().record
                        ( base::Log::warning, "Skipping series \"" + currentSeries.name + "\" because it has too few images!" );
                    continue;
                }

                ToggleSeriesPreview* const preview = new ToggleSeriesPreview();
                preview->setSeries( currentSeries );
                series->addWidget( preview );

                connect
                    ( preview, SIGNAL(       toggled( Carna::dicom::ToggleSeriesPreview& ) )
                    ,    this,   SLOT( seriesToggled( Carna::dicom::ToggleSeriesPreview& ) ) );

                connect
                    ( preview, SIGNAL(       doubleClicked( Carna::dicom::ToggleSeriesPreview& ) )
                    ,    this,   SLOT( seriesDoubleClicked( Carna::dicom::ToggleSeriesPreview& ) ) );
            }
        }
    }

    container->addStretch( 1 );
    containerWidget->setLayout( this->container );

 // ----------------------------------------------------------------------------------

    rebuildRequired = false;
}


void SeriesView::seriesToggled( ToggleSeriesPreview& seriesPreview )
{
    if( !seriesPreview.hasSeries() )
    {
        return;
    }

    const Series& series = seriesPreview.series();
    if( seriesPreview.isToggled() )
    {
        selectedSeries.insert( &series );
        selectedSeriesPreviews.insert( &seriesPreview );
        emit seriesSelected( series );
    }
    else
    {
        selectedSeries.erase( &series );
        selectedSeriesPreviews.erase( &seriesPreview );
        emit seriesUnselected( series );
    }

    emit selectionChanged();
}


const std::set< const Series* >& SeriesView::getSelectedSeries() const
{
    return selectedSeries;
}


void SeriesView::seriesDoubleClicked( ToggleSeriesPreview& seriesPreview )
{
    const auto selectedSeriesPreviews = this->selectedSeriesPreviews;
    for( auto preview_itr = selectedSeriesPreviews.begin(); preview_itr != selectedSeriesPreviews.end(); ++preview_itr )
    {
        ToggleSeriesPreview* const preview = *preview_itr;
        if( preview != &seriesPreview )
        {
            preview->setToggled( false );
        }
    }
    seriesPreview.setToggled( true );
    emit seriesDoubleClicked( seriesPreview.series() );
}



}  // namespace Carna :: dicom

}  // namespace Carna

#endif // CARNAQT_DISABLED
