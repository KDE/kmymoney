{
    "KMyMoney": {
        "OnlineTask": {
            "Editors": [
                {
                    "Name": "SEPA Order",
                    "Name[de]": "Sepa Überweisung",
                    "OnlineTaskIds": [
                        "org.kmymoney.creditTransfer.sepa"
                    ],
                    "PluginKeyword": "sepaCreditTransferUi"
                }
            ],
            "Iids": [
                "org.kmymoney.creditTransfer.sepa"
            ],
            "PluginKeyword": "sepaOnlineTasks"
        },
        "StoragePlugin": {
            "Iid": "org.kmymoney.creditTransfer.sepa.sqlStoragePlugin",
            "PluginKeyword": "sepaSqlStoragePlugin"
        }
    },
    "KPlugin": {
        "Authors": [
            {
                "Email": "christian-david@web.de",
                "Name": "Christian David",
                "Name[ca@valencia]": "Christian David",
                "Name[ca]": "Christian David",
                "Name[es]": "Christian David",
                "Name[gl]": "Christian David",
                "Name[it]": "Christian David",
                "Name[nl]": "Christian David",
                "Name[pt]": "Christian David",
                "Name[pt_BR]": "Christian David",
                "Name[sv]": "Christian David",
                "Name[uk]": "Christian David",
                "Name[x-test]": "xxChristian Davidxx"
            }
        ],
        "Description": "Order types for single european payment area (SEPA) orders",
        "Description[ca@valencia]": "Tipus d'ordres per a les ordres de l'àrea única europea de pagaments «SEPA» (Single European Payment Area)",
        "Description[ca]": "Tipus d'ordres per a les ordres de l'àrea única europea de pagaments «SEPA» (Single European Payment Area)",
        "Description[gl]": "Tipos das ordes da área europea única de pagos (SEPA)",
        "Description[it]": "Tipi per gli ordini dell'area unica dei pagamenti in euro (SEPA)",
        "Description[nl]": "Opdrachttypen voor enkelvoudige betalingsopdrachten in Europees gebied (SEPA)",
        "Description[pt]": "Tipos de transferências SEPA (Single European Payment Area)",
        "Description[pt_BR]": "Tipos de ordem para o SEPA (Single European Payment Area)",
        "Description[sv]": "Ordertyp för gemensamma eurobetalningsområdet (SEPA) order",
        "Description[uk]": "Типи векселів для Єдиної європейської системи сплат (SEPA)",
        "Description[x-test]": "xxOrder types for single european payment area (SEPA) ordersxx",
        "EnabledByDefault": true,
        "Icon": "network-connect",
        "Id": "SEPA orders for online banking",
        "License": "GPLv2+",
        "Name": "SEPA orders",
        "Name[ca@valencia]": "Ordres SEPA",
        "Name[ca]": "Ordres SEPA",
        "Name[es]": "Pagos SEPA",
        "Name[gl]": "Ordes SEPA",
        "Name[it]": "Ordini SEPA",
        "Name[nl]": "SEPA-opdrachten",
        "Name[pt]": "Transferências SEPA",
        "Name[pt_BR]": "Ordens SEPA",
        "Name[sv]": "SEPA-order",
        "Name[uk]": "Векселі SEPA",
        "Name[x-test]": "xxSEPA ordersxx",
        "ServiceTypes": [
            "KMyMoney/OnlineTask",
            "KMyMoney/OnlineTaskUi",
            "KMyMoney/sqlStoragePlugin"
        ],
        "Version": "@PROJECT_VERSION@@PROJECT_VERSION_SUFFIX@",
        "Website": "https://kmymoney.org/plugins.html"
    }
}
