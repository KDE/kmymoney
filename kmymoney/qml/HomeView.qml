import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: root
    title: i18n("Financial Summary")

    Component.onCompleted: {
        console.log("Kirigami HomeView Component.onCompleted")
        console.log("homeModel ready:", homeModel.isReady)
        console.log("homeModel rows:", homeModel.rowCount())
    }

    Kirigami.CardsListView {
        id: sectionsList
        anchors.fill: parent
        model: homeModel
        visible: homeModel && homeModel.isReady && homeModel.rowCount() > 0
        spacing: Kirigami.Units.largeSpacing

        delegate: Kirigami.AbstractCard {
            id: sectionCard
            implicitWidth: sectionsList.width - Kirigami.Units.gridUnit * 2
            
            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing
                
                Kirigami.Heading {
                    text: model.title
                    level: 2
                    Layout.fillWidth: true
                }

                Loader {
                    id: sectionLoader
                    Layout.fillWidth: true
                    property var sectionData: (model && model.sectionObject) ? model.sectionObject : null
                    sourceComponent: {
                        if (!model) return null;
                        switch(model.type) {
                            case 0: return accountsComp; // HomeModel::Accounts
                            case 1: return schedulesComp; // HomeModel::Schedules
                            case 4: return assetsComp;    // HomeModel::AssetsLiabilities
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

    Component {
        id: accountsComp
        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing
            Repeater {
                model: sectionData ? sectionData.accounts : []
                delegate: QQC2.ItemDelegate {
                    Layout.fillWidth: true
                    contentItem: RowLayout {
                        QQC2.Label {
                            text: modelData.name || ""
                            Layout.fillWidth: true
                            font.bold: true
                        }
                        QQC2.Label {
                            text: modelData.balance || ""
                            horizontalAlignment: Text.AlignRight
                            color: modelData.color || Kirigami.Theme.textColor
                        }
                    }
                }
            }
        }
    }

    Component {
        id: schedulesComp
        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing
            Repeater {
                model: sectionData ? sectionData.schedules : []
                delegate: QQC2.ItemDelegate {
                    Layout.fillWidth: true
                    contentItem: RowLayout {
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 0
                            QQC2.Label {
                                text: modelData.name || ""
                                font.bold: true
                            }
                            QQC2.Label {
                                text: modelData.occurrence || ""
                                font.pointSize: Kirigami.Theme.smallFont.pointSize
                                opacity: 0.7
                            }
                        }
                        ColumnLayout {
                            QQC2.Label {
                                text: modelData.amount || ""
                                font.bold: true
                                horizontalAlignment: Text.AlignRight
                            }
                            QQC2.Label {
                                text: modelData.nextDueDate || ""
                                font.pointSize: Kirigami.Theme.smallFont.pointSize
                                horizontalAlignment: Text.AlignRight
                            }
                        }
                    }
                }
            }
        }
    }

    Component {
        id: assetsComp
        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing
            
            Repeater {
                model: sectionData ? sectionData.assets : []
                delegate: QQC2.ItemDelegate {
                    Layout.fillWidth: true
                    contentItem: RowLayout {
                        QQC2.Label {
                            text: modelData.name || ""
                            Layout.fillWidth: true
                            font.bold: true
                        }
                        QQC2.Label {
                            text: modelData.balance || ""
                            horizontalAlignment: Text.AlignRight
                        }
                    }
                }
            }
            
            Kirigami.Separator {
                Layout.fillWidth: true
                visible: sectionData && sectionData.liabilities.length > 0
            }

            Repeater {
                model: sectionData ? sectionData.liabilities : []
                delegate: QQC2.ItemDelegate {
                    Layout.fillWidth: true
                    contentItem: RowLayout {
                        QQC2.Label {
                            text: modelData.name || ""
                            Layout.fillWidth: true
                        }
                        QQC2.Label {
                            text: modelData.balance || ""
                            horizontalAlignment: Text.AlignRight
                            color: modelData.color || "red"
                        }
                    }
                }
            }

            QQC2.Label {
                Layout.fillWidth: true
                text: sectionData ? (i18n("Net Worth: %1", sectionData.netWorth)) : ""
                font.bold: true
                font.pointSize: Kirigami.Theme.headerFont.pointSize
                horizontalAlignment: Text.AlignRight
                color: (sectionData && sectionData.netWorthColor) ? sectionData.netWorthColor : Kirigami.Theme.textColor
            }
        }
    }
}
