// Filename: dcParameter.cxx
// Created by:  drose (15Jun04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "dcParameter.h"
#include "hashGenerator.h"
#include "dcindent.h"
#include "dcTypedef.h"


////////////////////////////////////////////////////////////////////
//     Function: DCParameter::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
DCParameter::
DCParameter() {
  _typedef = NULL;
  _has_fixed_byte_size = false;
  _has_fixed_structure = false;
  _num_nested_fields = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: DCParameter::Copy Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
DCParameter::
DCParameter(const DCParameter &copy) :
  DCField(copy),
  _typedef(copy._typedef)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DCParameter::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
DCParameter::
~DCParameter() {
}

////////////////////////////////////////////////////////////////////
//     Function: DCParameter::as_parameter
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DCParameter *DCParameter::
as_parameter() {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: DCParameter::as_simple_parameter
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DCSimpleParameter *DCParameter::
as_simple_parameter() {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DCParameter::as_class_parameter
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DCClassParameter *DCParameter::
as_class_parameter() {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DCParameter::as_array_parameter
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DCArrayParameter *DCParameter::
as_array_parameter() {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DCParameter::get_typedef
//       Access: Published
//  Description: If this type has been referenced from a typedef,
//               returns the DCTypedef instance, or NULL if the
//               type was declared on-the-fly.
////////////////////////////////////////////////////////////////////
const DCTypedef *DCParameter::
get_typedef() const {
  return _typedef;
}

////////////////////////////////////////////////////////////////////
//     Function: DCParameter::set_typedef
//       Access: Published
//  Description: Records the DCTypedef object that generated this
//               parameter.  This is normally called only from
//               DCTypedef::make_new_parameter().
////////////////////////////////////////////////////////////////////
void DCParameter::
set_typedef(const DCTypedef *dtypedef) {
  _typedef = dtypedef;
}

////////////////////////////////////////////////////////////////////
//     Function: DCParameter::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void DCParameter::
output(ostream &out, bool brief) const {
  string name;
  if (!brief) {
    name = get_name();
  }
  output_instance(out, brief, "", name, "");
}

////////////////////////////////////////////////////////////////////
//     Function: DCParameter::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void DCParameter::
write(ostream &out, bool brief, int indent_level) const {
  // we must always output the name when the parameter occurs by
  // itself within a class, so we pass get_name() even if brief is
  // true.
  write_instance(out, brief, indent_level, "", get_name(), "");
}

////////////////////////////////////////////////////////////////////
//     Function: DCParameter::write_instance
//       Access: Public, Virtual
//  Description: Formats the parameter in the C++-like dc syntax as a
//               typename and identifier.
////////////////////////////////////////////////////////////////////
void DCParameter::
write_instance(ostream &out, bool brief, int indent_level,
               const string &prename, const string &name, 
               const string &postname) const {
  indent(out, indent_level);
  output_instance(out, brief, prename, name, postname);
  output_flags(out);
  out << ";";
  if (!brief && _number >= 0) {
    out << "  // field " << _number;
  }
  out << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: DCParameter::output_typedef_name
//       Access: Public
//  Description: Formats the instance like output_instance, but uses
//               the typedef name instead.
////////////////////////////////////////////////////////////////////
void DCParameter::
output_typedef_name(ostream &out, bool, const string &prename,
                    const string &name, const string &postname) const {
  out << get_typedef()->get_name();
  if (!prename.empty() || !name.empty() || !postname.empty()) {
    out << " " << prename << name << postname;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCParameter::write_typedef_name
//       Access: Public
//  Description: Formats the instance like write_instance, but uses
//               the typedef name instead.
////////////////////////////////////////////////////////////////////
void DCParameter::
write_typedef_name(ostream &out, bool brief, int indent_level, 
                   const string &prename, const string &name, 
                   const string &postname) const {
  indent(out, indent_level)
    << get_typedef()->get_name();
  if (!prename.empty() || !name.empty() || !postname.empty()) {
    out << " " << prename << name << postname;
  }
  output_flags(out);
  out << ";";
  if (!brief && _number >= 0) {
    out << "  // field " << _number;
  }
  out << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: DCParameter::generate_hash
//       Access: Public, Virtual
//  Description: Accumulates the properties of this type into the
//               hash.
////////////////////////////////////////////////////////////////////
void DCParameter::
generate_hash(HashGenerator &hashgen) const {
  // We specifically don't call up to DCField::generate_hash(), since
  // the parameter name is not actually significant to the hash.

  if (get_flags() != 0) {
    hashgen.add_int(get_flags());
  }
}
