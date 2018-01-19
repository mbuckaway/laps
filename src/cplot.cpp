#include "cplot.h"




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


#include "cplot.h"



#define DEFAULT_WIDGETWIDTH     512

#define DEFAULT_CIRCLESIZE		12.
#define DEFAULT_DOTSIZE			10.
#define DEFAULT_CROSSSIZE		12.
#define DEFAULT_PENWIDTH		3.
#define SMALL_DOTSIZE           5.
#define SMALL_CROSSSIZE         8.
#define SMALL_PENWIDTH          3.





cplot::cplot(QString title, QWidget *parent)
    : QwtPlot(parent)
{
    initializeDefaultWidget(enableDefault);
    setWindowTitle(title);
    setTitle(title);
    setAutoReplot(false);
    //setMouseTracking(false);
}



cplot::cplot(QString title, options_t options, QWidget *parent)
    : QwtPlot(parent)
{
    initializeDefaultWidget(options);
    setWindowTitle(title);
    setTitle(title);
    setAutoReplot(false);
    //setMouseTracking(false);
}



cplot::cplot(QString title, QFont *font, options_t options, QWidget *parent)
    : QwtPlot(parent)
{
    initializeDefaultWidget(options);
    setWindowTitle(title);
    QwtText s(title);
    if (font) s.setFont(*font);
    QwtPlot::setTitle(s);
    setAutoReplot(false);
    //setMouseTracking(false);
}



//cplot::cplot(double x[], double y[], int nPts, options_t options, QWidget *parent)
//    : QwtPlot(parent)
//{
//    initializeDefaultWidget(options);
//    if (nPts == 0) return;
//    addCurve(x, y, nPts);
//    setAutoReplot(false);
//    //setMouseTracking(false);
//}




cplot::cplot(const QList<unsigned int> &dateTime, const QList<float> &y, curveStyle_t curveStyle, options_t options, QWidget *parent)
    : QwtPlot(parent)
{
    initializeDefaultWidget(options);
    if (dateTime.size() != y.size()) throw("cplot:: Length of vX and vY are not the same");
    if (dateTime.size() == 0) return;
    addCurve(dateTime, y, Qt::SolidLine, Qt::blue, curveStyle);
    setAutoReplot(false);
    //setMouseTracking(false);
}



//cplot::cplot(const CVector &vX, const CVector &vY, QString title, curveStyle_t curveStyle, options_t options, QWidget *parent)
//: QwtPlot(parent)
//{
//    initializeDefaultWidget(options);
//    setWindowTitle(title);
//    QwtText s(title);
//    QwtPlot::setTitle(s);
//    if (vX.size() != vY.size()) throw("cplot:: Length of vX and vY are not the same");
//    if (vX.size() == 0) return;
//    addCurve(vX, vY, Qt::SolidLine, Qt::blue, curveStyle);
//    setAutoReplot(false);
//    //setMouseTracking(false);
//}



//cplot::cplot(const CVector &vY, QString title, curveStyle_t curveStyle, options_t options, QWidget *parent)
//: QwtPlot(parent)
//{
//    initializeDefaultWidget(options);
//    if (vY.size() == 0) return;
//    int size = vY.size();
//    CVector vX(size, CVector::Zeros);
//    for (int i=0; i<size; i++) {
//        vX.data[i] = (double)i;
//    }
//    setWindowTitle(title);
//    QwtText s(title);
//    QwtPlot::setTitle(s);
//    addCurve(vX, vY, Qt::SolidLine, Qt::blue, curveStyle);
//    setAutoReplot(false);
//    //setMouseTracking(false);
//}




//cplot::cplot(const CVectorXY &vXY, curveStyle_t curveStyle, options_t options, QWidget *parent)
// : QwtPlot(parent)
//{
//    initializeDefaultWidget(options);
//    if (vXY.size() == 0) return;
//    addCurve(vXY, Qt::SolidLine, Qt::blue, curveStyle);
//    setAutoReplot(false);
//    //setMouseTracking(false);
//}



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
    //magnifierObj = new QwtPlotMagnifier(canvas());
    //magnifierObj->setWheelModifiers(Qt::KeyboardModifier::ControlModifier);
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
        if (delta > 0) factor = sqrt(2.);
        else if (delta < 0) factor = 1. / sqrt(2.);
        double oldXMin = axisScaleDiv(QwtPlot::xBottom).lowerBound();
        double oldXMax = axisScaleDiv(QwtPlot::xBottom).upperBound();
        double oldYMin = axisScaleDiv(QwtPlot::yLeft).lowerBound();
        double oldYMax = axisScaleDiv(QwtPlot::yLeft).upperBound();
        double newXMin = pPlot.x() - (pPlot.x() - oldXMin) * factor;
        double newXMax = pPlot.x() + (oldXMax - pPlot.x()) * factor;
        double newYMin = pPlot.y() - (pPlot.y() - oldYMin) * factor;
        double newYMax = pPlot.y() + (oldYMax - pPlot.y()) * factor;
        setAxisScale(QwtPlot::xBottom, newXMin, newXMax);
        setAxisScale(QwtPlot::yLeft, newYMin, newYMax);
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


QwtPlotCurve *cplot::addCurve(const double *x, const double *y, int size, Qt::PenStyle penStyle, Qt::GlobalColor color, curveStyle_t curveStyle) {
    QwtPlotCurve *p = new QwtPlotCurve();
    p->attach(this);
    curveList.append(p);
    switch (curveStyle) {
    case lines:
        p->setSamples(x, y, size);
        break;
    case steps:
//    {
//        CVector x2(size*2 + 2, CVector::Zeros);
//        CVector y2(size*2 + 2, CVector::Zeros);
//        double step = x[1] - x[0];
//        int j=0;
//        x2.data[j] = x[0] - step / 2.;
//        y2.data[j] = 0.;
//        for (int i=0; i<size; i++) {
//            j++;
//            x2.data[j] = x[i] - step / 2.;
//            y2.data[j] = y[i];
//            j++;
//            x2.data[j] = x[i] + step / 2.;
//            y2.data[j] = y[i];
//        }
//        j++;
//        x2.data[j] = x2.data[j-1];
//        y2.data[j] = 0.;
//        p->setSamples(x2.data, y2.data, x2.size());
//    }
        break;
    case noLine:
        break;
    }
    p->setPen(QPen(color, DEFAULT_PENWIDTH, penStyle));
    p->setRenderHint(QwtPlotItem::RenderAntialiased);
    return p;
}



QwtPlotCurve *cplot::addCurve(const double *x, const double *y, int size, QPen pen, curveStyle_t curveStyle) {
    QwtPlotCurve *p = new QwtPlotCurve();
    p->attach(this);
    curveList.append(p);
    switch (curveStyle) {
    case lines:
        p->setSamples(x, y, size);
        break;
    case steps:
//    {
//        CVector x2(size*2 + 2, CVector::Zeros);
//        CVector y2(size*2 + 2, CVector::Zeros);
//        double step = x[1] - x[0];
//        int j=0;
//        x2.data[j] = x[0] - step / 2.;
//        y2.data[j] = 0.;
//        for (int i=0; i<size; i++) {
//            j++;
//            x2.data[j] = x[i] - step / 2.;
//            y2.data[j] = y[i];
//            j++;
//            x2.data[j] = x[i] + step / 2.;
//            y2.data[j] = y[i];
//        }
//        j++;
//        x2.data[j] = x2.data[j-1];
//        y2.data[j] = 0.;
//        p->setSamples(x2.data, y2.data, x2.size());
//    }
        break;
    case noLine:
        break;
    }
    p->setPen(pen);
    p->setRenderHint(QwtPlotItem::RenderAntialiased);
    return p;
}



QwtPlotCurve *cplot::addCurve(const QList<unsigned int> &dateTime, const QList<float> &y, Qt::PenStyle penStyle, Qt::GlobalColor color, curveStyle_t curveStyle) {
    QwtPlotCurve *p = new QwtPlotCurve();
    p->attach(this);
    curveList.append(p);
    QString s;
    if (dateTime.size() == 0) return p;
    if (dateTime.size() != y.size()) throw(s.sprintf("QwtPlotCurve *cplot::addCurve(CVector &, CVector &): length of vX (%d) not equal to length of vY (%d)", dateTime.size(), y.size()));
    switch (curveStyle) {
    case lines:
//        p->setSamples(dateT.data, vY.data, vX.size());
        break;
    case steps:
//    {
//        CVector x(vX.size()*2 + 2, CVector::Zeros);
//        CVector y(vY.size()*2 + 2, CVector::Zeros);
//        double step = vX.data[1] - vX.data[0];
//        int j=0;
//        x.data[j] = vX.data[0] - step / 2.;
//        y.data[j] = 0.;
//        for (int i=0; i<vX.size(); i++) {
//            j++;
//            x.data[j] = vX.data[i] - step / 2.;
//            y.data[j] = vY.data[i];
//            j++;
//            x.data[j] = vX.data[i] + step / 2.;
//            y.data[j] = vY.data[i];
//        }
//        j++;
//        x.data[j] = x.data[j-1];
//        y.data[j] = 0.;
//        p->setSamples(x.data, y.data, x.size());
//    }
        break;
    case noLine:
        break;
    }
    p->setPen(QPen(color, DEFAULT_PENWIDTH, penStyle));
    p->setRenderHint(QwtPlotItem::RenderAntialiased);
    return p;
}



//QwtPlotCurve *cplot::addCurve(const CVectorXY &vXY, Qt::PenStyle penStyle, Qt::GlobalColor color, curveStyle_t curveStyle) {
//    QwtPlotCurve *p = new QwtPlotCurve();
//    p->attach(this);
//    double stepWidth = 1.;
//    if (vXY.size() >= 2) {
//        stepWidth = vXY.xData[1] - vXY.xData[0];
//    }
//    curveList.append(p);

//    switch (curveStyle) {
//    case steps:
//    {
//        CVector vY(2*(vXY.size()+1), CVector::Zeros);
//        CVector vX(vY.size(), CVector::Zeros);
//        vX.data[0] = vXY.xData[0] - stepWidth / 2.;
//        vY.data[0] = 0.;
//        for (int i=0; i<vXY.size(); i++) {
//            vX.data[2*i+1] = vXY.xData[i] - stepWidth / 2.;
//            vY.data[2*i+1] = vXY.yData[i];
//            vX.data[2*i+2] = vXY.xData[i] + stepWidth / 2.;
//            vY.data[2*i+2] = vXY.yData[i];
//        }
//        vX.data[2*vXY.size()+1] = vXY.xData[vXY.size()-1] + stepWidth / 2.;
//        vY.data[2*vXY.size()+1] = 0.;
//        p->setSamples(vX.data, vY.data, vX.size());
//    }
//        break;
//    case lines:
//    {
//        CVector vX(vXY.size(), CVector::Zeros);
//        CVector vY(vX.size(), CVector::Zeros);
//        for (int i=0; i<vX.size(); i++) {
//            vX.data[i] = vXY.xData[i];
//            vY.data[i] = vXY.yData[i];
//        }
//        p->setSamples(vX.data, vY.data, vX.size());
//    }
//        break;
//    case noLine:
//        break;
//    }
//    p->setPen(QPen(color, DEFAULT_PENWIDTH, penStyle));
//    p->setRenderHint(QwtPlotItem::RenderAntialiased);
//    return p;
//}



//QwtPlotCurve *cplot::addCurve(const CVector &vY, Qt::PenStyle penStyle, Qt::GlobalColor color, curveStyle_t curveStyle) {
//    QwtPlotCurve *p = new QwtPlotCurve();
//    p->attach(this);
//    int size = vY.size();
//    CVector vX(size, CVector::Zeros);
//    for (int i=0; i<size; i++) {
//        vX.data[i] = (double)i;
//    }
//    curveList.append(p);
//    switch (curveStyle) {
//    case lines:
//        p->setSamples(vX.data, vY.data, size);
//        break;
//    case steps:
//    {
//        CVector x2(size*2 + 2, CVector::Zeros);
//        CVector y2(size*2 + 2, CVector::Zeros);
//        double step = vX.data[1] - vX.data[0];
//        int j=0;
//        x2.data[j] = vX.data[0] - step / 2.;
//        y2.data[j] = 0.;
//        for (int i=0; i<size; i++) {
//            j++;
//            x2.data[j] = vX.data[i] - step / 2.;
//            y2.data[j] = vY.data[i];
//            j++;
//            x2.data[j] = vX.data[i] + step / 2.;
//            y2.data[j] = vY.data[i];
//        }
//        j++;
//        x2.data[j] = x2.data[j-1];
//        y2.data[j] = 0.;
//        p->setSamples(x2.data, y2.data, x2.size());
//    }
//        break;
//    case noLine:
//        break;
//    }
//    p->setPen(QPen(color, DEFAULT_PENWIDTH, penStyle));
//    p->setRenderHint(QwtPlotItem::RenderAntialiased);
//    return p;
//}



//QwtPlotCurve *cplot::addCurve(const CVector &vY, QPen pen, curveStyle_t curveStyle) {
//    QwtPlotCurve *p = new QwtPlotCurve();
//    p->attach(this);
//    int size = vY.size();
//    CVector vX(size, CVector::Zeros);
//    for (int i=0; i<size; i++) {
//        vX.data[i] = (double)i;
//    }
//    curveList.append(p);
//    switch (curveStyle) {
//    case lines:
//        p->setSamples(vX.data, vY.data, size);
//        break;
//    case steps:
//    {
//        CVector x2(size*2 + 2, CVector::Zeros);
//        CVector y2(size*2 + 2, CVector::Zeros);
//        double step = vX.data[1] - vX.data[0];
//        int j=0;
//        x2.data[j] = vX.data[0] - step / 2.;
//        y2.data[j] = 0.;
//        for (int i=0; i<size; i++) {
//            j++;
//            x2.data[j] = vX.data[i] - step / 2.;
//            y2.data[j] = vY.data[i];
//            j++;
//            x2.data[j] = vX.data[i] + step / 2.;
//            y2.data[j] = vY.data[i];
//        }
//        j++;
//        x2.data[j] = x2.data[j-1];
//        y2.data[j] = 0.;
//        p->setSamples(x2.data, y2.data, x2.size());
//    }
//        break;
//    case noLine:
//        break;
//    }
//    p->setPen(QPen(pen));
//    p->setRenderHint(QwtPlotItem::RenderAntialiased);
//    return p;
//}



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



QwtPlotMarker *cplot::addVLine(double x, const QString &s, QFont *font, Qt::Alignment alignment) {
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



//QwtPlotCurve *cplot::addPoints(const CVector &vX, const CVector &vY, symbol_t symbol, CRange range) {
//    if (vX.size() == 0) return NULL;
//    if (vY.size() == 0) return NULL;
//    if (vX.size() != vY.size()) throw("cplot::addPoints() Vector lengths are not the same");
//    int first = 0;
//    if (!range.fromStart()) first = range.start();
//    int last = vX.size() - 1;
//    if (!range.toEnd()) last = range.end();
//    int size = last - first + 1;
//    if (size < 0) throw("Requested range in cplot::addCurve(CVector, CVector, QwtSymbol, CRange) is less than zero");
//    if (size == 0) return NULL;
//    curveList.append(new QwtPlotCurve());
//    int listIndex = curveList.size()-1;
//    curveList[listIndex]->setStyle(QwtPlotCurve::NoCurve);
//    curveList[listIndex]->setSymbol(newSymbol(symbol));
//    curveList[listIndex]->setSamples(&vX.data[first], &vY.data[first], size);
//    curveList[listIndex]->attach(this);
//    return curveList[listIndex];
//}



//QwtPlotCurve *cplot::addPoints(const CVector &vY, symbol_t symbol, CRange range) {
//    if (vY.size() == 0) return NULL;
//    CVector vX(vY.size(), CVector::ZeroOneTwo);
//    int first = 0;
//    if (!range.fromStart()) first = range.start();
//    int last = vX.size() - 1;
//    if (!range.toEnd()) last = range.end();
//    int size = last - first + 1;
//    if (size < 0) throw("Requested range in cplot::addCurve(CVector, QwtSymbol, CRange) is less than zero");
//    if (size == 0) return NULL;
//    curveList.append(new QwtPlotCurve());
//    int listIndex = curveList.size()-1;
//    curveList[listIndex]->setStyle(QwtPlotCurve::NoCurve);
//    curveList[listIndex]->setSymbol(newSymbol(symbol));
//    curveList[listIndex]->setSamples(&vX.data[first], &vY.data[first], size);
//    curveList[listIndex]->attach(this);
//    return curveList[listIndex];
//}



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





