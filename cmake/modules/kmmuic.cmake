# used internally by KMMMacros.cmake
# original from neundorf@kde.org

EXECUTE_PROCESS(COMMAND ${KDE_UIC_EXECUTABLE}
  -L ${KDE_UIC_PLUGIN_DIR} -L ${KMM_UIC_PLUGIN_DIR}
  -nounload -tr tr2i18n -impl ${KDE_UIC_H_FILE} ${KDE_UIC_FILE}
  OUTPUT_VARIABLE _uic_CONTENTS
  )

STRING(REGEX REPLACE "tr2i18n\\(\"\"\\)" "QString::null" _uic_CONTENTS "${_uic_CONTENTS}" )
STRING(REGEX REPLACE "tr2i18n\\(\"\", \"\"\\)" "QString::null" _uic_CONTENTS "${_uic_CONTENTS}" )

FILE(WRITE ${KDE_UIC_CPP_FILE} "#include <kdialog.h>\n#include <klocale.h>\n\n")
FILE(APPEND ${KDE_UIC_CPP_FILE} "${_uic_CONTENTS}")
