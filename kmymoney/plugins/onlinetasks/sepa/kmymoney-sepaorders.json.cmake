{
    "KMyMoney": {
        "OnlineTask": {
            "Editors": [
                {
                    "Name": "SEPA Order",
                    "Name[de]": "Sepa Überweisung",
                    "OnlineTaskIds": [
                        "org.kmymoney.creditTransfer.sepa"
                    ]
                }
            ],
            "Iids": [
                "org.kmymoney.creditTransfer.sepa"
            ]
        },
        "StoragePlugin": {
            "Iid": "org.kmymoney.creditTransfer.sepa.sqlStoragePlugin"
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
        "Version": "@PROJECT_VERSION@@PROJECT_VERSION_SUFFIX@",
        "Website": "https://kmymoney.org/plugins.html"
    }
}
