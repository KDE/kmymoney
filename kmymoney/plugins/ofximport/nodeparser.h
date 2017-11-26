/***************************************************************************
                          nodeparse.h
                             -------------------
    copyright            : (C) 2008 by Thomas Baumgart
    email                : <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
