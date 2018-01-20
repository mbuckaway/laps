#include "cplot.h"


#include <QVector>


#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_picker.h>
#include <qwt_plot.h>
#include <qwt_scale_div.h>
#include <qwt_scale_draw.h>
#include <qwt_legend.h>


#include "cdbase.h"
#include "cplot.h"
#include "cdbase.h"



#define DEFAULT_WIDGETWIDTH     512

#define DEFAULT_CIRCLESIZE		12.
#define DEFAULT_DOTSIZE			10.
#define DEFAULT_CROSSSIZE		12.
#define DEFAULT_PENWIDTH		3.
#define SMALL_DOTSIZE           5.
#define SMALL_CROSSSIZE         8.
#define SMALL_PENWIDTH          3.





cplot::cplot(QString title, options_t options, QWidget *parent)
    : QwtPlot(parent)
{
    initializeDefaultWidget(options);
    setWindowTitle(title);
    setTitle(title);
    setAutoReplot(false);
}



cplot::~cplot(void) {
    for (int i=0; i<curveList.size(); i++) {
        if (curveList[i]) delete curveList[i];
    }
}



void cplot::initializeDefaultWidget(options_t options) {
    canvasObj = new QwtPlotCanvas();
    canvasObj->setFrameStyle(QFrame::Box | QFrame::Plain);
    canvasObj->setLineWidth(1);
    canvasObj->setContentsMargins(0, 0, 0, 0);
    setCanvas(canvasObj);
    setCanvasBackground(Qt::white);
    QPen *majorPen = new QPen(QColor(175,175,175));
    QPen *minorPen = new QPen(QColor(235,235,235));
    gridObj = new QwtPlotGrid();
    gridObj->setMajorPen(*majorPen);
    gridObj->setMinorPen(*minorPen);
    gridObj->enableYMin(true);
    gridObj->enableXMin(true);
    gridObj->attach(this);
    zoomerObj = new QwtPlotZoomer(canvasObj);
    zoomerObj->setTrackerMode(QwtPicker::ActiveOnly);
    zoomerObj->setRubberBand(QwtPicker::RectRubberBand );
    zoomerObj->setRubberBandPen(QPen(Qt::red, 2, Qt::DotLine));
    zoomerObj->setTrackerPen(QPen(Qt::red));
    zoomerObj->setEnabled(false);
    pannerObj = new QwtPlotPanner(canvasObj);
    pannerObj->setMouseButton(Qt::MouseButton::LeftButton, Qt::KeyboardModifier::ControlModifier);

    plotLayout()->setAlignCanvasToScales(true);
    font.setFamily("Arial");
    font.setPixelSize(15);
    if (!parent()) {
        setContentsMargins(10, 10, 10, 10);
        setAxisFont(yLeft, font);
        setAxisFont(xBottom, font);
    }
    else {
        setContentsMargins(0, 0, 0, 0);
    }
    curveList.clear();
    resize(QSize(DEFAULT_WIDGETWIDTH, DEFAULT_WIDGETWIDTH));
    doubleClickSignalEnabled = false;
    zoomEnabled = false;
    if (options == enableDefault) {
        zoomEnabled = true;
    }
    setOptions(options);
}



void cplot::setOptions(options_t options) {
    doubleClickSignalEnabled |= (options & enableDoubleClickSignal)? true : false;
    zoomEnabled |= (options & enableZoom)? true : false;
    if (zoomEnabled) {
        zoomerObj->setEnabled(true);
    }
}



void cplot::setTitle(QString title, QFont *font) {
    QwtText s = title;
    if (font) s.setFont(*font);
    QwtPlot::setTitle(s);
}



void cplot::setAxisTitle(Axis axis, QString title, QFont *font) {
    QwtText s(title);
    if (font) s.setFont(*font);
    QwtPlot::setAxisTitle(axis, s);
}



// In this routine:
// event->pos() is pixel coord (int) from top left corner of canvas, which is larger than plot window
void cplot::wheelEvent(QWheelEvent *event) {
    if (zoomEnabled) {
        int delta = (int)event->delta();
        QPoint pRef = event->pos() - canvasObj->pos();
        QPointF pPlot = QPointF(invTransform(xBottom, pRef.x()), invTransform(yLeft, pRef.y()));
        double factor = 1.;
        if (delta > 0) factor = 1. / sqrt(2.);
        else if (delta < 0) factor = sqrt(2.);
        double oldXMin = axisScaleDiv(QwtPlot::xBottom).lowerBound();
        double oldXMax = axisScaleDiv(QwtPlot::xBottom).upperBound();
        double newXMin = pPlot.x() - (pPlot.x() - oldXMin) * factor;
        double newXMax = pPlot.x() + (oldXMax - pPlot.x()) * factor;
        setAxisScale(QwtPlot::xBottom, newXMin, newXMax);
        //double oldYMin = axisScaleDiv(QwtPlot::yLeft).lowerBound();
        //double oldYMax = axisScaleDiv(QwtPlot::yLeft).upperBound();
        //double newYMin = pPlot.y() - (pPlot.y() - oldYMin) * factor;
        //double newYMax = pPlot.y() + (oldYMax - pPlot.y()) * factor;
        //setAxisScale(QwtPlot::yLeft, newYMin, newYMax);
        replot();
    }
}



QwtPlotCanvas *cplot::canvas(void) {
    return canvasObj;
}



void cplot::mouseReleaseEvent(QMouseEvent *event) {
    if ((event->button() == Qt::RightButton) || (event->button() == Qt::MiddleButton)) {
        setAxisAutoScale(QwtPlot::yLeft, true);
        setAxisAutoScale(QwtPlot::xBottom, true);
        replot();
    }
}



void cplot::mouseDoubleClickEvent(QMouseEvent *) {
    emit plotDoubleClicked(this);
}



class TimeScaleDraw: public QwtScaleDraw {
public:
TimeScaleDraw(const QDateTime &base) : base(base) {
    setLabelRotation(0);
    setLabelAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    setSpacing(20);
}
virtual QwtText label(double hours) const {
    QDateTime dateTime;
    int days = (int)hours / 24.;
    dateTime = base.addDays(days);
    dateTime = dateTime.addSecs((hours - (double)days * 24.) * 3600.);
    return dateTime.toString("MMM.dd\nhh:mm");
}
private:
QDateTime base;
};



QwtPlotCurve *cplot::addCurve(const QList<CLapInfo> &laps, Qt::PenStyle penStyle, Qt::GlobalColor color, curveStyle_t curveStyle) {
    QwtPlotCurve *p = new QwtPlotCurve();
    p->attach(this);
    curveList.append(p);
    if (laps.size() == 0)
        return p;

    QDate startDate;
    QDate endDate;
    QVector<double> hours(laps.size());
    QVector<double> sec(laps.size());
    QDateTime base(QDate(2018, 1, 1), QTime(0, 0, 0));
    for (int i=0; i<laps.size(); i++) {
        CDateTime dt(laps[i].dateTime);
        if (i == 0)
            startDate = dt.toQDateTime().date();
        if (i ==(laps.size() - 1))
            endDate = dt.toQDateTime().date();
        hours[i] = (double)base.date().daysTo(dt.toQDate()) * 24. + (double)base.time().secsTo(dt.toQTime()) / 3600.;
        sec[i] = laps[i].lapSec;
    }

    setTitle(title().text() + " " + startDate.toString("MMM yyyy") + " to " + endDate.toString("MMM yyyy"));

    switch (curveStyle) {
    case lines:
        p->setSamples(hours, sec);
        break;
    case steps:
//    {
//        QVector<double> x(laps.size()*2 + 2, 0);
//        QVector<double> y(laps.size()*2 + 2, 0);
//        double step = 1;//vX.data[1] - vX.data[0];
//        int j=0;
//        x[j] = hours[0] - step / 2.;
//        y[j] = 0.;
//        for (int i=0; i<x.size(); i++) {
//            j++;
//            x[j] = x[i] - step / 2.;
//            y[j] = y[i];
//            j++;
//            x[j] = hours[i] + step / 2.;
//            y[j] = sec[i];
//        }
//        j++;
//        x[j] = hours[j-1];
//        y[j] = 0.;
//        p->setSamples(x, y);
//    }
        break;
    case noLine:
        break;
    }
    p->setPen(QPen(color, DEFAULT_PENWIDTH, penStyle));
//    qDebug() << axisScaleDiv(QwtPlot::xBottom).lowerBound() << axisScaleDiv(QwtPlot::xBottom);

    setAxisScaleDraw(QwtPlot::xBottom, new TimeScaleDraw(base));
    p->setRenderHint(QwtPlotItem::RenderAntialiased);
    return p;
}



void cplot::removeCurves(void) {
    for (int i=curveList.size()-1; i>= 0; i--) {
        curveList[i]->detach();
    }
}



QwtPlotMarker *cplot::addLabel(double x, double y, const QString &s, QFont *font, Qt::Alignment alignment) {
    QwtPlotMarker *marker = new QwtPlotMarker();
    QwtText text(s);
    if (font) text.setFont(*font);
    marker->setValue(x, y);
    marker->setLabel(text);
    marker->setLabelAlignment(alignment);
    marker->attach(this);
    return marker;
}



QwtPlotMarker *cplot::addVerticalLine(double x, const QString &s, QFont *font, Qt::Alignment alignment) {
    QwtPlotMarker *marker = new QwtPlotMarker();
    QwtText text(s);
    if (font) text.setFont(*font);
    marker->setLabel(text);
    marker->setLabelAlignment(alignment);
    marker->setLabelOrientation(Qt::Vertical);
    marker->setLineStyle(QwtPlotMarker::VLine);
    marker->setXValue(x);
    marker->attach(this);
    return marker;
}



QwtSymbol *cplot::newSymbol(symbol_t symbol) {
    QwtSymbol *qwtSymbol = NULL;
    switch (symbol) {
    case noSymbol:
        break;
    case blueDot:
        qwtSymbol = new QwtSymbol(QwtSymbol::Ellipse, QBrush(Qt::blue, Qt::SolidPattern), QPen(Qt::blue, 0., Qt::SolidLine), QSize(DEFAULT_DOTSIZE, DEFAULT_DOTSIZE));
        break;
    case redDot:
        qwtSymbol = new QwtSymbol(QwtSymbol::Ellipse, QBrush(Qt::red, Qt::SolidPattern), QPen(Qt::red, 0., Qt::SolidLine), QSize(DEFAULT_DOTSIZE, DEFAULT_DOTSIZE));
        break;
    case smallRedDot:
        qwtSymbol = new QwtSymbol(QwtSymbol::Ellipse, QBrush(Qt::red, Qt::SolidPattern), QPen(Qt::red, 0., Qt::SolidLine), QSize(SMALL_DOTSIZE, SMALL_DOTSIZE));
        break;
    case greenDot:
        qwtSymbol = new QwtSymbol(QwtSymbol::Ellipse, QBrush(Qt::green, Qt::SolidPattern), QPen(Qt::green, 0., Qt::SolidLine), QSize(DEFAULT_DOTSIZE, DEFAULT_DOTSIZE));
        break;
    case darkGreenDot:
        qwtSymbol = new QwtSymbol(QwtSymbol::Ellipse, QBrush(Qt::darkGreen, Qt::SolidPattern), QPen(Qt::darkGreen, 0., Qt::SolidLine), QSize(DEFAULT_DOTSIZE, DEFAULT_DOTSIZE));
        break;
    case blackDot:
        qwtSymbol = new QwtSymbol(QwtSymbol::Ellipse, QBrush(Qt::black, Qt::SolidPattern), QPen(Qt::black, 0., Qt::SolidLine), QSize(DEFAULT_DOTSIZE, DEFAULT_DOTSIZE));
        break;
    case darkYellowCircle:
        qwtSymbol = new QwtSymbol(QwtSymbol::Ellipse, QBrush(Qt::NoBrush), QPen(Qt::darkYellow, 3., Qt::SolidLine), QSize((int)(1.5*DEFAULT_DOTSIZE), (int)(1.5*DEFAULT_DOTSIZE)));
        break;
    case blueCross:
        qwtSymbol = new QwtSymbol(QwtSymbol::XCross, QBrush(Qt::NoBrush), QPen(Qt::blue, 3., Qt::SolidLine), QSize(DEFAULT_CROSSSIZE, DEFAULT_CROSSSIZE));
        break;
    case darkBlueCross:
        qwtSymbol = new QwtSymbol(QwtSymbol::XCross, QBrush(Qt::NoBrush), QPen(Qt::darkBlue, 3., Qt::SolidLine), QSize(DEFAULT_CROSSSIZE, DEFAULT_CROSSSIZE));
        break;
    case redCross:
        qwtSymbol = new QwtSymbol(QwtSymbol::XCross, QBrush(Qt::NoBrush), QPen(Qt::red, 3., Qt::SolidLine), QSize(DEFAULT_CROSSSIZE, DEFAULT_CROSSSIZE));
        break;
    case darkRedCross:
        qwtSymbol = new QwtSymbol(QwtSymbol::XCross, QBrush(Qt::NoBrush), QPen(Qt::darkRed, 3., Qt::SolidLine), QSize(DEFAULT_CROSSSIZE, DEFAULT_CROSSSIZE));
        break;
    case smallRedCross:
        qwtSymbol = new QwtSymbol(QwtSymbol::XCross, QBrush(Qt::NoBrush), QPen(Qt::red, 3., Qt::SolidLine), QSize(SMALL_CROSSSIZE, SMALL_CROSSSIZE));
        break;
    case greenCross:
        qwtSymbol = new QwtSymbol(QwtSymbol::XCross, QBrush(Qt::NoBrush), QPen(Qt::green, 3., Qt::SolidLine), QSize(DEFAULT_CROSSSIZE, DEFAULT_CROSSSIZE));
        break;
    case darkGreenCross:
        qwtSymbol = new QwtSymbol(QwtSymbol::XCross, QBrush(Qt::NoBrush), QPen(Qt::darkGreen, 3., Qt::SolidLine), QSize(DEFAULT_CROSSSIZE, DEFAULT_CROSSSIZE));
        break;
    case magentaCross:
        qwtSymbol = new QwtSymbol(QwtSymbol::XCross, QBrush(Qt::NoBrush), QPen(Qt::magenta, 3., Qt::SolidLine), QSize(DEFAULT_CROSSSIZE, DEFAULT_CROSSSIZE));
        break;
    case darkMagentaCross:
        qwtSymbol = new QwtSymbol(QwtSymbol::XCross, QBrush(Qt::NoBrush), QPen(Qt::darkMagenta, 3., Qt::SolidLine), QSize(DEFAULT_CROSSSIZE, DEFAULT_CROSSSIZE));
        break;
    case yellowCross:
        qwtSymbol = new QwtSymbol(QwtSymbol::XCross, QBrush(Qt::NoBrush), QPen(Qt::yellow, 3., Qt::SolidLine), QSize(DEFAULT_CROSSSIZE, DEFAULT_CROSSSIZE));
        break;
    case darkYellowCross:
        qwtSymbol = new QwtSymbol(QwtSymbol::XCross, QBrush(Qt::NoBrush), QPen(Qt::darkYellow, 3., Qt::SolidLine), QSize(DEFAULT_CROSSSIZE, DEFAULT_CROSSSIZE));
        break;
    }
    return qwtSymbol;
}



QwtPlotCurve *cplot::addPoints(const QList<CLapInfo> &laps, symbol_t symbol) {
    QwtPlotCurve *p = new QwtPlotCurve();
    p->attach(this);
    curveList.append(p);

    if (laps.size() == 0)
        return NULL;

    QDate startDate;
    QDate endDate;
    QVector<double> hours(laps.size());
    QVector<double> speed(laps.size());
    QDateTime base(QDate(2018, 1, 1), QTime(0, 0, 0));
    for (int i=0; i<laps.size(); i++) {
        QDateTime dt(CDateTime(laps[i].dateTime).toQDateTime());
        if (i == 0)
            startDate = dt.date();
        if (i ==(laps.size() - 1))
            endDate = dt.date();
        hours[i] = (double)base.date().daysTo(dt.date()) * 24. + (double)base.time().secsTo(dt.time()) / 3600.;
        if (laps[i].lapSec > 0.)
            speed[i] = laps[i].lapM / 1000. / (laps[i].lapSec / 3600.);
        else
            speed[i] = 0.;
    }

    if (startDate.year() == endDate.year())
        setTitle(title().text() + " " + startDate.toString("MMM yyyy"));
    else
        setTitle(title().text() + " " + startDate.toString("MMM yyyy") + " to " + endDate.toString("MMM yyyy"));

    p->setSamples(hours, speed);
//    p->setPen(QPen(color, DEFAULT_PENWIDTH, penStyle));

    setAxisScaleDraw(QwtPlot::xBottom, new TimeScaleDraw(base));
    p->setRenderHint(QwtPlotItem::RenderAntialiased);
    setAxisTitle(QwtPlot::yLeft, "Lap Speed (km/h)");
    p->setStyle(QwtPlotCurve::NoCurve);
    p->setSymbol(newSymbol(symbol));
    return p;
}



QwtPlotCurve *cplot::addPoint(double x, double y, symbol_t symbol) {
    curveList.append(new QwtPlotCurve());
    int listIndex = curveList.size()-1;
    curveList[listIndex]->setStyle(QwtPlotCurve::NoCurve);
    curveList[listIndex]->setSymbol(newSymbol(symbol));
    curveList[listIndex]->setSamples(&x, &y, 1);
    curveList[listIndex]->attach(this);
    return curveList[listIndex];
}



QwtPlotCurve *cplot::addHiddenPoint(double x, double y) {
    curveList.append(new QwtPlotCurve());
    int listIndex = curveList.size()-1;
    curveList[listIndex]->setStyle(QwtPlotCurve::NoCurve);
    curveList[listIndex]->setSymbol(NULL);
    curveList[listIndex]->setSamples(&x, &y, 1);
    curveList[listIndex]->attach(this);
    return curveList[listIndex];
}

