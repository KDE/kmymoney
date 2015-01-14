# NOTE: this payeeIdentifier and its cmake file cannot be used as a template
# This payeeIdentifier muss be linked with payeeIdentifierLoader because it
# is a fallback if something failed during loading of plugins (for xml files only)

set ( UNAVIALABLE_IDENTIFIER_SCRS
  unavailableplugin.cpp
)

kde4_add_plugin( payeeidentifier_unavailable STATIC
  ${UNAVIALABLE_IDENTIFIER_SCRS}
)

target_link_libraries( payeeidentifier_unavailable
  kmm_payeeidentifier
  Qt5::Core
  Qt5::Xml
)
