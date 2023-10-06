#ifndef MEASUREMENTTREEWIDGET_H
#define MEASUREMENTTREEWIDGET_H

#include <QObject>
#include <QTreeWidget>
#include <QDropEvent>
#include <QPushButton>
#include <QHBoxLayout>
#include <QHeaderView>
#include "measurements/measurement.h"

//======================================================================
class Header
    : public QHeaderView
{
    Q_OBJECT
public:
    Header(QWidget* parent);
signals:
    void newTopLevelItem();
private:

    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const;

    QRect sectionRect(int logicalIndex) const;
    QRect buttonMenuRect(int logicalIndex) const;
    void drawMenuButton(QPainter *painter, int logicalIndex, bool enabled) const;

};

//======================================================================
class DoubleSpinBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    DoubleSpinBoxDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;
};


//======================================================================
class MeasurementTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    struct SMeasurement
    {
        std::string name;
        std::string value;
        QTreeWidgetItem *item;
        int id;
    };

    struct SGroup {
        std::string name;
        std::string mean;
        std::vector <SMeasurement> measurements;
    };

    explicit MeasurementTreeWidget(QWidget *parent = nullptr);
    void setDefaultGroupName(const QString &text);
    void setCalubrationView(bool val);
    void recalcMeans();

    void updateItem(QTreeWidgetItem *item, Measurement *m);

    QVector <double> getErrors();

    std::vector<SGroup> groups();

    QTreeWidgetItem *addMeasurement(Measurement *m);

signals:
    void deleteMeasurement(int index);
  private:
    virtual void  dropEvent(QDropEvent * event);

    QTreeWidgetItem *default_group_;
    Header* m_header;
    bool calibration_view_;
    QVector <double> errors_;


};

#endif // MEASUREMENTTREEWIDGET_H
