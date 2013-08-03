import QtQuick 2.0
import Sailfish.Silica 1.0
import Sailfish.Silica.theme 1.0
import org.SfietKonstantin.weatherfish 1.0


Page {
    id: page
    onStatusChanged: cityManager.pushAttached()
    
    SilicaFlickable {
        id: emptyStatePage
        anchors.fill: parent
        visible: cityManager.cities.length == 0

        PageHeader {
            visible: !cityManager.attachedPushed
            title: qsTr("Weather-fish")
        }

        ViewPlaceholder {
            enabled: true
            text: qsTr("There is no city to display\nAdd one via the pulley-menu")
        }

        PullDownMenu {
            id: menu
            MenuItem {
                text: qsTr("Add city")
                onClicked: pageStack.push(Qt.resolvedUrl("AddCityDialog.qml"),
                                          {"cityManager": cityManager})
            }
        }
    }

    CityManager {
        id: cityManager
        property bool attachedPushed: false
        property Page pushedPage

        function pushAttached() {
            if (page.status != PageStatus.Active) {
                return
            }

            if (!attachedPushed && cities.length > 0) {
                pushedPage = pageStack.pushAttached(attachedPage, {"model": cities})
                attachedPushed = true
                cityPage.city = cities[0]
                cityPage.visible = true
            } else if (attachedPushed && cities.length == 0) {
                attachedPushed = false
            }
        }

        Component.onCompleted: pushAttached()
        onCitiesChanged: pushAttached()

    }

    AttachedPage {
        id: attachedPage
        model: cityManager.cities
        currentCity: cityPage.city
        cityManager: cityManager
        Connections {
            target: cityManager
            onCitiesChanged: attachedPage.model = cityManager.cities
        }
    }

    Connections {
        target: cityManager.pushedPage
        onChangeCity: cityPage.city = city
    }

    CityPage {
        id: cityPage
        anchors.fill: parent
        onCityChanged: console.debug(city)
    }
}


