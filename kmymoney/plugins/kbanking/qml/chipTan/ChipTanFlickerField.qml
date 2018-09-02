/*
 * A tan input dialog for optical chipTan used in online banking
 * Copyright 2014  Christian David <c.david@christian-david.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

import QtQuick 2.3
import "chipTan.js" as Logic

Rectangle {
    id: chipTanFlickerField

    /** You should set the width only, height is set accordingly */
    width: 260

    /** @brief Duration of each takt in ms */
    property int clockSetting: 50

    /** @brief suggested minimum width of this component */
    property int minimumWidth: 100

    /** @brief Data to transfer as HEX code in a string */
    property string transferData: ""

    readonly property bool stoppedState: true

    color: "black"

    height: 0.6*width
    radius: 0.1*width

    border {
        width: 0.01*width
        color: "red"
    }

    function toggleStart()
    {
        state = (state == "") ? "paused" : ""
    }


    Row {
        id: flickerFields
        // height is set so parent's borders never get overlapped
        height: parent.height-Math.ceil(2*parent.border.width)-1
        anchors.centerIn: parent

        property int fieldWidth: 0.15*parent.width
        spacing: 0.1*fieldWidth

        Repeater {
            id: flickerFieldsRepeater
            model: 5

            Rectangle {
                width: flickerFields.fieldWidth
                height: parent.height
                color: chipTanFlickerField.color

                property bool bitState: true

                Image {
                    visible: (index == 0 || index == 4)
                    source: "positionmarker.svg"
                    smooth: true
                    width: height
                    anchors {
                        top: parent.top
                        bottom: flickerBar.top
                        horizontalCenter: flickerBar.horizontalCenter
                    }
                }

                Rectangle {
                    id: flickerBar
                    width: flickerFields.fieldWidth

                    readonly property color colorOn: "white"
                    readonly property color colorOff: "black"

                    color: (bitState) ? colorOn : colorOff
                    height: 0.7*parent.height
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
    }

    Timer {
        id: timer
        interval: parent.clockSetting
        repeat: true
        running: true
        onTriggered: Logic.timerTriggered(flickerFields)
        onRunningChanged: Logic.timerStarted(chipTanFlickerField.transferData, flickerFields )
    }

    onTransferDataChanged: {
      // Restart timer to load new data to transfer into the script
      if ( timer.running == true )  {
        timer.stop()
        timer.start()
      }
    }

    states: [
        State {
            name: "paused"

            PropertyChanges {
                target: flickerFieldsRepeater.itemAt(0)
                bitState: stoppedState
            }

            PropertyChanges {
                target: flickerFieldsRepeater.itemAt(1)
                bitState: stoppedState
            }

            PropertyChanges {
                target: flickerFieldsRepeater.itemAt(2)
                bitState: stoppedState
            }

            PropertyChanges {
                target: flickerFieldsRepeater.itemAt(3)
                bitState: stoppedState
            }

            PropertyChanges {
                target: flickerFieldsRepeater.itemAt(4)
                bitState: stoppedState
            }

            PropertyChanges {
                target: timer
                running: false
            }
        }
    ]
}
