#include "measurement.h"
#include <device/settings.h>
int Measurement::objects_cnt_ = 0;

int Measurement::static_index_ = 0;


//======================================================================
Measurement::Measurement(E_MEASUREMENTS type, QCustomPlot *p, int rep_count): plot_(p), current_selection_(0), type_(type), finished_(false), selection_stoped_(false)
{
    objects_cnt_ ++;
    static_index_ ++;

    id_ = static_index_;

    for (int i = 0; i < plot()->curveCount(); i++)
    {
        addConnection(connect(plot()->curve(i), SIGNAL(selectionChanged(const QCPDataSelection &)), this, SLOT(graphSelectionChanged(const QCPDataSelection &))));
    }

    addConnection(connect(plot(), &QCustomPlot::mouseMove, this, [=](QMouseEvent *event) {

            QVector2D cursor_coords(plot()->xAxis->pixelToCoord(event->pos().x()), plot()->yAxis->pixelToCoord(event->pos().y()));
            if (selectionCount() > 0 && selection(0) && selection(0)->valid())
            {
                cursor_coords -= QVector2D( selection(0)->data_begin().x(),
                                            selection(0)->data_begin().y());
            }
            else
            {
                cursor_coords -= QVector2D( plot()->curve(0)->data()->at(0)->mainKey(),
                                            plot()->curve(0)->data()->at(0)->mainValue());
            }

            relative_cursor_pos_ = cursor_coords.toPointF();

            if (Settings::instance().view.axis_unit == ViewParameters::MK) //convert to mm here
            {
                relative_cursor_pos_ = QPointF(relative_cursor_pos_.x()* (1.0e-4), relative_cursor_pos_.y()* (1.0e-4));
            }
            if (Settings::instance().view.axis_unit == ViewParameters::NM) //convert to mm here
            {
                relative_cursor_pos_ = QPointF(relative_cursor_pos_.x()* (1.0e-6), relative_cursor_pos_.y()* (1.0e-6));
            }

            calculateMeasurement();
            plot()->replot();
            emit measumentChanged();

    }));


    setReperCount(rep_count);

    reloadSettings();
}

//======================================================================
Measurement::~Measurement()
{
    deactivate();
    objects_cnt_ --;

    for (auto r: repers_)
    {
        emit reperPointRemoved(r);
        delete r;
    }

    qDebug() << "dctr measure ";
}

//======================================================================
E_MEASUREMENTS Measurement::type()
{
    return type_;
}

//======================================================================
DataSelection * Measurement::selection(size_t i)
{
    if (selectionCount() == 0) return nullptr;
    if (i <= selectionCount()-1)
        return &selections_[i];
    else
        return nullptr;
}

//======================================================================
size_t Measurement::selectionCount()
{
    return selections_.count();
}

//======================================================================
void Measurement::changeSelection(size_t index, DataSelection s)
{
    selections_[index] = s;
}

//======================================================================
void Measurement::addSelection(DataSelection s)
{
    selections_.push_back(s);
}

//======================================================================
void Measurement::nextSelection()
{
    current_selection_ ++;
    addSelection(DataSelection(0,0,0));
}

//======================================================================
void Measurement::stopSelection()
{
    selection_stoped_ = true;
}

//======================================================================
bool Measurement::allSelectionsValid()
{
    qDebug() << "Selections " << selections_.count();
    if (selections_.empty())
        return false;

    for (auto s : selections_)
    {
        qDebug() << "Selection valid: " << s.valid();
        if (!s.valid()) return false;
    }

    return true;
}

//======================================================================
void Measurement::graphSelectionChanged(const QCPDataSelection &sel)
{
    if (selection_stoped_) return;
    QCPCurve *g  = (QCPCurve*)QObject::sender();
    if (current_selection_ >= selectionCount())
        addSelection(DataSelection(sel.dataRange().begin(), sel.dataRange().begin(), g ));
    else
        changeSelection(current_selection_, DataSelection(sel.dataRange().begin(), sel.dataRange().end(), g ));
}

//======================================================================
QCustomPlot* Measurement::plot()
{
    return plot_;
}


//======================================================================
int Measurement::id()
{
    return id_;
}

/*
//======================================================================
void Measurement::setSelection(const QCPDataSelection &selection, QCPCurve *g)
{
    //a crook for multiple X-range selection.
    //we use real qcp selection for updating selection_ranges until mouse release.
    last_qcp_data_selection_ = selection;
    selection_ranges_[current_selection_] = selection.dataRange();
    graph_ = g;
}
*/

//======================================================================
/*QCPDataSelection Measurement::selection()
{
    QCPDataSelection s;
    for (auto r: selection_ranges_)
        s.addDataRange(r);
    return s;
}*/

//======================================================================
QPointF Measurement::rel_cursor_pos()
{
    if (Settings::instance().view.axis_unit == ViewParameters::MK) //convert back to mkm here
    {
        return QPointF(relative_cursor_pos_.x()* 1.0e4, relative_cursor_pos_.y()*1.0e4);
    }
    if (Settings::instance().view.axis_unit == ViewParameters::NM) //convert back to nm here
    {
        return QPointF(relative_cursor_pos_.x()* 1.0e6, relative_cursor_pos_.y()* 1.0e6);
    }

    return relative_cursor_pos_;
}

//======================================================================
void Measurement::addConnection(QMetaObject::Connection conn)
{
    connections_.push_back(conn);
}

//======================================================================
void Measurement::removeConnection(const QMetaObject::Connection &conn)
{
    connections_.removeOne(conn);
    QObject::disconnect(conn);
}

//======================================================================
void Measurement::finish()
{
    qDebug() << "measurement finished";
    finished_ = true;
    /*
    if (graph()) //can be no graph (in case of distance measurements for example)
    {
        graph()->setAdditionSelectionDataRanges( QVector <QCPDataRange> ());
        graph()->setSelection(QCPDataSelection());
    }
    */
    deactivate();
    select(false);
    plot()->replot();
}

//======================================================================
bool Measurement::isFinished()
{
    qDebug() << "finished" << finished_;
    return finished_;
}

//======================================================================
void Measurement::deactivate()
{
    for (auto conn: connections_)
        QObject::disconnect(conn);
}


//======================================================================
void Measurement::setReper(int id, QPointF pt)
{
    repers_[id]->setPosition(pt);
    repers_[id]->setVisible(true);
    emit reperPointChanged(repers_[id]);
}

//======================================================================
void Measurement::setReperCount(int val)
{
    for (int i = 0; i < val; i++)
    repers_.push_back(new ReperPoint(plot(), this));
}

//======================================================================
void Measurement::setNameText(QString text)
{
    name_text_ = text;
}

//======================================================================
void Measurement::clearNameText()
{
    name_text_ = "";
}

//======================================================================
void Measurement::activatePlotDecorators()
{
 //stub for abstract measurement
}

//======================================================================
double Measurement::guessRequestedValue()
{
    return 0;
}

//======================================================================
double Measurement::guessOptimizationWeight()
{
    return 1;
}

//======================================================================
//
//======================================================================
ReperPoint::ReperPoint(QCustomPlot *plot, Measurement *m): plot_(plot), m(m)
{
    reper_ = new QCPItemTracer(plot);
    reper_->setStyle(QCPItemTracer::tsPlus);
    reper_->setPen(kPenMarker);
    reper_->setBrush(Qt::red);
    reper_->setSize(7);
    reper_->setVisible(false);
    reper_->setLayer("overlay");
}

//======================================================================
ReperPoint::~ReperPoint()
{
    plot_->removeItem(reper_);
}

//======================================================================
void ReperPoint::setPosition(QPointF pt)
{
    reper_->position->setCoords(pt);
}

//======================================================================
QPointF ReperPoint::getPosition()
{
    return reper_->position->coords();
}

//======================================================================
bool ReperPoint::setActive(bool v)
{
    QCPItemTracer::TracerStyle st = reper_->style();

    reper_->setStyle(v ? QCPItemTracer::tsCircle : QCPItemTracer::tsPlus);
    reper_->setPen(v ? kPenActive : kPenMarker);
    reper_->setBrush(v ? Qt::green : Qt::red);
    return st == QCPItemTracer::tsCircle;
}

//======================================================================
Measurement * ReperPoint::measurement()
{
    return m;
}

//======================================================================
void ReperPoint::setVisible(bool v)
{
    reper_->setVisible(v);
}





