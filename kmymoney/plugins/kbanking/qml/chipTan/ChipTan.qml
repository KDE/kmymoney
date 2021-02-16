/*
    A tan input dialog for optical chipTan used in online banking
    SPDX-FileCopyrightText: 2014 Christian David <c.david@christian-david.de>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

*/

import QtQuick 2.3

Rectangle {
    width: 360
    height: 300
    id: chipTan

    color: "#00FFFFFF"

    property string transferData: ""

    function setFlickerFieldWidth( width )
    {
      chipTanFlickerField.userWidth = width;
    }

    function flickerFieldWidth()
    {
      return chipTanFlickerField.width
    }

    function enlargeFlickerField()
    {
      chipTanFlickerField.userWidth = chipTanFlickerField.width+10
    }

    function reduceFlickerField()
    {
      chipTanFlickerField.userWidth = chipTanFlickerField.width-10
    }

    function accelerateTransmission()
    {
      chipTanFlickerField.userClockSetting =  chipTanFlickerField.clockSetting - 20
    }

    function decelerateTransmission()
    {
      chipTanFlickerField.userClockSetting =  chipTanFlickerField.clockSetting + 20
    }

    function setFlickerClockSetting( clockSetting )
    {
      chipTanFlickerField.userClockSetting = clockSetting;
    }

    signal flickerFieldWidthChanged( int width );
    signal flickerFieldClockSettingChanged( int clockSetting );

    /*
    // Toolbar
    Rectangle {
        id: toolbar

        anchors {
            margins: 10
            left: parent.left
        }

        width: buttonRow.width+10
        height: buttonRow.height+10

        // Toolbar
        Row {
            id: buttonRow
            spacing: 5

            anchors {
                verticalCenter: parent.verticalCenter
                left: parent.left
            }

            Image {
                source: "/usr/share/icons/default.kde4/22x22/actions/zoom-in.png"
                MouseArea {
                    anchors.fill: parent
                    onClicked: chipTanFlickerField.userWidth = chipTanFlickerField.width+10
                }
            }

            Image {
                source: "/usr/share/icons/default.kde4/22x22/actions/zoom-out.png"
                MouseArea {
                    anchors.fill: parent
                    onClicked: chipTanFlickerField.userWidth = chipTanFlickerField.width-10
                }
            }

            Image {
                source: "/usr/share/icons/default.kde4/22x22/actions/media-playback-start.png"
                MouseArea {
                    anchors.fill: parent
                    onClicked: chipTanFlickerField.userTaktLength =  chipTanFlickerField.taktLength + 20
                }
            }

            Image {
                source: "/usr/share/icons/default.kde4/22x22/actions/media-seek-forward.png"
                MouseArea {
                    anchors.fill: parent
                    onClicked: chipTanFlickerField.userTaktLength =  chipTanFlickerField.taktLength - 20
                }
            }
        }
    }
    */
    ChipTanFlickerField {
        id: chipTanFlickerField
        anchors {
            left: parent.left
            verticalCenter: parent.verticalCenter
        }

        transferData: chipTan.transferData

        property int userWidth: parent.width
        width: Math.max( userWidth, minimumWidth )
        onWidthChanged: parent.flickerFieldWidthChanged(width);

        property int userClockSetting: 100
        clockSetting: Math.min(Math.max(10, userClockSetting), 2000)
        onClockSettingChanged: parent.flickerFieldClockSettingChanged(clockSetting);

        MouseArea {
            id: resizeMouseArea
            anchors.fill: parent
        }

        states:
            State {
                name: "resizeWidth"
                when: resizeMouseArea.pressed
                extend: "paused"

                StateChangeScript {
                    script: {
                        resizePropertyChanger.oldWidth = chipTanFlickerField.width
                        resizePropertyChanger.mouseStartPosition = resizeMouseArea.mouseX
                    }
                }

                PropertyChanges {
                    id: resizePropertyChanger
                    target: chipTanFlickerField
                    restoreEntryValues: false
                    property real oldWidth: 0
                    property real mouseStartPosition: 0

                    userWidth: resizePropertyChanger.oldWidth + resizeMouseArea.mouseX - resizePropertyChanger.mouseStartPosition
                }
            }
    }
}
