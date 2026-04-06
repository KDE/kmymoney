import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: root
    title: i18n("Financial Summary")

    Kirigami.CardsListView {
        id: cardsView
        anchors.fill: parent
        model: homeModel
        visible: homeModel.isReady && homeModel.rowCount() > 0
        delegate: Kirigami.AbstractCard {
            id: sectionCard
            implicitWidth: parent.width

            header: Kirigami.Heading {
                text: model.title
                level: 2
            }

            contentItem: Item {
                implicitHeight: contentLoader.item ? contentLoader.item.implicitHeight : 0
                Loader {
                    id: contentLoader
                    anchors.fill: parent
                    sourceComponent: {
                        switch(model.type) {
                            case 0: return accountsComponent; // HomeModel::Accounts
                            case 1: return schedulesComponent; // HomeModel::Schedules
                            case 4: return assetsLiabilitiesComponent; // HomeModel::AssetsLiabilities
                            default: return null;
                        }
                    }
                    Binding {
                        target: contentLoader
                        property: "section"
                        value: model.sectionObject
                    }
                }
            }

            footer: (model.type === 4) ? netWorthFooter : null
            
            Component {
                id: netWorthFooter
                Kirigami.Heading {
                    level: 3
                    text: i18n("Net Worth: %1", model.sectionObject.netWorth)
                    color: model.sectionObject.netWorthColor
                    horizontalAlignment: Text.AlignRight
                }
            }
        }
    }

    Kirigami.PlaceholderMessage {
        anchors.centerIn: parent
        width: parent.width - (Kirigami.Units.gridUnit * 4)
        visible: !homeModel.isReady || homeModel.rowCount() === 0
        text: !homeModel.isReady ? i18n("Loading financial data...") : i18n("No data to display. Please check your Home view settings.")
        icon.name: !homeModel.isReady ? "view-refresh" : "view-list-icons"
    }

    Component {
        id: accountsComponent
        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing
            Repeater {
                model: section.accounts
                delegate: QQC2.ItemDelegate {
                    width: parent ? parent.width : 0
                    contentItem: RowLayout {
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 0
                            QQC2.Label {
                                text: modelData.name
                                font.bold: true
                            }
                            QQC2.Label {
                                text: modelData.institution || ""
                                font.pointSize: Kirigami.Theme.smallFont.pointSize
                                visible: text !== ""
                                opacity: 0.7
                            }
                        }
                        ColumnLayout {
                            spacing: 0
                            QQC2.Label {
                                text: modelData.balance
                                color: modelData.color
                                font.bold: true
                                horizontalAlignment: Text.AlignRight
                            }
                            QQC2.Label {
                                text: modelData.totalBalance || ""
                                color: modelData.totalColor || Kirigami.Theme.textColor
                                font.pointSize: Kirigami.Theme.smallFont.pointSize
                                visible: section.showTotalBalance
                                horizontalAlignment: Text.AlignRight
                            }
                        }
                    }
                }
            }
        }
    }

    Component {
        id: schedulesComponent
        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing
            Repeater {
                model: section.schedules
                delegate: QQC2.ItemDelegate {
                    width: parent ? parent.width : 0
                    contentItem: RowLayout {
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 0
                            QQC2.Label {
                                text: modelData.name
                                font.bold: true
                            }
                            QQC2.Label {
                                text: modelData.occurrence
                                font.pointSize: Kirigami.Theme.smallFont.pointSize
                                opacity: 0.7
                            }
                        }
                        ColumnLayout {
                            spacing: 0
                            QQC2.Label {
                                text: modelData.amount
                                font.bold: true
                                horizontalAlignment: Text.AlignRight
                            }
                            QQC2.Label {
                                text: modelData.nextDueDate
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
        id: assetsLiabilitiesComponent
        ColumnLayout {
            QQC2.Label {
                text: i18n("Summary of assets and liabilities")
                font.italic: true
                opacity: 0.7
            }
        }
    }
}
