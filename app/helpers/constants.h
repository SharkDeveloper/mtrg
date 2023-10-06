#ifndef CONSTANTS_H
#define CONSTANTS_H
#include <QString>
#include <QFont>
#include <QPen>
#include <QFontDatabase>


const QString kUnknownValue  = "неизвестно";
const QString kAreYouShureString = "Вы уверены?";

const QFont kFontFixed("Monospace");// = QFontDatabase::systemFont(QFontDatabase::FixedFont);
//QFont kFontFixed("Monospace",;


const QPen kPenDefault = QPen(QBrush(Qt::black),0);
const QPen kPenGraphSelected = QPen(QBrush(Qt::black), 1);
const QPen kPenActive = QPen(QBrush(Qt::green),0);
const QPen kPenSelection = QPen(QBrush(Qt::red),1);

const QPen kPenMarker = QPen(QBrush(Qt::magenta),1, Qt::DashLine);

const QPen kPenDefaultBlue = QPen(QBrush(Qt::blue),0);


#endif // CONSTANTS_H
