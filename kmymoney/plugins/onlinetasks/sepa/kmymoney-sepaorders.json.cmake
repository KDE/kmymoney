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
                "Name": "Christian David"
            }
        ],
        "Description": "Order types for single european payment area (SEPA) orders",
        "EnabledByDefault": true,
        "Icon": "network-connect",
        "Id": "SEPA orders for online banking",
        "License": "GPLv2+",
        "Name": "SEPA orders",
        "ServiceTypes": [
            "KMyMoney/OnlineTask",
            "KMyMoney/OnlineTaskUi",
            "KMyMoney/sqlStoragePlugin"
        ],
        "Version": "@PROJECT_VERSION@@PROJECT_VERSION_SUFFIX@",
        "Website": "https://kmymoney.org/plugins.html"
    }
}
