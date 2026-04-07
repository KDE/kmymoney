import QtQuick

Rectangle {
    id: root
    color: "#f0f0f0"
    border.color: "red"
    border.width: 5

    Component.onCompleted: {
        console.log("Minimalist HomeView Component.onCompleted")
        console.log("homeModel ready:", homeModel.isReady)
        console.log("homeModel rows:", homeModel.rowCount())
    }

    Text {
        id: placeholderText
        anchors.centerIn: parent
        text: (homeModel && !homeModel.isReady) ? "Loading financial data..." : "No data to display. Please check your Home view settings."
        visible: (homeModel && (!homeModel.isReady || homeModel.rowCount() === 0))
        font.pointSize: 18
        color: "black"
    }

    ListView {
        id: sectionsList
        anchors.fill: parent
        anchors.margins: 10
        model: homeModel
        visible: homeModel && homeModel.isReady && homeModel.rowCount() > 0
        spacing: 20
        clip: true

        delegate: Rectangle {
            id: sectionBox
            width: sectionsList.width - 20
            height: (model && model.title) ? (sectionColumn.height + 40) : 0
            color: "white"
            border.color: "#cccccc"
            border.width: 1
            radius: 5
            visible: !!model

            Column {
                id: sectionColumn
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 10
                spacing: 10

                Text {
                    text: (model && model.title) ? model.title : ""
                    font.bold: true
                    font.pointSize: 16
                }

                Loader {
                    id: sectionLoader
                    width: parent.width
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

    Component {
        id: accountsComp
        Column {
            width: parent.width
            spacing: 5
            Repeater {
                model: sectionData ? sectionData.accounts : []
                delegate: Row {
                    width: parent.width
                    Text {
                        text: modelData.name || ""
                        width: parent.width * 0.6
                        font.bold: true
                    }
                    Text {
                        text: modelData.balance || ""
                        width: parent.width * 0.4
                        horizontalAlignment: Text.AlignRight
                        color: modelData.color || "black"
                    }
                }
            }
        }
    }

    Component {
        id: schedulesComp
        Column {
            width: parent.width
            spacing: 5
            Repeater {
                model: sectionData ? sectionData.schedules : []
                delegate: Row {
                    width: parent.width
                    Text {
                        text: modelData.name || ""
                        width: parent.width * 0.6
                    }
                    Text {
                        text: modelData.amount || ""
                        width: parent.width * 0.4
                        horizontalAlignment: Text.AlignRight
                    }
                }
            }
        }
    }

    Component {
        id: assetsComp
        Column {
            width: parent.width
            spacing: 5
            property var section: sectionData

            Repeater {
                model: section ? section.assets : []
                delegate: Row {
                    width: parent.width
                    Text {
                        text: modelData.name || ""
                        width: parent.width * 0.6
                        font.bold: true
                    }
                    Text {
                        text: modelData.balance || ""
                        width: parent.width * 0.4
                        horizontalAlignment: Text.AlignRight
                        color: modelData.color || "black"
                    }
                }
            }
            
            Rectangle {
                width: parent.width
                height: 1
                color: "#eeeeee"
                visible: section && section.liabilities.length > 0
            }

            Repeater {
                model: section ? section.liabilities : []
                delegate: Row {
                    width: parent.width
                    Text {
                        text: modelData.name || ""
                        width: parent.width * 0.6
                    }
                    Text {
                        text: modelData.balance || ""
                        width: parent.width * 0.4
                        horizontalAlignment: Text.AlignRight
                        color: modelData.color || "red"
                    }
                }
            }

            Text {
                width: parent.width
                text: section ? ("Net Worth: " + section.netWorth) : ""
                font.bold: true
                font.pointSize: 14
                horizontalAlignment: Text.AlignRight
                color: (section && section.netWorthColor) ? section.netWorthColor : "black"
            }
        }
    }
}
