#ifndef DATATYPES_H
#define DATATYPES_H
#include <QVector>

typedef QVector <QVector<double>> TData;

enum E_ANALISIS_TYPE {
    CONTOUR,
    PROFILE,
    CIRCULAR
};

enum E_UNIT {MM, NM, MK};


#endif // DATATYPES_H
