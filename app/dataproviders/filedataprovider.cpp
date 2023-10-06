#include "filedataprovider.h"
#include <QDebug>
#include <QFile>
#include <QThread>
#include <QFileInfo>
#include "traces/singlepointprocessors.h"

#ifdef IGES
#include "IGESControl_Reader.hxx"
#include "IGESControl_Reader.hxx"
#include <BRepTools.hxx>
#include "TColStd_HSequenceOfTransient.hxx"
#include "TopoDS_Shape.hxx"
#include "IGESControl_Reader.hxx"
#include "TColStd_HSequenceOfTransient.hxx"
#include "TopoDS_Shape.hxx"
#include <AIS_Shape.hxx>
#include <AIS_InteractiveObject.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <IGESControl_Reader.hxx>
#include <IGESControl_Writer.hxx>
#include <IGESControl_Controller.hxx>
#include <STEPControl_Reader.hxx>
#include <STEPControl_Writer.hxx>
#include <STEPControl_StepModelType.hxx>
#include <Interface_Static.hxx>
//#include <Interface_TraceFile.hxx>

#include <StlAPI_Writer.hxx>
#include <VrmlAPI_Writer.hxx>


#include <BRep_Tool.hxx>
#include <BRep_Builder.hxx>

#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Compound.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_HSequenceOfShape.hxx>

#include <Geom_Line.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Plane.hxx>
#include <Geom_Surface.hxx>


#include <Standard_ErrorHandler.hxx>
#include <Standard_CString.hxx>

#endif
const int kBatchNotificationSize = 500;

//======================================================================
FileDataProvider::FileDataProvider(const QString &file):
    DataProvider(file),
    filename(file)
{

}

//======================================================================
void FileDataProvider::prepareTrace()
{
    getTrace()->setSourceName(QFileInfo(filename).fileName());
    getTrace()->clearPreProcessors();
}

bool FileDataProvider::importContourFile(QString filename)
{
    getTrace()->addSinglePointProcessor(new YFromAnglePointProcessor());
    getTrace()->addSinglePointProcessor(new MultiplicationProcessor(1.0e-3, {0, 1})); //convert nm to mm

    QFile f(filename);

    if (f.open(QFile::ReadOnly) && !f.isOpen())
    {
        qDebug() << "Can't open file " << filename;
        emit dataReadError("Can't open file ");
        return false;
    }

    //read all data by defined coluns (in data it already set)
    int line_readed = 0;
    while (!f.atEnd())
    {
        QByteArray bytes = f.readLine();
        QTextStream sstream(bytes);

        QVector <double> data;
        for(int i = 0; i < getTrace()->getDimensions(); i++)
        {
            double d;
            sstream >> d;
            if (sstream.status() > QTextStream::ReadPastEnd)
            {
                emit dataReadError("Error in " + filename);
                return false;
            }

            data.push_back(d);
        }

        getTrace()->writeData(data);

        line_readed ++;
        if (line_readed % kBatchNotificationSize == 0){ emit dataBatchReady(line_readed-kBatchNotificationSize, kBatchNotificationSize); }
    }
    return true;
}

bool FileDataProvider::importProfileFile(QString filename)
{

    getTrace()->addSinglePointProcessor(new NormalisationProcessor(0, 3.6, {1,2})); //convert raw value to a um


    QFile f(filename);

    if (f.open(QFile::ReadOnly) && !f.isOpen())
    {
        qDebug() << "Can't open file " << filename;
        emit dataReadError("Can't open file ");
        return false;
    }

    //read all data by defined coluns (in data it already set)
    int line_readed = 0;
    while (!f.atEnd())
    {
        QByteArray bytes = f.readLine();
        QTextStream sstream(bytes);

        QVector <double> data;
        for(int i = 0; i < getTrace()->getDimensions(); i++)
        {
            double d;
            sstream >> d;
            if (sstream.status() > QTextStream::ReadPastEnd)
            {
                emit dataReadError("Error in " + filename);
                return false;
            }

            data.push_back(d);
        }

        getTrace()->writeData(data);

        line_readed ++;
        if (line_readed % kBatchNotificationSize == 0){ emit dataBatchReady(line_readed-kBatchNotificationSize, kBatchNotificationSize); }
    }
    return true;
}


#ifdef IGES
Handle(TopTools_HSequenceOfShape) importIGES( std::string file )
{
    Handle(TopTools_HSequenceOfShape) aSequence;
    TCollection_AsciiString  aFilePath = file.c_str();

    IGESControl_Reader Reader;
    int status = Reader.ReadFile(aFilePath.ToCString() );

    if ( status == IFSelect_RetDone )
    {
        aSequence = new TopTools_HSequenceOfShape();
        Reader.TransferRoots();
        TopoDS_Shape aShape = Reader.OneShape();
        aSequence->Append( aShape );
    }
    return aSequence;

}


bool FileDataProvider::importIGSFile(QString filename)
{
    Handle(TopTools_HSequenceOfShape) seq = importIGES(filename.toStdString());
    TopoDS_Shape shape = seq->First();
    TopAbs_ShapeEnum shapetype = shape.ShapeType();

    //support only curve (Wire or Edge) shapes....
    if (shape.ShapeType() != TopAbs_WIRE && shape.ShapeType() != TopAbs_EDGE)
    {
        emit dataReadError("Data not Wire Or Edge");
        return false;
    }

    //
    int pts_processed = 0;

    auto process_edge = [&](const TopoDS_Edge &edge)
    {
        double f,l;

        Handle(Geom_Curve) curve = BRep_Tool::Curve(edge,f,l);

        for (double v = f; v < l; v+=0.1)
        {
           QVector <double> data;
           gp_Pnt p = curve->Value(v);

           double x = p.X();
           double y = p.Y();
           double z = p.Z();
           data.push_back( y);
           data.push_back( x);
           data.push_back( z);

           getTrace()->writeData(data);

           pts_processed ++;

           if (pts_processed % kBatchNotificationSize == 0){ emit dataBatchReady(pts_processed-kBatchNotificationSize, kBatchNotificationSize); }
         }
    };

    if (shape.ShapeType() == TopAbs_WIRE)
    {
        BRepTools_WireExplorer wire_explorer(TopoDS::Wire(shape));
        while(wire_explorer.More())
        {
            process_edge(wire_explorer.Current());
            wire_explorer.Next();
        }
    }
    else
    {
        process_edge(TopoDS::Edge(shape));
    }

    return true;

}
#endif

//======================================================================
bool FileDataProvider::obtainData()
{

    qDebug() << "FileDataProvider obtain data from "<<filename;
    #ifdef IGES
    if(filename.endsWith(".igs"))
    {
        return importIGSFile(filename);
    }
    else
    #endif
    if(filename.endsWith(".prf130"))
    {
        return importProfileFile(filename);
    }
    else
    {
        return importContourFile(filename);
    }
    return false;
}
