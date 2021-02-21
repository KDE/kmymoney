#!/bin/bash

# SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
# SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

find kmymoney tools \( -name "*.cpp" -or -name "*.h"  -or -name "*.c"  -or -name "*.cc" \) -exec clang-format-11 -i {} \;
