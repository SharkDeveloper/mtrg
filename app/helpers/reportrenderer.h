#ifndef REPORTRENDERER_H
#define REPORTRENDERER_H

#include <QString>
#include <QPixmap>
#include "../inja/include/inja/inja.hpp"
class ReportRenderer
{
public:
    ReportRenderer(QWidget *parent, QString report_template_doc, QString report_save_doc, QString ref, QVector<QPixmap> pix, inja::json report_data);
};

#endif // REPORTRENDERER_H
