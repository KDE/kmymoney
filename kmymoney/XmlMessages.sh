function get_files
{
    echo x-kmymoney.xml
}

function po_for_file
{
    case "$1" in
       x-kmymoney.xml)
           echo kmymoney_xml_mimetypes.po
       ;;
    esac
}

function tags_for_file
{
    case "$1" in
       x-kmymoney.xml)
           echo comment
       ;;
    esac
}
