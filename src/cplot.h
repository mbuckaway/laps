#ifndef CPLOT_H
#define CPLOT_H

#include <QSize>
#include <QPen>
#include <QBrush>
#include <QList>
#include <QObject>
#include <QMouseEvent>
#include <QWheelEvent>


#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>
#include <qwt_symbol.h>
#include <qwt_plot_canvas.h>
#include <qwt_legend.h>
#include <qwt_scale_engine.h>


#include "cdbase.h"


class cplot : public QwtPlot {
    Q_OBJECT
public:
    enum symbol_t {noSymbol, blueDot, redDot, smallRedDot, greenDot, darkGreenDot, blackDot, darkYellowCircle, blueCross, darkBlueCross, redCross, darkRedCross, smallRedCross, greenCross, darkGreenCross, yellowCross, darkYellowCross, magentaCross, darkMagentaCross};
    //enum line_t {noLine, blueLine, redLine};
    enum curveStyle_t {noLine, lines, steps};
    enum options_t {enableNone=0, enableDefault=0x01, enableDoubleClickSignal=0x02, enableZoom=0x04, enableAll=0xffff};
    cplot(QString title=QString(), options_t=enableDefault, QWidget *parent=NULL);
    ~cplot(void);

    QwtPlotCurve *addCurve(const QList<CLapInfo> &laps, Qt::PenStyle=Qt::SolidLine, Qt::GlobalColor=Qt::blue, curveStyle_t=lines);
    void removeCurves(void);
    QwtPlotCurve *addPoints(const QList<CLapInfo> &laps, symbol_t=blueDot);
    QwtPlotCurve *addPoint(double x, double y, symbol_t=blueDot);
    QwtPlotCurve *addHiddenPoint(const QDateTime &, double y);
    QwtPlotMarker *addLabel(double x, double y, const QString &text, QFont *font=NULL, Qt::Alignment=Qt::AlignRight | Qt::AlignTop);
    QwtPlotMarker *addVerticalLine(double x, const QString &text, QFont* =NULL, Qt::Alignment=Qt::AlignLeft | Qt::AlignBottom);
    void setOptions(options_t options);
    void setTitle(QString, QFont *font=NULL);
    void setAxisTitle(Axis axis, QString, QFont* font=NULL);
    void setupWheelZooming(void);
signals:
    void plotDoubleClicked(cplot *);
protected:
    void wheelEvent(QWheelEvent *event);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);
private:
    void initializeDefaultWidget(options_t);
    QwtPlotCanvas *canvas(void);
    QwtSymbol *newSymbol(symbol_t);
    bool doubleClickSignalEnabled;
    bool zoomEnabled;
    QwtPlotCanvas *canvasObj;
    QwtPlotGrid *gridObj;
    QwtPlotZoomer *zoomerObj;
    QwtPlotMagnifier *magnifierObj;
    QwtPlotPanner *pannerObj;
    QList<QwtPlotCurve *> curveList;
    QFont font;
private slots:
};


#endif // CPLOT_H
