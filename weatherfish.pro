# The name of your app
TARGET = weatherfish

# C++ sources
SOURCES += main.cpp \
    citysearchmodel.cpp \
    abstractopenweathermodel.cpp \
    citymanager.cpp \
    city.cpp \
    flickrimageprovider.cpp \
#    hourweathermodel.cpp
    currentweathermodel.cpp

# C++ headers
HEADERS += \
    citysearchmodel.h \
    apikey.h \
    abstractopenweathermodel.h \
    citymanager.h \
    city.h \
    flickrimageprovider.h \
#    hourweathermodel.h
    currentweathermodel.h

# QML files and folders
qml.files = *.qml pages cover main.qml

# The .desktop file
desktop.files = weatherfish.desktop

# Please do not modify the following line.
include(sailfishapplication/sailfishapplication.pri)

# Icons
icons.files = icons/*.png
icons.path = $$DEPLOYMENT_PATH/icons
INSTALLS += icons

OTHER_FILES = \
    rpm/weatherfish.yaml \
    rpm/weatherfish.spec \
    pages/MainPage.qml \
    pages/AddCityDialog.qml
