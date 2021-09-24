
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import QtQml.Models 2.15

import org.kde.kirigami 2.15 as Kirigami

Kirigami.ScrollablePage {
    id: root

    titleDelegate: Rectangle {
        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: Kirigami.Theme.Window
        color: Kirigami.Theme.backgroundColor
        implicitHeight: titleHeading.implicitHeight
        Kirigami.Heading {
            id: titleHeading
            level: 1
            anchors.fill: parent
            text: "Your Financial Summary"
        }
    }

    background: Rectangle {
        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: Kirigami.Theme.Window
        color: Kirigami.Theme.backgroundColor
    }

    Kirigami.CardsListView {
        model: sectionsModel

        delegate: SectionCard {
            title: model.title
            summary: model.summary ?? ""
            contentItem: QQC2.Label {
                text: "Here is where the section item goes"
            }
        }
    }

    component SectionCard : Kirigami.AbstractCard {
        property alias title: sectionTitle.text
        property alias summary: sectionSummary.text

        header: Kirigami.Heading {
            id: sectionTitle
            level: 1
        }

        footer: Kirigami.Heading {
            id: sectionSummary
            level: 4
            visible: text.length > 0
            horizontalAlignment: Text.AlignRight
        }
    }

    ListModel {
        id: sectionsModel
        ListElement { title: "Assets and Liabilites"; summary: "Example Section Summary: 45€"}
        ListElement { title: "Payments" }
        ListElement { title: "Preferred accounts" }
        ListElement { title: "Payment accounts" }
        ListElement { title: "Favorite reports" }
        ListElement { title: "Forecast (schedule)" }
        ListElement { title: "Net worth forecast" }
        ListElement { title: "Budget" }
        ListElement { title: "Cash Flow" }
    }
}
