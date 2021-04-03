/*
    SPDX-FileCopyrightText: 2008 Thomas Baumgart <<ipwizard@users.sourceforge.net>>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "nodeparser.h"

using std::string;
using std::vector;

NodeParser::NodeParser(const xmlpp::Node::NodeList& list): xmlpp::Node::NodeList(list)
{
}

NodeParser::NodeParser(const xmlpp::Node* node)
{
    push_back(const_cast<xmlpp::Node*>(node));
}

NodeParser::NodeParser(const xmlpp::DomParser& parser)
{
    xmlpp::Node* node = parser.get_document()->get_root_node();
    push_back(const_cast<xmlpp::Node*>(node));
}

NodeParser NodeParser::Path(const xmlpp::Node* node, const std::string& path)
{
    //std::cout << Q_FUNC_INFO << std::endl;

    NodeParser result;

    // split path string into the 1st level, and the rest
    std::string key = path;
    std::string remainder;
    std::string::size_type token_pos = path.find('/');
    if (token_pos != std::string::npos) {
        key = path.substr(0, token_pos);
        remainder = path.substr(token_pos + 1);
    }

    // find the first level nodes that match
    xmlpp::Node::NodeList list = node->get_children();
    for (xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter) {
        if ((*iter)->get_name() == key) {
            // if there is still some path left, ask for the rest of the path from those nodes.
            if (remainder.length()) {
                NodeParser remain_list = NodeParser(*iter).Path(remainder);
                result.splice(result.end(), remain_list);
            }

            // otherwise add the node to the result list.
            else
                result.push_back(*iter);
        }
    }

    return result;
}

NodeParser NodeParser::Path(const std::string& path) const
{
    //std::cout << Q_FUNC_INFO << std::endl;
    NodeParser result;

    for (const_iterator iter = begin(); iter != end(); ++iter) {
        NodeParser iter_list = Path(*iter, path);
        result.splice(result.end(), iter_list);
    }

    return result;
}

NodeParser NodeParser::Select(const std::string& key, const std::string& value) const
{
    //std::cout << Q_FUNC_INFO << std::endl;
    NodeParser result;
    for (const_iterator iter = begin(); iter != end(); ++iter) {
        xmlpp::Node::NodeList list = (*iter)->get_children();
        for (xmlpp::Node::NodeList::const_iterator iter3 = list.begin(); iter3 != list.end(); ++iter3) {
            if ((*iter3)->get_name() == key) {
                xmlpp::Node::NodeList list = (*iter3)->get_children();
                for (xmlpp::Node::NodeList::const_iterator iter4 = list.begin(); iter4 != list.end(); ++iter4) {
                    const xmlpp::TextNode* nodeText = dynamic_cast<const xmlpp::TextNode*>(*iter4);
                    if (nodeText && nodeText->get_content() == value)
                        result.push_back(*iter);
                    break;
                }
            }
        }
    }
    return result;
}

vector<string> NodeParser::Text(void) const
{
    //std::cout << Q_FUNC_INFO << std::endl;
    vector<string> result;

    // Go through the list of nodes
    for (xmlpp::Node::NodeList::const_iterator iter = begin(); iter != end(); ++iter) {
        // Find the text child node, and print that
        xmlpp::Node::NodeList list = (*iter)->get_children();
        for (xmlpp::Node::NodeList::const_iterator iter2 = list.begin(); iter2 != list.end(); ++iter2) {
            const xmlpp::TextNode* nodeText = dynamic_cast<const xmlpp::TextNode*>(*iter2);
            if (nodeText) {
                result.push_back(nodeText->get_content());
            }
        }
    }
    if (result.empty())
        result.push_back(string());
    return result;
}
