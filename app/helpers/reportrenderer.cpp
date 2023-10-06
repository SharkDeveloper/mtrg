#include "reportrenderer.h"
#include <QTemporaryDir>

#include <QDebug>
#include <QBuffer>
#include <QTemporaryFile>

#include "../inja/include/inja/inja.hpp"
#include <QMessageBox>
#include <QProcess>

using namespace inja;

ReportRenderer::ReportRenderer(QWidget *parent, QString report_template_doc, QString report_save_doc, QString ref, QVector<QPixmap> pix, inja::json report_data)
{

    try
    {
        QFile template_file(report_template_doc);
        template_file.open(QIODevice::ReadOnly);
        QByteArray ba = template_file.readAll();
        template_file.close();

        for (auto p: pix)
        {
            QBuffer buffer;
            buffer.open(QIODevice::WriteOnly);
            p.save(&buffer, "PNG");
            auto const encoded = buffer.data().toBase64();
            report_data["imgs"].push_back(encoded);
        }

        std::string rendered = render(ba.toStdString(), report_data);

        QTemporaryFile report_rendered_md;

        report_rendered_md.open();
        report_rendered_md.write(QByteArray::fromStdString(rendered));
        report_rendered_md.close();

        QFile rmd(report_save_doc+".md");
        rmd.open(QIODevice::WriteOnly);
        rmd.write(QByteArray::fromStdString(rendered));
        rmd.close();

        QProcess process;
        QStringList params;
        params << report_rendered_md.fileName() << "-o" << report_save_doc;

        if (!ref.isEmpty())
            params << "--reference-doc" << ref;
        params << "--css" <<"reports/style.css";
        params << "-s";

        qDebug() << params;
        process.start("pandoc", params);

        while(!process.waitForFinished());

        if (process.exitCode() != 0)
        {
            QMessageBox::warning(parent,
                                        "Ошибка",
                                        "Что то пошло не так при конвертации отчета\nПроверьте, что установлен pandoc ");
            return;

        }

    }
    catch (inja::InjaError e) {
        QMessageBox::warning(parent,
                                    "Ошибка",
                                    "Что то пошло не так при генерации отчета. Что именно:\n" +
                                    QString::fromStdString(e.what()) +
                                    "\n Проверьте шаблон отчета " +
                                    report_template_doc);
        qDebug() << e.what();
        return;
    }
    catch (std::exception e){
        qDebug() << e.what() ;
        QMessageBox::warning(parent,
                                    "Ошибка",
                                    "Что то пошло не так при генерации отчета. Что именно:\n" +
                                    QString::fromStdString(e.what()) +
                                    "\n Проверьте шаблон отчета " +
                                    report_template_doc);
        return;
    }
    QMessageBox::information(parent, "Отчет сохранен", "Успешно!");
}
