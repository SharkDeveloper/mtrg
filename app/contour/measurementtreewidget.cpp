#include "measurementtreewidget.h"
#include <QDebug>
#include <QAction>
#include <QMenu>
#include <QPainter>
#include <QStyledItemDelegate>

//======================================================================
Header::Header(QWidget* parent)
    : QHeaderView(Qt::Horizontal, parent)
{

}

//======================================================================
void Header::mousePressEvent ( QMouseEvent * event )
{
    QHeaderView::mousePressEvent(event);

    int logicalIndex = logicalIndexAt(event->pos());

    if (buttonMenuRect(logicalIndex).contains(event->pos())) {
        emit newTopLevelItem();
    }
}

//======================================================================
void Header::mouseMoveEvent(QMouseEvent * event)
{
    QHeaderView::mouseMoveEvent(event);

    // Required to refresh the button menu enable/disable state.
    int logicalIndex = logicalIndexAt(event->pos());
    updateSection(logicalIndex);
}

//======================================================================
void Header::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();

    QHeaderView::paintSection(painter, rect, logicalIndex);

    painter->restore();

    if (logicalIndex == 0)
    {
        QPoint pos = mapFromGlobal(QCursor::pos());
        if (rect.contains(pos)) {
            drawMenuButton(painter, logicalIndex, buttonMenuRect(logicalIndex).contains(pos));
        }
    }
}

//======================================================================
QRect Header::sectionRect(int logicalIndex) const
{
    return QRect(sectionViewportPosition(logicalIndex), 0, sectionSize(logicalIndex), height());
}

//======================================================================
QRect Header::buttonMenuRect(int logicalIndex) const
{
    QRect sr = sectionRect(logicalIndex);

    return QRect(sr.left()+1, sr.top()+1, sr.height()-2, sr.height()-2);
}

//======================================================================
void Header::drawMenuButton(QPainter *painter, int logicalIndex, bool enabled) const
{
    QRect brect = buttonMenuRect(logicalIndex);

    painter->setPen(enabled ? QColor(186,186,186) : QColor(223, 223, 223));
    painter->setBrush(QColor(246,246,246));
    painter->drawRect(brect.adjusted(0,0,-1,-1));

    painter->setPen(enabled ? QColor(71,71,71) : QColor(193, 193, 193));
    painter->drawLine(brect.center().x()-(brect.width()/3-1),
                      brect.center().y(),
                      brect.center().x()+(brect.width()/3-1),
                      brect.center().y());

    painter->drawLine(brect.center().x(),
                      brect.center().y()-(brect.height()/3-1),
                      brect.center().x(),
                      brect.center().y()+(brect.height()/3-1));

    //painter->drawLine(brect.left()+4, brect.top()+6, brect.right()-4, brect.top()+6);
    //painter->drawLine(brect.left()+5, brect.top()+7, brect.right()-5, brect.top()+7);
    //painter->drawPoint(brect.left()+6, brect.top()+8);
}

//======================================================================
class NoEditDelegate: public QStyledItemDelegate {
    public:
      NoEditDelegate(QObject* parent=0): QStyledItemDelegate(parent) {}
      virtual QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
        return 0;
      }
    };


//======================================================================
DoubleSpinBoxDelegate::DoubleSpinBoxDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

//======================================================================
QWidget *DoubleSpinBoxDelegate::createEditor(QWidget *parent,
                                       const QStyleOptionViewItem &/* option */,
                                       const QModelIndex &/* index */) const
{
    QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
    editor->setFrame(false);
    editor->setMinimum(-10000);
    editor->setMaximum(10000);
    editor->setDecimals(6);

    return editor;
}

//======================================================================
void DoubleSpinBoxDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    double value = index.model()->data(index, Qt::EditRole).toDouble();

    QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
    spinBox->setValue(value);
}

//======================================================================
void DoubleSpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
    spinBox->interpretText();
    double value = spinBox->value();

    model->setData(index, value, Qt::EditRole);

    ((MeasurementTreeWidget *)parent())->recalcMeans();
}

//======================================================================
void DoubleSpinBoxDelegate::updateEditorGeometry(QWidget *editor,
                                           const QStyleOptionViewItem &option,
                                           const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

//======================================================================
MeasurementTreeWidget::MeasurementTreeWidget(QWidget *parent) : QTreeWidget(parent), calibration_view_(false)
{
    m_header = new Header(this);
    setHeader(m_header);
    connect (m_header, &Header::newTopLevelItem, this, [&]() {

        //add new toplevel for editing
        auto item = new QTreeWidgetItem();

        item->setFlags(item->flags() | Qt::ItemFlag::ItemIsEditable);

        item->setText(0,"ГРУППА");
        item->setTextAlignment(0, Qt::AlignLeft);
        item->setTextAlignment(1, Qt::AlignRight);

        item->setFlags(item->flags() ^ Qt::ItemIsDragEnabled);
        this->addTopLevelItem(item);
        this->editItem(item);
    });

    //this->setItemDelegateForColumn(1, new NoEditDelegate(this));

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QTreeWidget::customContextMenuRequested,this, [=](const QPoint & pos){

        QTreeWidgetItem *nd = itemAt( pos );

        //check if any item clicked
        if (!nd) {return;}

        if (nd  == default_group_) //no default group deletion
            return;


        QAction *deleteMeasureAction = new QAction("Удалить", this);
        connect(deleteMeasureAction, &QAction::triggered, this, [=]()
        {
            if (calibration_view_)
            {
                emit deleteMeasurement(nd->data(0, Qt::UserRole).toInt());
                return;
            }
            else
            {
                if (!nd->parent())
                {
                    //if this is group: move all childs to default
                    default_group_->addChildren(nd->takeChildren());
                    return;
                }
                //if this is measure
                {
                    emit deleteMeasurement(nd->data(0, Qt::UserRole).toInt());
                    return;
                }
            }
        });

        QMenu menu(this);
        menu.addAction(deleteMeasureAction);
        menu.exec( mapToGlobal(pos) );
    });

    default_group_ = new QTreeWidgetItem(this);
    default_group_->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled );
    default_group_->setFlags(default_group_->flags() ^ Qt::ItemIsDragEnabled);
    default_group_->setTextAlignment(0, Qt::AlignLeft);
    default_group_->setTextAlignment(1, Qt::AlignRight);

    addTopLevelItem(default_group_);

    header()->setStretchLastSection(true);
    header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    setFont(kFontFixed);
    expandAll();
    auto rooItem = invisibleRootItem();
    rooItem->setFlags( rooItem->flags() ^ Qt::ItemIsDropEnabled );
}

//======================================================================
void  MeasurementTreeWidget::dropEvent(QDropEvent * event)
{
    QTreeWidgetItem *nd = itemAt( event->pos() );

    //check if any item clicked
    if (!nd) {return;}

    //check if clicked item is toplevel
    if(nd->parent()) {return;}

   QTreeWidget::dropEvent(event);

   recalcMeans();
}


enum E_HPOS
{
    E_NAME,
    E_VAL,
    E_REQ,
    E_WT,
    E_ERR,
};

//======================================================================
void MeasurementTreeWidget::recalcMeans()
{
    errors_.clear();
    if (!calibration_view_)
    {
        for (int i =0; i < topLevelItemCount(); i++)
        {
            double summ = 0;
            if (topLevelItem(i)->childCount() == 0 )
            {
                topLevelItem(i)->setText(1, "");
                continue;
            }
            for (int j = 0; j < topLevelItem(i)->childCount(); j++)
            {
                summ += topLevelItem(i)->child(j)->data(1, Qt::UserRole).toDouble();
            }
            summ /= topLevelItem(i)->childCount();
            topLevelItem(i)->setText(1, QString::number(summ, 'f', 4));
        }
    }
    else
    {
        //gather errors
        for (int i =0; i < topLevelItemCount(); i++)
        {
            double err = topLevelItem(i)->data(E_ERR, Qt::UserRole).toDouble();
            errors_.push_back(err);
        }
    }

}

//======================================================================
QVector <double> MeasurementTreeWidget::getErrors()
{
   return errors_;
}

//======================================================================
void MeasurementTreeWidget::setDefaultGroupName(const QString &text)
{
    default_group_->setText(0,text);
}

//======================================================================
QTreeWidgetItem * MeasurementTreeWidget::addMeasurement(Measurement *m)
{
    auto item = new QTreeWidgetItem();

    if (!calibration_view_)
    {
        item->setText(0, m->name());
        item->setText(1, m->value());

        item->setData(0, Qt::UserRole, m->id());
        item->setData(1, Qt::UserRole, m->dvalue());
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
    }
    else
    {
        item->setText(0, m->name());
        item->setText(1, m->value());

        item->setText(2, QString::number(m->guessRequestedValue()));
        item->setText(3, QString::number(m->guessOptimizationWeight()));
        item->setText(4, QString::number(m->dvalue() - m->guessRequestedValue()));

        item->setData(0, Qt::UserRole, m->id());
        item->setData(1, Qt::UserRole, m->dvalue());
        item->setData(4, Qt::UserRole, m->dvalue() - m->guessRequestedValue());
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
    }


    item->setTextAlignment(0, Qt::AlignLeft);
    item->setTextAlignment(1, Qt::AlignRight);

    if (!calibration_view_)
        default_group_->addChild(item);
    else
        this->addTopLevelItem(item);
    return item;
}

//======================================================================
std::vector<MeasurementTreeWidget::SGroup> MeasurementTreeWidget::groups()
{
    std::vector<MeasurementTreeWidget::SGroup> res;

    for (int i =0; i < topLevelItemCount(); i++)
    {
        MeasurementTreeWidget::SGroup group;
        group.name = topLevelItem(i)->text(0).toStdString();
        group.mean = topLevelItem(i)->text(1).toStdString();

        if (topLevelItem(i)->childCount() == 0) continue; //ignore empty groups, not going to report

        for (int j = 0; j < topLevelItem(i)->childCount(); j++)
        {
            group.measurements.push_back(SMeasurement {topLevelItem(i)->child(j)->text(0).toStdString(),
                                                       topLevelItem(i)->child(j)->text(1).toStdString(),
                                                       topLevelItem(i)->child(j),
                                                       topLevelItem(i)->child(j)->data(0, Qt::UserRole).toInt()});
        }
        res.push_back(group);
    }
    return res;
}


//======================================================================
void MeasurementTreeWidget::setCalubrationView(bool val)
{
    calibration_view_ = val;
    if (val)
    {
        //set stock header
        this->setHeader(new QHeaderView(Qt::Orientation::Horizontal,this));
        this->setColumnCount(4);

        this->headerItem()->setText(E_NAME, "ИЗМЕРЕНИЕ");
        this->headerItem()->setText(E_VAL, "ЗНАЧЕНИЕ");
        this->headerItem()->setText(E_REQ, "ЗАДАНО");
        this->headerItem()->setText(E_WT, "ВЕС");
        this->headerItem()->setText(E_ERR,"ОШИБКА");

        this->setItemDelegateForColumn(E_NAME, new NoEditDelegate(this));
        this->setItemDelegateForColumn(E_VAL, new NoEditDelegate(this));
        this->setItemDelegateForColumn(E_REQ, new DoubleSpinBoxDelegate(this));
        this->setItemDelegateForColumn(E_WT, new DoubleSpinBoxDelegate(this));
        this->setItemDelegateForColumn(E_ERR, new NoEditDelegate(this));


        delete default_group_;
        //maybe here we need to go throug all top-level items and allow edit for them
    }
    else
    {
        this->setColumnCount(2);
    }

}

void MeasurementTreeWidget::updateItem(QTreeWidgetItem *item, Measurement *m)
{
    if (!calibration_view_)
    {
        item->setText(E_VAL, m->value());
        item->setData(E_VAL, Qt::UserRole, m->dvalue());
    }
    else
    {
        item->setText(E_VAL, m->value());
        item->setData(E_VAL, Qt::UserRole, m->dvalue());

        //if item req val was edited: skip set, else - set default
        if (item->data(E_REQ, Qt::EditRole).isNull() || !m->isFinished())
        {
            item->setText(E_REQ, QString::number(m->guessRequestedValue()));
            item->setData(E_REQ, Qt::UserRole, m->guessRequestedValue());
        }

        //if item wt val was edited: skip set, else - set default
        if (item->data(E_WT, Qt::EditRole).isNull() || !m->isFinished())
        {
            item->setText(E_WT, QString::number(m->guessOptimizationWeight()));
            item->setData(E_WT, Qt::UserRole, m->guessOptimizationWeight());
        }

        if (item->data(E_REQ, Qt::EditRole).isNull() || !m->isFinished())
        {
            double err = m->dvalue() - item->data(E_REQ, Qt::UserRole).toDouble();
            double wt = 1;
            if (item->data(E_WT, Qt::EditRole).isNull() || !m->isFinished())
                wt = item->data(E_WT, Qt::UserRole).toDouble();
            else
                wt = item->data(E_WT, Qt::EditRole).toDouble();

            err = err*wt;

            item->setText(E_ERR, QString::number(qSqrt(err*err)));
            item->setData(E_ERR, Qt::UserRole, qSqrt(err*err));
        }
        else
        {
            double err = m->dvalue() - item->data(E_REQ, Qt::EditRole).toDouble();
            double wt = 1;
            if (item->data(E_WT, Qt::EditRole).isNull() || !m->isFinished())
                wt = item->data(E_WT, Qt::UserRole).toDouble();
            else
                wt = item->data(E_WT, Qt::EditRole).toDouble();

            err = err*wt;
            item->setText(E_ERR, QString::number(qSqrt(err*err)));
            item->setData(E_ERR, Qt::UserRole, qSqrt(err*err));
        }
    }
}
