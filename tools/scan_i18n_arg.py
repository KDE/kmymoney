#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2025 Thomas Baumgart <tbaumgart@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later

"""
scan_i18n.py - Scan C++ source files for i18n function calls still using deprecated .arg member

This tool scans C++ source code files to find all occurrences of function calls
that start with 'i18n' (like i18n, i18nc, i18np, etc.), have one or more comma-
separated arguments, and are still using deprecated .arg member. It properly handles nested
parentheses in function arguments and multi-line constructs.

Usage examples:
  ./scan_i18n_arg.py file.cpp
  ./scan_i18n_arg.py -v *.cpp *.h
  find -name "*.cpp" -o -name "*.h" | xargs ./scan_i18n_arg.py

The last example is particularly useful for scanning entire source trees.

Developed with the help of Amazon-Kiro.
"""

import re
import sys

def scan_i18n_calls(filename, verbose=False, progress=False):
    if progress:
        print(f"Scanning {filename}")
    
    with open(filename, 'r') as f:
        content = f.read()
    
    # Find i18n calls with balanced parentheses followed by .arg
    pattern = r'i18n\w*\s*\('
    
    matches = []
    for match in re.finditer(pattern, content):
        start = match.start()
        paren_start = match.end() - 1  # Position of opening parenthesis
        paren_count = 0
        i = paren_start
        
        # Find matching closing parenthesis
        while i < len(content):
            if content[i] == '(':
                paren_count += 1
            elif content[i] == ')':
                paren_count -= 1
                if paren_count == 0:
                    # Check if followed by .arg (without semicolon in between)
                    remaining = content[i+1:]
                    arg_match = re.match(r'\s*\.\s*arg', remaining)
                    if arg_match and ';' not in remaining[:arg_match.start()]:
                        end_pos = i + 1 + arg_match.end()
                        matches.append((start, end_pos))
                    break
            i += 1
    
    # Read original content for line numbers and display
    with open(filename, 'r') as f:
        original_content = f.read()
    
    results = []
    for start, end in matches:
        # Find line number by counting newlines before the match
        line_num = original_content[:start].count('\n') + 1
        if verbose:
            matched_text = original_content[start:end]
            results.append(f"{filename}:{line_num}:\n{matched_text}\n")
        else:
            results.append(f"{filename}:{line_num}")
    
    return results

if __name__ == "__main__":
    verbose = False
    progress = False
    files = []
    
    for arg in sys.argv[1:]:
        if arg in ['-h', '--help']:
            print("Usage: [python3] scan_i18n_arg.py [-v|--verbose] [-p|--progress] <cpp_file> [cpp_file2] ...")
            print("Scan C++ files for i18n function calls still using deprecated .arg member")
            print("Options:")
            print("  -v, --verbose   Show full matched content")
            print("  -p, --progress  Show scanning progress")
            print("  -h, --help      Show this help message")
            sys.exit(0)
        elif arg in ['-v', '--verbose']:
            verbose = True
        elif arg in ['-p', '--progress']:
            progress = True
        else:
            files.append(arg)
    
    if not files:
        print("Usage: [python3] scan_i18n_arg.py [-v|--verbose] [-p|--progress] <cpp_file> [cpp_file2] ...")
        sys.exit(1)
    
    all_results = []
    for filename in files:
        results = scan_i18n_calls(filename, verbose, progress)
        all_results.extend(results)
    
    for result in all_results:
        print(result)
