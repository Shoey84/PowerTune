import QtQuick 2.8
import QtGraphicalEffects 1.0
Item {

    property int speed: 0

    height: (parent.width / 1.5)
    width: height
    x: (parent.width / 2) - (width / 2)
    y: (parent.height / 2) - (height / 2)

    Image {
         id: innerRingRect
         height: parent.height
         width: parent.width
         source: "/graphics/Tacho_Mitte.png"


         Text {
             id: speeddigit
             text: speed
             font.pixelSize: (parent.width / 3)
             font.bold: true
             font.family: "Eurostile"
             y: (parent.width / 3)
             color: "white"
             anchors.horizontalCenter: parent.horizontalCenter
         }

         DropShadow {
                 anchors.fill: speeddigit
                 horizontalOffset: 0
                 verticalOffset: 8
                 radius: 4.0
                 samples: 16
                 color: "black"
                 source: speeddigit
             }

         Text {
             text: "mph"
             font.pixelSize: (parent.width / 12)
              y: (parent.width / 5)
             font.bold: true
             font.family: "Eurostile"
             color: "white"
             anchors.horizontalCenter: parent.horizontalCenter
         }
    }
}
