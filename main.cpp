
#include <QtGui/QGuiApplication>
#include <QtQuick/QQuickView>
#include <QtQml/qqml.h>

#include "sailfishapplication.h"
#include "citysearchmodel.h"
#include "citymanager.h"
#include "city.h"
#include "flickrimageprovider.h"
#include "currentweathermodel.h"

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QScopedPointer<QGuiApplication> app(Sailfish::createApplication(argc, argv));
    app.data()->setOrganizationName("SfietKonstantin");
    app.data()->setApplicationName("weatherfish");

    qmlRegisterType<CitySearchModel>("org.SfietKonstantin.weatherfish", 1, 0, "CitySearchModel");
    qmlRegisterType<CityManager>("org.SfietKonstantin.weatherfish", 1, 0, "CityManager");
    qmlRegisterType<City>("org.SfietKonstantin.weatherfish", 1, 0, "City");
    qmlRegisterType<FlickrImageProvider>("org.SfietKonstantin.weatherfish", 1, 0,
                                         "FlickrImageProvider");
    qmlRegisterType<CurrentWeatherModel>("org.SfietKonstantin.weatherfish", 1, 0, "CurrentWeather");

    QScopedPointer<QQuickView> view(Sailfish::createView("main.qml"));
    
    Sailfish::showView(view.data());
    
    return app->exec();
}


