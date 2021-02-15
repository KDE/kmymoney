/*
    SPDX-FileCopyrightText: 2008 Thomas Baumgart <<ipwizard@users.sourceforge.net>>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NODEPARSER_H
#define NODEPARSER_H

#include <string>
#include <vector>
#include <libxml++/libxml++.h>

class NodeParser: public xmlpp::Node::NodeList
{
public:
  NodeParser(void) {}
  explicit NodeParser(const xmlpp::Node::NodeList&);
  explicit NodeParser(const xmlpp::Node*);
  explicit NodeParser(const xmlpp::DomParser&);

  NodeParser Path(const std::string& path) const;
  NodeParser Select(const std::string& key, const std::string& value) const;
  std::vector<std::string> Text(void) const;

protected:
  static NodeParser Path(const xmlpp::Node* node, const std::string& path);
};


#endif // NODEPARSER_H
