#include "reperpointcontroller.h"
#include "measurements/measurement.h"


//======================================================================
ReperPointController::ReperPointController(QCustomPlot *plt)
{
    plot_ = plt;
}

//======================================================================
void ReperPointController::reperPointChanged(ReperPoint *p)
{
    repers[p] = p;
}

//======================================================================
void ReperPointController::reperPointRemoved(ReperPoint *p)
{
    repers.remove(p);
}
//======================================================================
void ReperPointController::mouseMove(QMouseEvent *e)
{
    //find closes reper to mouse

    bool need_replot = false;
    for (auto r : repers)
    {
        QPointF reppos = r->getPosition();
        QPoint rep_pix_pos(plot_->xAxis->coordToPixel(reppos.x()), plot_->yAxis->coordToPixel(reppos.y()));
        qreal dist = (e->pos() - rep_pix_pos).manhattanLength();

        if (dist < 5 && r->measurement()->isFinished()) //5px
        {
            QCursor::setPos(plot_->mapToGlobal( rep_pix_pos));
            need_replot |= !r->setActive(true);
            plot_->replot();
        }
        else
        {
            need_replot |= r->setActive(false);
        }
    }
    if (need_replot) plot_->replot();
}
