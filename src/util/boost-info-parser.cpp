/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014-2015 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * From PyNDN boost_info_parser by Adeola Bannis.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version, with the additional exemption that
 * compiling, linking, and/or using OpenSSL is allowed.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * A copy of the GNU Lesser General Public License is in the file COPYING.
 */

#include <string.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include "boost-info-parser.hpp"

using namespace std;

namespace ndn {

static const char *WHITESPACE_CHARS = " \t\n\r";

/**
 * Modify str in place to erase whitespace on the left.
 * @param str
 */
static __inline void
trimLeft(string& str)
{
  size_t found = str.find_first_not_of(WHITESPACE_CHARS);
  if (found != string::npos) {
    if (found > 0)
      str.erase(0, found);
  }
  else
    // All whitespace
    str.clear();
}

/**
 * Modify str in place to erase whitespace on the right.
 * @param str
 */
static __inline void
trimRight(string& str)
{
  size_t found = str.find_last_not_of(WHITESPACE_CHARS);
  if (found != string::npos) {
    if (found + 1 < str.size())
      str.erase(found + 1);
  }
  else
    // All whitespace
    str.clear();
}

/**
 * Modify str in place to erase whitespace on the left and right.
 * @param str
 */
static void
trim(string& str)
{
  trimLeft(str);
  trimRight(str);
}

void
BoostInfoTree::addSubtree
  (const string& treeName, ptr_lib::shared_ptr<BoostInfoTree> newTree)
{
  vector<ptr_lib::shared_ptr<BoostInfoTree> >* subtreeList = find(treeName);
  if (subtreeList)
    subtreeList->push_back(newTree);
  else {
    subtrees_.push_back(make_pair
      (treeName, vector<ptr_lib::shared_ptr<BoostInfoTree> >()));
    subtrees_.back().second.push_back(newTree);
  }

  newTree->parent_ = this;
  lastChild_ = newTree.get();
}

const BoostInfoTree&
BoostInfoTree::createSubtree(const string& treeName, const string& value)
{
  ptr_lib::shared_ptr<BoostInfoTree> newTree(new BoostInfoTree(value, this));
  addSubtree(treeName, newTree);
  return *newTree;
}

vector<const BoostInfoTree*>
BoostInfoTree::operator [] (const string& keyIn) const
{
  vector<const BoostInfoTree*> foundVals;

  size_t iNotSlash = keyIn.find_first_not_of('/');
  string key;
  if (iNotSlash != string::npos)
    key = keyIn.substr(iNotSlash);
  if (key.size() == 0) {
    foundVals.push_back(this);
    return foundVals;
  }
  vector<string> path = split(key, '/');

  const vector<ptr_lib::shared_ptr<BoostInfoTree> >* subtrees =
    const_cast<BoostInfoTree*>(this)->find(path[0]);
  if (!subtrees)
    return foundVals;

  if (path.size() == 1) {
    // Copy the pointers.
    for (int i = 0; i < (*subtrees).size(); ++i)
      foundVals.push_back((*subtrees)[i].get());
    return foundVals;
  }

  // newPath = '/'.join(path[1:])
  string newPath;
  for (int i = 1; i < path.size(); ++i) {
    if (i > 1)
      newPath += "/";
    newPath += path[i];
  }

  for (int i = 0; i < (*subtrees).size(); ++i) {
    const BoostInfoTree& t = *(*subtrees)[i];
    vector<const BoostInfoTree*> partial = t[newPath];
    foundVals.insert(foundVals.end(), partial.begin(), partial.end());
  }

  return foundVals;
}

vector<ptr_lib::shared_ptr<BoostInfoTree> >*
BoostInfoTree::find(const string& treeName)
{
  for (size_t i = 0; i < subtrees_.size(); ++i) {
    if (subtrees_[i].first == treeName)
      return &subtrees_[i].second;
  }

  return 0;
}

string
BoostInfoTree::prettyPrint(int indentLevel) const
{
  string prefix(indentLevel, ' ');
  string s;

  if (parent_) {
    if (value_.size() > 0)
      s += "\"" + value_ + "\"";
    s += "\n";
  }

  if (subtrees_.size() > 0) {
    if (parent_)
      s += prefix+ "{\n";
    string nextLevel(indentLevel + 2, ' ');
    for (size_t i = 0; i < subtrees_.size(); ++i) {
      for (size_t iSubTree = 0; iSubTree < subtrees_[i].second.size(); ++iSubTree)
        s += nextLevel + subtrees_[i].first + " " +
             subtrees_[i].second[iSubTree]->prettyPrint(indentLevel + 2);
    }

    if (parent_)
      s +=  prefix + "}\n";
  }

  return s;
}

vector<string>
BoostInfoTree::split(const string &input, char separator)
{
  vector<string> result;
  stringstream is(input);
  string str;
  while (getline(is, str, separator))
    result.push_back(str);

  return result;
}

const BoostInfoTree&
BoostInfoParser::read(const string& fileName)
{
  read(fileName, root_.get());
  return *root_;
}

BoostInfoTree*
BoostInfoParser::read(const string& fileName, BoostInfoTree* ctx)
{
  ifstream stream(fileName.c_str());

  string line;
  while (getline(stream, line)) {
    trim(line);
    ctx = parseLine(line, ctx);
  }

  stream.close();
  return ctx;
}

void
BoostInfoParser::write(const string& fileName) const
{
  ofstream stream(fileName.c_str());
  stream << root_;
  stream.close();
}

void
BoostInfoParser::shlex_split(const string& s, vector<string>& result)
{
  if (s.size() == 0)
    return;
  const char* whiteSpace = " \t\n\r";
  size_t iStart = 0;

  while (true) {
    // Move iStart past whitespace.
    while (::strchr(whiteSpace, s[iStart])) {
      ++iStart;
      if (iStart >= s.size())
        // Done.
        return;
    }

    // Move iEnd to the end of the token.
    size_t iEnd = iStart;
    bool inQuotation = false;
    string token;
    while (true) {
      if (s[iEnd] == '\\') {
        // Append characters up to the backslash, skip the backslash and
        //   move iEnd past the escaped character.
        token.append(s, iStart, iEnd - iStart);
        iStart = iEnd + 1;
        iEnd = iStart;
        if (iEnd >= s.size()) {
          // An unusual case: A backslash at the end of the string.
          break;
        }
      }
      else {
        if (inQuotation) {
          if (s[iEnd] == '\"') {
            // Append characters up to the end quote and skip.
            token.append(s, iStart, iEnd - iStart);
            iStart = iEnd + 1;
            inQuotation = false;
          }
        }
        else {
          if (s[iEnd] == '\"') {
            // Append characters up to the start quote and skip.
            token.append(s, iStart, iEnd - iStart);
            iStart = iEnd + 1;
            inQuotation = true;
          }
          else {
            if (::strchr(whiteSpace, s[iEnd]))
              break;
          }
        }
      }

      ++iEnd;
      if (iEnd >= s.size())
          break;
    }


    token.append(s, iStart, iEnd - iStart);
    result.push_back(token);
    if (iEnd >= s.size())
        // Done.
        return;

    iStart = iEnd;
  }
}

BoostInfoTree*
BoostInfoParser::parseLine(const string& lineIn, BoostInfoTree* context)
{
  // Make a working copy.
  string line(lineIn);

  // Skip blank lines and comments.
  size_t commentStart = line.find_first_of(';');
  if (commentStart != string::npos) {
    line = line.substr(0, commentStart);
    trim(line);
  }
  if (line.size() == 0)
    return context;

  // Usually we are expecting key and optional value.
  vector<string> strings;
  shlex_split(line, strings);
  bool isSectionStart = false;
  bool isSectionEnd = false;
  for (size_t i = 0; i < strings.size(); ++i) {
    isSectionStart = (isSectionStart || strings[i] == "{");
    isSectionEnd = (isSectionEnd || strings[i] == "}");
  }

  if (!isSectionStart && !isSectionEnd) {
    string key = strings[0];
    string val;
    if (strings.size() > 1)
      val = strings[1];

    // If it is an "#include", load the new file instead of inserting keys.
    if (key == "#include")
      context = read(val, context);
    else
      context->createSubtree(key, val);

    return context;
  }

  // OK, who is the joker who put a { on the same line as the key name?!
  size_t sectionStart = line.find_first_of('{');
  if (sectionStart != string::npos && sectionStart > 0) {
    string firstPart = line.substr(0, sectionStart);
    string secondPart = line.substr(sectionStart);

    BoostInfoTree* ctx = parseLine(firstPart, context);
    return parseLine(secondPart, ctx);
  }

  // If we encounter a {, we are beginning a new context.
  // TODO: Error if there was already a subcontext here.
  if (line[0] == '{') {
    context = context->getLastChild();
    return context;
  }

  // If we encounter a }, we are ending a list context.
  if (line[0] == '}') {
    context = context->getParent();
    return context;
  }

  throw runtime_error("BoostInfoParser: input line is malformed");
}

}
