import QtQuick 2.8
import QtQuick.Extras 1.4
import QtQuick.Controls 2.1
import QtQuick.Controls.Styles 1.4


Item {

    width: parent.width
    height:parent.height
    property  var unit : Dashboard.units;
    Component.onCompleted: {units.unitadjust()} // adjusts the Gauges to metric or imperial



    Rectangle {
        id:racedashwindow
        width: parent.width
        height:parent.height
        color:"grey"
        Gauge {
            id: gauge
            height: parent.height
            width: parent.width /1.025
            y:0
            minorTickmarkCount: 0
            tickmarkStepSize : 9000
            //labelStepSize: 9000
            orientation : Qt.Horizontal
            minimumValue: 0
            maximumValue: 9000

            value: Dashboard.revs
            Behavior on value {
                NumberAnimation {
                    duration: 5
                }
            }
            style: GaugeStyle {
                valueBar: Rectangle {
                    width:  400
                    color: Qt.rgba(gauge.value / gauge.maximumValue, 0, 1 - gauge.value / gauge.maximumValue, 1)
                }
            }
        }

        Image {
            height: parent.height
            width: parent.width
            source: "/graphics/Racedash800x480.png"

        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter

            spacing: 20
            StatusIndicator {
                color: "green"
                active: { (Dashboard.revs > "3000") ? true: false; }
            }
            StatusIndicator {
                color: "green"
                active: { (Dashboard.revs > "4000") ? true: false; }
            }
            StatusIndicator {
                color: "yellow"
                active: { (Dashboard.revs > "5000") ? true: false; }
            }
            StatusIndicator {
                color: "yellow"
                active: { (Dashboard.revs > "6000") ? true: false; }
            }
            StatusIndicator {
                color: "red"
                active: { (Dashboard.revs > "7900") ? true: false; }
            }
            StatusIndicator {
                color: "red"
                active: { (Dashboard.revs > "7900") ? true: false; }
            }
            StatusIndicator {
                color: "yellow"
                active: { (Dashboard.revs > "6000") ? true: false; }
            }
            StatusIndicator {
                color: "yellow"
                active: { (Dashboard.revs > "5000") ? true: false; }
            }
            StatusIndicator {
                color: "green"
                active: { (Dashboard.revs > "4000") ? true: false; }
            }
            StatusIndicator {
                color: "green"
                active: { (Dashboard.revs > "3000") ? true: false; }
            }
        }

        Text {
            text:"0"
            font.pixelSize: racedashwindow.width /40
            y: 220
            x: 10
            font.bold: true
            font.family: "Eurostile"
            color: "grey"

        }
        Text {
            text:"1"
            font.pixelSize: racedashwindow.width /40
            y: 220
            x: 80
            font.bold: true
            font.family: "Eurostile"
            color: "grey"

        }
        Text {
            text:"2"
            font.pixelSize: racedashwindow.width /40
            y: 160
            x: 165
            font.bold: true
            font.family: "Eurostile"
            color: "grey"

        }
        Text {
            text:"3"
            font.pixelSize: racedashwindow.width /40
            y: 120
            x: 250
            font.bold: true
            font.family: "Eurostile"
            color: "grey"

        }
        Text {
            text:"4"
            font.pixelSize: racedashwindow.width /40
            y: 120
            x: 335
            font.bold: true
            font.family: "Eurostile"
            color: "grey"

        }
        Text {
            text:"5"
            font.pixelSize: racedashwindow.width /40
            y: 120
            x: 420
            font.bold: true
            font.family: "Eurostile"
            color: "grey"

        }
        Text {
            text:"6"
            font.pixelSize: racedashwindow.width /40
            y: 120
            x: 505
            font.bold: true
            font.family: "Eurostile"
            color: "grey"

        }
        Text {
            text:"7"
            font.pixelSize: racedashwindow.width /40
            y: 120
            x: 590
            font.bold: true
            font.family: "Eurostile"
            color: "grey"

        }
        Text {
            text:"8"
            font.pixelSize: racedashwindow.width /40
            y: 120
            x: 675
            font.bold: true
            font.family: "Eurostile"
            color: "grey"

        }
        Text {
            text:"9"
            font.pixelSize: racedashwindow.width /40
            y: 120
            x: 760
            font.bold: true
            font.family: "Eurostile"
            color: "grey"

        }
        Text {
            text:"RPM"
            font.pixelSize: 20
            y: 220
            x: 180
            font.bold: true
            font.family: "Eurostile"
            color: "grey"

        }
        Text {
            text: (Dashboard.revs)
            font.pixelSize: 100
            y: 140
            x: 240
        font.italic: true
            font.bold: true
            font.family: "Eurostile"
            color: "white"

        }
        Text {
            id :speed
            font.pixelSize: 20
            y: 170
            x: 550
            font.bold: true
            font.family: "Eurostile"
            color: "grey"

        }
        Text {
            text: (Dashboard.speed)
            font.pixelSize: 50
            y: 145
            x: 620
        font.italic: true
            font.bold: true
            font.family: "Eurostile"
            color: "white"

        }
    Text {
            id:boost
            //text:"Boost [mmHg/bar]"
            font.pixelSize: 20
            y: 280
            x: 310
            font.bold: true
            font.family: "Eurostile"
            color: "grey"

        }
        Text {
            text: (Dashboard.pim).toFixed(1)
            font.pixelSize: 50
            y: 305
            x: 310
        font.italic: true
            font.bold: true
            font.family: "Eurostile"
            color: "white"

        }
        Text {
            text:"Knock"
            font.pixelSize: 18
            y: 370
            x: 310
            font.bold: true
            font.family: "Eurostile"
            color: "grey"

        }
        Text {
            text: (Dashboard.Knock).toFixed(0)
            font.pixelSize: 50
            y: 395
            x: 310
        font.italic: true
            font.bold: true
            font.family: "Eurostile"
            color: "white"

        }
        Text {
            id:air
            font.pixelSize: 18
            y: 370
            x: 550
            font.bold: true
            font.family: "Eurostile"
            color: "grey"

        }
        Text {
            text: (Dashboard.Intaketemp).toFixed(1)
            font.pixelSize: 50
            y: 395
            x: 550
        font.italic: true
            font.bold: true
            font.family: "Eurostile"
            color: "white"

        }
        Text {
            id: water
            font.pixelSize: 18
            y: 280
            x: 550
            font.bold: true
            font.family: "Eurostile"
            color: "grey"

        }
        Text {
            text: (Dashboard.Watertemp).toFixed(1)
            font.pixelSize: 50
            y: 305
            x: 550
        font.italic: true
            font.bold: true
            font.family: "Eurostile"
            color: "white"

        }
        Text {
            text:"AFR"
            font.pixelSize: 18
            y: 230
            x: 550
            font.bold: true
            font.family: "Eurostile"
            color: "grey"

        }
        Text {
            text: (Dashboard.auxcalc1).toFixed(1)
            font.pixelSize: 50
            y: 205
            x: 620
        font.italic: true
            font.bold: true
            font.family: "Eurostile"
            color: "white"

        }

    // Sensor Status ON/OFF

        Grid {
        anchors.left: parent.left
        anchors.leftMargin: 50
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 30
            rows: 8
            columns: 6
            spacing: 3

            //ROW 1
            Text {
                font.pixelSize: 15
                color:"white"
                text: qsTr(Dashboard.SensorString1)

            }
            Text {
                font.pixelSize: 15
                color:{ (Dashboard.sens1 > "5") ? "red": "yellow"; }
                text: (Dashboard.sens1).toFixed(2) + " V   "
            }

            Text {
                font.pixelSize: 15
                color:"white"
                text: qsTr(Dashboard.FlagString1)
            }

            StatusIndicator {
                height: 15
                width: height
                color: "green"
                active: { (Dashboard.Flag1 == "1") ? true: false; }
            }

            Text {
                font.pixelSize: 15
                color:"white"
                text: qsTr(Dashboard.FlagString2)
            }
            StatusIndicator {
                height: 15
                width: height
                color: "green"
                active: { (Dashboard.Flag2 == "1") ? true: false; }
            }

            // ROW 2

            Text {
                font.pixelSize: 15
                color:"white"
                text: qsTr(Dashboard.SensorString2)
            }
            Text {
                font.pixelSize: 15
                color:{ (Dashboard.sens2 > "5") ? "red": "yellow"; }
                text: (Dashboard.sens2).toFixed(2) + " V   "
            }

            Text {
                font.pixelSize: 15
                color:"white"
                text: qsTr(Dashboard.FlagString3)
            }
            StatusIndicator {
                height: 15
                width: height
                color: "green"
                active: { (Dashboard.Flag3 == "1") ? true: false; }

            }

            Text {
                font.pixelSize: 15
                color:"white"
                text: qsTr(Dashboard.FlagString4)
            }
            StatusIndicator {
                height: 15
                width: height
                color: "green"
                active: { (Dashboard.Flag4 == "1") ? true: false; }
            }
            // ROW 3

            Text {
                font.pixelSize: 15
                color:"white"
                text: qsTr(Dashboard.SensorString3)
            }
            Text {
                font.pixelSize: 15
                color:{ (Dashboard.sens3 > "5") ? "red": "yellow"; }
                text: (Dashboard.sens3).toFixed(2) + " V   "
            }
            Text {
                font.pixelSize: 15
                color:"white"
                text: qsTr(Dashboard.FlagString5)
            }
            StatusIndicator {
                height: 15
                width: height
                color: "green"
                active: { (Dashboard.Flag5 == "1") ? true: false; }
            }

            Text {
                font.pixelSize: 15
                color:"white"
                text: qsTr(Dashboard.FlagString6)
            }
            StatusIndicator {
                height: 15
                width: height
                color: "green"
                active: { (Dashboard.Flag6 == "1") ? true: false; }

            }

            // ROW 4

            Text {
                font.pixelSize: 15
                color:"white"
                text: qsTr(Dashboard.SensorString4)
            }
            Text {
                font.pixelSize: 15
                color:{ (Dashboard.sens4 > "5") ? "red": "yellow"; }
                text: (Dashboard.sens4).toFixed(2) + " V   "
            }

            Text {
                font.pixelSize: 15
                color:"white"
                text: qsTr(Dashboard.FlagString7)
            }
            StatusIndicator {
                height: 15
                width: height
                color: "green"
                active: { (Dashboard.Flag7 == "1") ? true: false; }
            }

            Text {
                font.pixelSize: 15
                color:"white"
                text: qsTr(Dashboard.FlagString8)
            }
            StatusIndicator {
                height: 15
                width: height
                color: "green"
                active: { (Dashboard.Flag8 == "1") ? true: false; }
            }
            // ROW 5

            Text {
                font.pixelSize: 15
                color:"white"
                text: qsTr(Dashboard.SensorString5)
            }
            Text {
                font.pixelSize: 15
                color:{ (Dashboard.sens5 > "5") ? "red": "yellow"; }
                text: (Dashboard.sens5).toFixed(2) + " V   "
            }

            Text {
                font.pixelSize: 15
                color:"white"
                text: qsTr(Dashboard.FlagString9)
            }
            StatusIndicator {
                height: 15
                width: height
                color: "green"
                active: { (Dashboard.Flag9 == "1") ? true: false; }
            }


            Text {
                font.pixelSize: 15
                color:"white"
                text: qsTr(Dashboard.FlagString10)
            }
            StatusIndicator {
                height: 15
                width: height
                color: "green"
                active: { (Dashboard.Flag10 == "1") ? true: false; }
            }

            // ROW 6


            Text {
                font.pixelSize: 15
                color:"white"
                text: qsTr(Dashboard.SensorString6)
            }
            Text {
                font.pixelSize: 15
                color:{ (Dashboard.sens6 > "5") ? "red": "yellow"; }
                text: (Dashboard.sens6).toFixed(2) + " V   "
            }

            Text {
                font.pixelSize: 15
                color:"white"
                text: qsTr(Dashboard.FlagString11)
            }
            StatusIndicator {
                height: 15
                width: height
                color: "green"
                active: { (Dashboard.Flag11 == "1") ? true: false; }
            }


            Text {
                font.pixelSize: 15
                color:"white"
                text: qsTr(Dashboard.FlagString12)
            }
            StatusIndicator {
                height: 15
                width: height
                color: "green"
                active: { (Dashboard.Flag12 == "1") ? true: false; }
            }

            // ROW 7


            Text {
                font.pixelSize: 15
                color:"white"
                text: qsTr(Dashboard.SensorString7)
            }
            Text {
                font.pixelSize: 15
                color:{ (Dashboard.sens7 > "5") ? "red": "yellow"; }
                text: (Dashboard.sens7).toFixed(2) + " V   "
            }

            Text {
                font.pixelSize: 15
                color:"white"
                text: qsTr(Dashboard.FlagString13)
            }
            StatusIndicator {
                height: 15
                width: height
                color: "green"
                active: { (Dashboard.Flag13 == "1") ? true: false; }
            }



            Text {
                font.pixelSize: 15
                color:"white"
                text: qsTr(Dashboard.FlagString14)
            }
            StatusIndicator {
                height: 15
                width: height
                color: "green"
                active: { (Dashboard.Flag14 == "1") ? true: false; }
            }


            // ROW 8

            Text {
                font.pixelSize: 15
                color:"white"
                text: qsTr(Dashboard.SensorString8)
            }
            Text {
                font.pixelSize: 15
                color:{ (Dashboard.sens8 > "5") ? "red": "yellow"; }
                text: (Dashboard.sens8).toFixed(2) + " V   "
            }


            Text {
                font.pixelSize: 15
                color:"white"
                text: qsTr(Dashboard.FlagString15)
            }
            StatusIndicator {
                height: 15
                width: height
                color: "green"
                active: { (Dashboard.Flag15 == "1") ? true: false; }
            }



            Text {
                font.pixelSize: 15
                color:"white"
                text: qsTr(Dashboard.FlagString16)
            }
            StatusIndicator {
                height: 15
                width: height
                color: "green"
                active: { (Dashboard.Flag16 == "1") ? true: false; }

            }


        }
    }
    Item {
        id: units
        function unitadjust()
        {
            if (unit == "imperial") {boost.text = "Boost [inHg/PSI]",speed.text= "MP/H",air.text = "Air Temp [°F]",water.text = "Water Temp [°F]"};
            if (unit == "metric") {boost.text ="Boost [mmHg/kgcm2]",speed.text= "Km/H",air.text = "Air Temp [°C]",water.text = "Water Temp [°C]"};

        }


    }
}
