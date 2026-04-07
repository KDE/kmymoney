import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Rectangle {
    id: root
    color: Kirigami.Theme.backgroundColor

    Kirigami.ScrollablePage {
        id: page
        anchors.fill: parent
        title: i18n("Financial Summary")

        Kirigami.CardsListView {
            id: sectionsList
            anchors.fill: parent
            model: homeModel
            visible: homeModel && homeModel.isReady && homeModel.rowCount() > 0
            spacing: Kirigami.Units.largeSpacing

            topMargin: Kirigami.Units.gridUnit
            bottomMargin: Kirigami.Units.gridUnit
            leftMargin: Kirigami.Units.gridUnit
            rightMargin: Kirigami.Units.gridUnit

            delegate: Kirigami.AbstractCard {
                id: sectionCard
                implicitWidth: sectionsList.width - Kirigami.Units.gridUnit * 2
                
                contentItem: ColumnLayout {
                    spacing: 0
                    
                    Kirigami.Heading {
                        text: (model && model.title) ? model.title : ""
                        level: 2
                        Layout.fillWidth: true
                        Layout.margins: Kirigami.Units.smallSpacing
                    }

                    Kirigami.Separator {
                        Layout.fillWidth: true
                    }

                    Loader {
                        id: sectionLoader
                        Layout.fillWidth: true
                        property var sectionData: (model && model.sectionObject) ? model.sectionObject : null
                        sourceComponent: {
                            if (!model) return null;
                            switch(model.type) {
                                case 0: return accountsComp; 
                                case 1: return schedulesComp;
                                case 4: return assetsLiabilitiesComp;
                                default: return null;
                            }
                        }
                    }
                }
            }
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (Kirigami.Units.gridUnit * 4)
            visible: homeModel && (!homeModel.isReady || homeModel.rowCount() === 0)
            text: (homeModel && !homeModel.isReady) ? i18n("Loading financial data...") : i18n("No data to display. Please check your Home view settings.")
            icon.name: (homeModel && !homeModel.isReady) ? "view-refresh" : "view-list-icons"
        }
    }

    // --- REUSABLE ACCOUNT LIST ITEM (Simple) ---
    Component {
        id: accountRowComp
        QQC2.ItemDelegate {
            id: rowDelegate
            width: parent ? parent.width : 0
            padding: Kirigami.Units.smallSpacing
            background: Rectangle {
                color: index % 2 === 0 ? "transparent" : Kirigami.Theme.alternateBackgroundColor
                opacity: 0.3
            }
            contentItem: RowLayout {
                spacing: Kirigami.Units.smallSpacing
                QQC2.Label {
                    text: modelData.name || ""
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1 // Force sharing space
                    font.bold: true
                    elide: Text.ElideRight
                }
                QQC2.Label {
                    text: modelData.balance || ""
                    font.family: "Monospace"
                    Layout.alignment: Qt.AlignRight
                    horizontalAlignment: Text.AlignRight
                    color: modelData.color || Kirigami.Theme.textColor
                }
            }
        }
    }

    // --- UNIVERSAL COMPONENT FOR ACCOUNT LISTS ---
    Component {
        id: accountListComp
        Column {
            property var accounts: []
            width: parent ? parent.width : 0
            Repeater {
                model: accounts
                delegate: accountRowComp
            }
        }
    }

    // --- ACCOUNTS SECTION (Preferred/Payment) ---
    Component {
        id: accountsComp
        ColumnLayout {
            spacing: 0
            Repeater {
                model: sectionData ? sectionData.accounts : []
                delegate: QQC2.ItemDelegate {
                    Layout.fillWidth: true
                    padding: Kirigami.Units.smallSpacing
                    background: Rectangle {
                        color: index % 2 === 0 ? "transparent" : Kirigami.Theme.alternateBackgroundColor
                        opacity: 0.3
                    }
                    contentItem: RowLayout {
                        spacing: Kirigami.Units.largeSpacing
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 0
                            QQC2.Label {
                                text: modelData.name || ""
                                Layout.fillWidth: true
                                font.bold: true
                                elide: Text.ElideRight
                            }
                            QQC2.Label {
                                text: modelData.institution || ""
                                Layout.fillWidth: true
                                font.pointSize: Kirigami.Theme.smallFont.pointSize
                                opacity: 0.6
                                visible: text !== ""
                                elide: Text.ElideRight
                            }
                        }
                        ColumnLayout {
                            spacing: 0
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            QQC2.Label {
                                text: modelData.balance || ""
                                font.bold: true
                                font.family: "Monospace"
                                horizontalAlignment: Text.AlignRight
                                color: modelData.color || Kirigami.Theme.textColor
                            }
                            QQC2.Label {
                                text: modelData.totalBalance || ""
                                font.pointSize: Kirigami.Theme.smallFont.pointSize
                                font.family: "Monospace"
                                horizontalAlignment: Text.AlignRight
                                opacity: 0.6
                                visible: (typeof sectionData !== "undefined" && sectionData && sectionData.showTotalBalance && text !== "")
                            }
                        }
                    }
                }
            }
        }
    }

    // --- SCHEDULES SECTION ---
    Component {
        id: schedulesComp
        ColumnLayout {
            spacing: 0
            Repeater {
                model: sectionData ? sectionData.schedules : []
                delegate: QQC2.ItemDelegate {
                    Layout.fillWidth: true
                    padding: Kirigami.Units.smallSpacing
                    background: Rectangle {
                        color: index % 2 === 0 ? "transparent" : Kirigami.Theme.alternateBackgroundColor
                        opacity: 0.3
                    }
                    contentItem: RowLayout {
                        spacing: Kirigami.Units.largeSpacing
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 0
                            QQC2.Label {
                                text: modelData.name || ""
                                Layout.fillWidth: true
                                font.bold: true
                                elide: Text.ElideRight
                            }
                            QQC2.Label {
                                text: modelData.occurrence || ""
                                Layout.fillWidth: true
                                font.pointSize: Kirigami.Theme.smallFont.pointSize
                                opacity: 0.6
                                elide: Text.ElideRight
                            }
                        }
                        ColumnLayout {
                            spacing: 0
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            QQC2.Label {
                                text: modelData.amount || ""
                                font.bold: true
                                font.family: "Monospace"
                                horizontalAlignment: Text.AlignRight
                            }
                            QQC2.Label {
                                text: modelData.nextDueDate || ""
                                font.pointSize: Kirigami.Theme.smallFont.pointSize
                                horizontalAlignment: Text.AlignRight
                                opacity: 0.8
                                color: Kirigami.Theme.neutralTextColor
                            }
                        }
                    }
                }
            }
        }
    }

    // --- ASSETS & LIABILITIES SECTION (Fixed Width 50/50 Split) ---
    Component {
        id: assetsLiabilitiesComp
        ColumnLayout {
            width: parent.width
            spacing: 0

            RowLayout {
                Layout.fillWidth: true
                spacing: 0

                // Left Column: Assets
                ColumnLayout {
                    Layout.preferredWidth: parent.width / 2
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignTop
                    spacing: 0

                    Kirigami.Heading {
                        text: i18n("Assets")
                        level: 4
                        Layout.fillWidth: true
                        Layout.margins: Kirigami.Units.smallSpacing
                        horizontalAlignment: Text.AlignHCenter
                    }
                    Kirigami.Separator { Layout.fillWidth: true }
                    
                    Loader {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignTop
                        sourceComponent: accountListComp
                        onLoaded: item.accounts = (sectionData ? sectionData.assets : [])
                    }
                }

                // Middle: Vertical Separator
                Rectangle {
                    Layout.fillHeight: true
                    width: 5
                    color: Kirigami.Theme.alternateBackgroundColor
                    opacity: 0.5
                }

                // Right Column: Liabilities
                ColumnLayout {
                    Layout.preferredWidth: parent.width / 2
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignTop
                    spacing: 0

                    Kirigami.Heading {
                        text: i18n("Liabilities")
                        level: 4
                        Layout.fillWidth: true
                        Layout.margins: Kirigami.Units.smallSpacing
                        horizontalAlignment: Text.AlignHCenter
                    }
                    Kirigami.Separator { Layout.fillWidth: true }

                    Loader {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignTop
                        sourceComponent: accountListComp
                        onLoaded: item.accounts = (sectionData ? sectionData.liabilities : [])
                    }
                }
            }

            //TODO: Total Assets
            //TODO: Total Liabilities
            // Net Worth Summary Bar
            Rectangle {
                Layout.fillWidth: true
                height: Kirigami.Units.gridUnit * 2
                color: Kirigami.Theme.alternateBackgroundColor
                
                Kirigami.Separator { 
                    anchors.top: parent.top
                    width: parent.width 
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: Kirigami.Units.gridUnit
                    anchors.rightMargin: Kirigami.Units.gridUnit
                    QQC2.Label {
                        text: i18n("Net Worth")
                        Layout.fillWidth: true
                        font.bold: true
                    }
                    QQC2.Label {
                        // TODO: ectionData.netWorth to return actual value
                        text: sectionData ? sectionData.netWorth : ""
                        font.bold: true
                        font.pointSize: Kirigami.Theme.headerFont.pointSize
                        font.family: "Monospace"
                        horizontalAlignment: Text.AlignRight
                        color: (sectionData && sectionData.netWorthColor) ? sectionData.netWorthColor : Kirigami.Theme.textColor
                    }
                }
            }
        }
    }
}
