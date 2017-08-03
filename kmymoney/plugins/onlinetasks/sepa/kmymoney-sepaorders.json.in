{
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
    },
    "KMyMoney": {
      "OnlineTask": {
          "Iids": ["org.kmymoney.creditTransfer.sepa"],
          "PluginKeyword": "sepaOnlineTasks",
          "Editors": [
            {
            "PluginKeyword": "sepaCreditTransferUi",
            "OnlineTaskIds": ["org.kmymoney.creditTransfer.sepa"],
            "Name": "SEPA Order",
            "Name[de]": "Sepa Überweisung"
            }
          ]
      },
      "StoragePlugin": {
          "Iid": "org.kmymoney.creditTransfer.sepa.sqlStoragePlugin",
          "PluginKeyword": "sepaSqlStoragePlugin"
      }
    }
}
