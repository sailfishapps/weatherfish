import QtQuick 2.0
import Sailfish.Silica 1.0
import org.SfietKonstantin.weatherfish 1.0

Page {
    id: container
    property CityManager cityManager
    property alias model: view.model
    property City currentCity
    signal changeCity(City city)

    SilicaListView {
        id: view
        anchors.fill: parent
        property Item contextMenu
        property string identifierToDelete
        header: PageHeader {
            title: qsTr("Cities")
        }

        delegate: Item {
            id: item
            property bool menuOpen: view.contextMenu != null && view.contextMenu.parent === item
            width: view.width
            height: menuOpen ? contentItem.height + view.contextMenu.height
                             : contentItem.height
            BackgroundItem {
                id: contentItem

                GlassItem {
                    id: decorator
                    anchors {
                        left: parent.left
                        leftMargin: Theme.paddingMedium
                        verticalCenter: parent.verticalCenter
                    }
                    dimmed: model.modelData != container.currentCity
                }

                Label {
                    anchors {
                        left: decorator.right
                        leftMargin: Theme.paddingMedium
                        right: parent.right
                        rightMargin: Theme.paddingMedium
                        verticalCenter: parent.verticalCenter
                    }
                    text: model.modelData.name
                }
                onClicked: {
                    container.changeCity(model.modelData)
                    pageStack.navigateBack()
                }

                onPressAndHold: {
                    if (!view.contextMenu) {
                        view.contextMenu = contextMenuComponent.createObject(view)
                    }
                    view.contextMenu.show(item)
                    view.identifierToDelete = model.modelData.identifier
                }
            }
        }

        PullDownMenu {
            id: menu
            MenuItem {
                text: qsTr("Add city")
                onClicked: pageStack.push(Qt.resolvedUrl("AddCityDialog.qml"),
                                          {"cityManager": cityManager})
            }
        }

        Component {
            id: contextMenuComponent
            ContextMenu {
                MenuItem {
                    text: qsTr("Delete")
                    onClicked: {
                        if (view.identifierToDelete != "") {
                            var currentCityId = container.currentCity.identifier
                            var selectedRemoved = (currentCity.identifier == currentCityId)
                            cityManager.removeCity(view.identifierToDelete)
                            if (selectedRemoved) {
                                if (cityManager.cities.length > 0) {
                                    container.changeCity(cityManager.cities[0])
                                } else {
                                    container.changeCity(null)
                                    pageStack.pop()
                                }
                            }

                            view.identifierToDelete = ""
                        }
                    }
                }
            }
        }
    }
}
