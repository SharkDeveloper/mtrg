#include "mainwindow.h"

#include <QApplication>
#include "helpers/constants.h"
#include <tuple>
#include <QDebug>


int main(int argc, char *argv[])
{



    QString req = "GET /?method=GetWallets&id=0xD29E866D8C7157C56006C74D002AB75EBBA5784ADD3038BC74200675017CE014\n... ... ... ";
    QString http_method = req.split("\n")[0].split(" ")[0];
    QString http_url = req.split("\n")[0].split(" ")[1];

    QUrlQuery query(QUrl(req).query());

    qDebug()<<query.queryItemValue("method");
    qDebug()<<query.queryItemValue("id");
    qDebug()<<query.queryItemValue("walletName");

    QApplication a(argc, argv);
    MainWindow w;
    w.showMaximized();

    return a.exec();
}
