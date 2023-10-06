#include "analisyswidget.h"
#include "dataproviders/dataprovider.h"
#include "dataproviders/filedataprovider.h"
#include "QDebug"
//======================================================================
AnalisysWidget::AnalisysWidget(const QString &readable_name):
    readable_name_(readable_name)
{

}

//======================================================================
QString AnalisysWidget::readableName()
{
    return readable_name_;
}

//======================================================================
void AnalisysWidget::setDataProvider(DataProvider *p)
{
    if (provider_ != nullptr)
    {
        disconnect(provider_, &DataProvider::dataReadStarted, this, &AnalisysWidget::dataReadStarted);
        disconnect(provider_, &DataProvider::dataReadFinished, this, &AnalisysWidget::dataReadFinished);
        disconnect(provider_, &DataProvider::dataBatchReady, this, &AnalisysWidget::dataBatchReady);
        delete provider_;
    }

    provider_ = p;

    connect(p, &DataProvider::dataReadStarted, this, &AnalisysWidget::dataReadStarted);
    connect(p, &DataProvider::dataReadFinished, this, &AnalisysWidget::dataReadFinished);
    connect(p, &DataProvider::dataBatchReady, this, &AnalisysWidget::dataBatchReady);
}

//======================================================================
bool AnalisysWidget::obtainData(E_ANALISIS_TYPE /*type*/)
{
    provider()->run(getNewTrace());
    return true;
}

//======================================================================
DataProvider * AnalisysWidget::provider()
{
    return provider_;
}

//======================================================================
AnalisysWidget::~AnalisysWidget()
{
    qDebug() << "Destroy AnalisysWidget " ;//<< readableName() << provider()->readableName();
    delete provider_;
}
