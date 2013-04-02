// Copyright (C) 2005-2010 Code Synthesis Tools CC
//
// This program was generated by CodeSynthesis XSD, an XML Schema to
// C++ data binding compiler.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
//
// In addition, as a special exception, Code Synthesis Tools CC gives
// permission to link this program with the Xerces-C++ library (or with
// modified versions of Xerces-C++ that use the same license as Xerces-C++),
// and distribute linked combinations including the two. You must obey
// the GNU General Public License version 2 in all respects for all of
// the code used other than Xerces-C++. If you modify this copy of the
// program, you may extend this exception to your version of the program,
// but you are not obligated to do so. If you do not wish to do so, delete
// this exception statement from your version.
//
// Furthermore, Code Synthesis Tools CC makes a special exception for
// the Free/Libre and Open Source Software (FLOSS) which is described
// in the accompanying FLOSSE file.
//

#ifndef CXX______RESOURCES_XSD_AGROS2D_STRUCTURE_XML_H
#define CXX______RESOURCES_XSD_AGROS2D_STRUCTURE_XML_H

// Begin prologue.
//
//
// End prologue.

#include <xsd/cxx/config.hxx>

#if (XSD_INT_VERSION != 3030000L)
#error XSD runtime version mismatch
#endif

#include <xsd/cxx/pre.hxx>

#ifndef XSD_USE_CHAR
#define XSD_USE_CHAR
#endif

#ifndef XSD_CXX_TREE_USE_CHAR
#define XSD_CXX_TREE_USE_CHAR
#endif

#include <xsd/cxx/xml/char-utf8.hxx>

#include <xsd/cxx/tree/exceptions.hxx>
#include <xsd/cxx/tree/elements.hxx>
#include <xsd/cxx/tree/types.hxx>

#include <xsd/cxx/xml/error-handler.hxx>

#include <xsd/cxx/xml/dom/auto-ptr.hxx>

#include <xsd/cxx/tree/parsing.hxx>
#include <xsd/cxx/tree/parsing/byte.hxx>
#include <xsd/cxx/tree/parsing/unsigned-byte.hxx>
#include <xsd/cxx/tree/parsing/short.hxx>
#include <xsd/cxx/tree/parsing/unsigned-short.hxx>
#include <xsd/cxx/tree/parsing/int.hxx>
#include <xsd/cxx/tree/parsing/unsigned-int.hxx>
#include <xsd/cxx/tree/parsing/long.hxx>
#include <xsd/cxx/tree/parsing/unsigned-long.hxx>
#include <xsd/cxx/tree/parsing/boolean.hxx>
#include <xsd/cxx/tree/parsing/float.hxx>
#include <xsd/cxx/tree/parsing/double.hxx>
#include <xsd/cxx/tree/parsing/decimal.hxx>

#include <xsd/cxx/xml/dom/serialization-header.hxx>
#include <xsd/cxx/tree/serialization.hxx>
#include <xsd/cxx/tree/serialization/byte.hxx>
#include <xsd/cxx/tree/serialization/unsigned-byte.hxx>
#include <xsd/cxx/tree/serialization/short.hxx>
#include <xsd/cxx/tree/serialization/unsigned-short.hxx>
#include <xsd/cxx/tree/serialization/int.hxx>
#include <xsd/cxx/tree/serialization/unsigned-int.hxx>
#include <xsd/cxx/tree/serialization/long.hxx>
#include <xsd/cxx/tree/serialization/unsigned-long.hxx>
#include <xsd/cxx/tree/serialization/boolean.hxx>
#include <xsd/cxx/tree/serialization/float.hxx>
#include <xsd/cxx/tree/serialization/double.hxx>
#include <xsd/cxx/tree/serialization/decimal.hxx>

#include <xsd/cxx/tree/std-ostream-operators.hxx>

namespace xml_schema
{
  // anyType and anySimpleType.
  //
  typedef ::xsd::cxx::tree::type type;
  typedef ::xsd::cxx::tree::simple_type< type > simple_type;
  typedef ::xsd::cxx::tree::type container;

  // 8-bit
  //
  typedef signed char byte;
  typedef unsigned char unsigned_byte;

  // 16-bit
  //
  typedef short short_;
  typedef unsigned short unsigned_short;

  // 32-bit
  //
  typedef int int_;
  typedef unsigned int unsigned_int;

  // 64-bit
  //
  typedef long long long_;
  typedef unsigned long long unsigned_long;

  // Supposed to be arbitrary-length integral types.
  //
  typedef long long integer;
  typedef long long non_positive_integer;
  typedef unsigned long long non_negative_integer;
  typedef unsigned long long positive_integer;
  typedef long long negative_integer;

  // Boolean.
  //
  typedef bool boolean;

  // Floating-point types.
  //
  typedef float float_;
  typedef double double_;
  typedef double decimal;

  // String types.
  //
  typedef ::xsd::cxx::tree::string< char, simple_type > string;
  typedef ::xsd::cxx::tree::normalized_string< char, string > normalized_string;
  typedef ::xsd::cxx::tree::token< char, normalized_string > token;
  typedef ::xsd::cxx::tree::name< char, token > name;
  typedef ::xsd::cxx::tree::nmtoken< char, token > nmtoken;
  typedef ::xsd::cxx::tree::nmtokens< char, simple_type, nmtoken > nmtokens;
  typedef ::xsd::cxx::tree::ncname< char, name > ncname;
  typedef ::xsd::cxx::tree::language< char, token > language;

  // ID/IDREF.
  //
  typedef ::xsd::cxx::tree::id< char, ncname > id;
  typedef ::xsd::cxx::tree::idref< char, ncname, type > idref;
  typedef ::xsd::cxx::tree::idrefs< char, simple_type, idref > idrefs;

  // URI.
  //
  typedef ::xsd::cxx::tree::uri< char, simple_type > uri;

  // Qualified name.
  //
  typedef ::xsd::cxx::tree::qname< char, simple_type, uri, ncname > qname;

  // Binary.
  //
  typedef ::xsd::cxx::tree::buffer< char > buffer;
  typedef ::xsd::cxx::tree::base64_binary< char, simple_type > base64_binary;
  typedef ::xsd::cxx::tree::hex_binary< char, simple_type > hex_binary;

  // Date/time.
  //
  typedef ::xsd::cxx::tree::time_zone time_zone;
  typedef ::xsd::cxx::tree::date< char, simple_type > date;
  typedef ::xsd::cxx::tree::date_time< char, simple_type > date_time;
  typedef ::xsd::cxx::tree::duration< char, simple_type > duration;
  typedef ::xsd::cxx::tree::gday< char, simple_type > gday;
  typedef ::xsd::cxx::tree::gmonth< char, simple_type > gmonth;
  typedef ::xsd::cxx::tree::gmonth_day< char, simple_type > gmonth_day;
  typedef ::xsd::cxx::tree::gyear< char, simple_type > gyear;
  typedef ::xsd::cxx::tree::gyear_month< char, simple_type > gyear_month;
  typedef ::xsd::cxx::tree::time< char, simple_type > time;

  // Entity.
  //
  typedef ::xsd::cxx::tree::entity< char, ncname > entity;
  typedef ::xsd::cxx::tree::entities< char, simple_type, entity > entities;

  // Namespace information and list stream. Used in
  // serialization functions.
  //
  typedef ::xsd::cxx::xml::dom::namespace_info< char > namespace_info;
  typedef ::xsd::cxx::xml::dom::namespace_infomap< char > namespace_infomap;
  typedef ::xsd::cxx::tree::list_stream< char > list_stream;
  typedef ::xsd::cxx::tree::as_double< double_ > as_double;
  typedef ::xsd::cxx::tree::as_decimal< decimal > as_decimal;
  typedef ::xsd::cxx::tree::facet facet;

  // Flags and properties.
  //
  typedef ::xsd::cxx::tree::flags flags;
  typedef ::xsd::cxx::tree::properties< char > properties;

  // Parsing/serialization diagnostics.
  //
  typedef ::xsd::cxx::tree::severity severity;
  typedef ::xsd::cxx::tree::error< char > error;
  typedef ::xsd::cxx::tree::diagnostics< char > diagnostics;

  // Exceptions.
  //
  typedef ::xsd::cxx::tree::exception< char > exception;
  typedef ::xsd::cxx::tree::bounds< char > bounds;
  typedef ::xsd::cxx::tree::duplicate_id< char > duplicate_id;
  typedef ::xsd::cxx::tree::parsing< char > parsing;
  typedef ::xsd::cxx::tree::expected_element< char > expected_element;
  typedef ::xsd::cxx::tree::unexpected_element< char > unexpected_element;
  typedef ::xsd::cxx::tree::expected_attribute< char > expected_attribute;
  typedef ::xsd::cxx::tree::unexpected_enumerator< char > unexpected_enumerator;
  typedef ::xsd::cxx::tree::expected_text_content< char > expected_text_content;
  typedef ::xsd::cxx::tree::no_prefix_mapping< char > no_prefix_mapping;
  typedef ::xsd::cxx::tree::no_type_info< char > no_type_info;
  typedef ::xsd::cxx::tree::not_derived< char > not_derived;
  typedef ::xsd::cxx::tree::serialization< char > serialization;

  // Error handler callback interface.
  //
  typedef ::xsd::cxx::xml::error_handler< char > error_handler;

  // DOM interaction.
  //
  namespace dom
  {
    // Automatic pointer for DOMDocument.
    //
    using ::xsd::cxx::xml::dom::auto_ptr;

#ifndef XSD_CXX_TREE_TREE_NODE_KEY__XML_SCHEMA
#define XSD_CXX_TREE_TREE_NODE_KEY__XML_SCHEMA
    // DOM user data key for back pointers to tree nodes.
    //
    const XMLCh* const tree_node_key = ::xsd::cxx::tree::user_data_keys::node;
#endif
  }
}

// Forward declarations.
//
namespace XMLStructure
{
  class structure;
  class element_data;
  class files;
  class newton_residuals;
  class file;
  class residual;
}


#include <memory>    // std::auto_ptr
#include <limits>    // std::numeric_limits
#include <algorithm> // std::binary_search

#include <xsd/cxx/xml/char-utf8.hxx>

#include <xsd/cxx/tree/exceptions.hxx>
#include <xsd/cxx/tree/elements.hxx>
#include <xsd/cxx/tree/containers.hxx>
#include <xsd/cxx/tree/list.hxx>

#include <xsd/cxx/xml/dom/parsing-header.hxx>

namespace XMLStructure
{
  class structure: public ::xml_schema::type
  {
    public:
    // element_data
    // 
    typedef ::XMLStructure::element_data element_data_type;
    typedef ::xsd::cxx::tree::sequence< element_data_type > element_data_sequence;
    typedef element_data_sequence::iterator element_data_iterator;
    typedef element_data_sequence::const_iterator element_data_const_iterator;
    typedef ::xsd::cxx::tree::traits< element_data_type, char > element_data_traits;

    const element_data_sequence&
    element_data () const;

    element_data_sequence&
    element_data ();

    void
    element_data (const element_data_sequence& s);

    // Constructors.
    //
    structure ();

    structure (const ::xercesc::DOMElement& e,
               ::xml_schema::flags f = 0,
               ::xml_schema::container* c = 0);

    structure (const structure& x,
               ::xml_schema::flags f = 0,
               ::xml_schema::container* c = 0);

    virtual structure*
    _clone (::xml_schema::flags f = 0,
            ::xml_schema::container* c = 0) const;

    virtual 
    ~structure ();

    // Implementation.
    //
    protected:
    void
    parse (::xsd::cxx::xml::dom::parser< char >&,
           ::xml_schema::flags);

    protected:
    element_data_sequence element_data_;
  };

  class element_data: public ::xml_schema::type
  {
    public:
    // files
    // 
    typedef ::XMLStructure::files files_type;
    typedef ::xsd::cxx::tree::traits< files_type, char > files_traits;

    const files_type&
    files () const;

    files_type&
    files ();

    void
    files (const files_type& x);

    void
    files (::std::auto_ptr< files_type > p);

    // newton_residuals
    // 
    typedef ::XMLStructure::newton_residuals newton_residuals_type;
    typedef ::xsd::cxx::tree::traits< newton_residuals_type, char > newton_residuals_traits;

    const newton_residuals_type&
    newton_residuals () const;

    newton_residuals_type&
    newton_residuals ();

    void
    newton_residuals (const newton_residuals_type& x);

    void
    newton_residuals (::std::auto_ptr< newton_residuals_type > p);

    // field_id
    // 
    typedef ::xml_schema::string field_id_type;
    typedef ::xsd::cxx::tree::traits< field_id_type, char > field_id_traits;

    const field_id_type&
    field_id () const;

    field_id_type&
    field_id ();

    void
    field_id (const field_id_type& x);

    void
    field_id (::std::auto_ptr< field_id_type > p);

    // time_step
    // 
    typedef ::xml_schema::integer time_step_type;
    typedef ::xsd::cxx::tree::traits< time_step_type, char > time_step_traits;

    const time_step_type&
    time_step () const;

    time_step_type&
    time_step ();

    void
    time_step (const time_step_type& x);

    // adaptivity_step
    // 
    typedef ::xml_schema::integer adaptivity_step_type;
    typedef ::xsd::cxx::tree::traits< adaptivity_step_type, char > adaptivity_step_traits;

    const adaptivity_step_type&
    adaptivity_step () const;

    adaptivity_step_type&
    adaptivity_step ();

    void
    adaptivity_step (const adaptivity_step_type& x);

    // solution_type
    // 
    typedef ::xml_schema::string solution_type_type;
    typedef ::xsd::cxx::tree::traits< solution_type_type, char > solution_type_traits;

    const solution_type_type&
    solution_type () const;

    solution_type_type&
    solution_type ();

    void
    solution_type (const solution_type_type& x);

    void
    solution_type (::std::auto_ptr< solution_type_type > p);

    // time_step_length
    // 
    typedef ::xml_schema::double_ time_step_length_type;
    typedef ::xsd::cxx::tree::optional< time_step_length_type > time_step_length_optional;
    typedef ::xsd::cxx::tree::traits< time_step_length_type, char, ::xsd::cxx::tree::schema_type::double_ > time_step_length_traits;

    const time_step_length_optional&
    time_step_length () const;

    time_step_length_optional&
    time_step_length ();

    void
    time_step_length (const time_step_length_type& x);

    void
    time_step_length (const time_step_length_optional& x);

    // adaptivity_error
    // 
    typedef ::xml_schema::double_ adaptivity_error_type;
    typedef ::xsd::cxx::tree::optional< adaptivity_error_type > adaptivity_error_optional;
    typedef ::xsd::cxx::tree::traits< adaptivity_error_type, char, ::xsd::cxx::tree::schema_type::double_ > adaptivity_error_traits;

    const adaptivity_error_optional&
    adaptivity_error () const;

    adaptivity_error_optional&
    adaptivity_error ();

    void
    adaptivity_error (const adaptivity_error_type& x);

    void
    adaptivity_error (const adaptivity_error_optional& x);

    // dofs
    // 
    typedef ::xml_schema::int_ dofs_type;
    typedef ::xsd::cxx::tree::optional< dofs_type > dofs_optional;
    typedef ::xsd::cxx::tree::traits< dofs_type, char > dofs_traits;

    const dofs_optional&
    dofs () const;

    dofs_optional&
    dofs ();

    void
    dofs (const dofs_type& x);

    void
    dofs (const dofs_optional& x);

    // Constructors.
    //
    element_data (const files_type&,
                  const newton_residuals_type&,
                  const field_id_type&,
                  const time_step_type&,
                  const adaptivity_step_type&,
                  const solution_type_type&);

    element_data (::std::auto_ptr< files_type >&,
                  ::std::auto_ptr< newton_residuals_type >&,
                  const field_id_type&,
                  const time_step_type&,
                  const adaptivity_step_type&,
                  const solution_type_type&);

    element_data (const ::xercesc::DOMElement& e,
                  ::xml_schema::flags f = 0,
                  ::xml_schema::container* c = 0);

    element_data (const element_data& x,
                  ::xml_schema::flags f = 0,
                  ::xml_schema::container* c = 0);

    virtual element_data*
    _clone (::xml_schema::flags f = 0,
            ::xml_schema::container* c = 0) const;

    virtual 
    ~element_data ();

    // Implementation.
    //
    protected:
    void
    parse (::xsd::cxx::xml::dom::parser< char >&,
           ::xml_schema::flags);

    protected:
    ::xsd::cxx::tree::one< files_type > files_;
    ::xsd::cxx::tree::one< newton_residuals_type > newton_residuals_;
    ::xsd::cxx::tree::one< field_id_type > field_id_;
    ::xsd::cxx::tree::one< time_step_type > time_step_;
    ::xsd::cxx::tree::one< adaptivity_step_type > adaptivity_step_;
    ::xsd::cxx::tree::one< solution_type_type > solution_type_;
    time_step_length_optional time_step_length_;
    adaptivity_error_optional adaptivity_error_;
    dofs_optional dofs_;
  };

  class files: public ::xml_schema::type
  {
    public:
    // file
    // 
    typedef ::XMLStructure::file file_type;
    typedef ::xsd::cxx::tree::sequence< file_type > file_sequence;
    typedef file_sequence::iterator file_iterator;
    typedef file_sequence::const_iterator file_const_iterator;
    typedef ::xsd::cxx::tree::traits< file_type, char > file_traits;

    const file_sequence&
    file () const;

    file_sequence&
    file ();

    void
    file (const file_sequence& s);

    // Constructors.
    //
    files ();

    files (const ::xercesc::DOMElement& e,
           ::xml_schema::flags f = 0,
           ::xml_schema::container* c = 0);

    files (const files& x,
           ::xml_schema::flags f = 0,
           ::xml_schema::container* c = 0);

    virtual files*
    _clone (::xml_schema::flags f = 0,
            ::xml_schema::container* c = 0) const;

    virtual 
    ~files ();

    // Implementation.
    //
    protected:
    void
    parse (::xsd::cxx::xml::dom::parser< char >&,
           ::xml_schema::flags);

    protected:
    file_sequence file_;
  };

  class newton_residuals: public ::xml_schema::type
  {
    public:
    // residual
    // 
    typedef ::XMLStructure::residual residual_type;
    typedef ::xsd::cxx::tree::sequence< residual_type > residual_sequence;
    typedef residual_sequence::iterator residual_iterator;
    typedef residual_sequence::const_iterator residual_const_iterator;
    typedef ::xsd::cxx::tree::traits< residual_type, char > residual_traits;

    const residual_sequence&
    residual () const;

    residual_sequence&
    residual ();

    void
    residual (const residual_sequence& s);

    // Constructors.
    //
    newton_residuals ();

    newton_residuals (const ::xercesc::DOMElement& e,
                      ::xml_schema::flags f = 0,
                      ::xml_schema::container* c = 0);

    newton_residuals (const newton_residuals& x,
                      ::xml_schema::flags f = 0,
                      ::xml_schema::container* c = 0);

    virtual newton_residuals*
    _clone (::xml_schema::flags f = 0,
            ::xml_schema::container* c = 0) const;

    virtual 
    ~newton_residuals ();

    // Implementation.
    //
    protected:
    void
    parse (::xsd::cxx::xml::dom::parser< char >&,
           ::xml_schema::flags);

    protected:
    residual_sequence residual_;
  };

  class file: public ::xml_schema::type
  {
    public:
    // array_id
    // 
    typedef ::xml_schema::int_ array_id_type;
    typedef ::xsd::cxx::tree::traits< array_id_type, char > array_id_traits;

    const array_id_type&
    array_id () const;

    array_id_type&
    array_id ();

    void
    array_id (const array_id_type& x);

    // mesh_filename
    // 
    typedef ::xml_schema::string mesh_filename_type;
    typedef ::xsd::cxx::tree::traits< mesh_filename_type, char > mesh_filename_traits;

    const mesh_filename_type&
    mesh_filename () const;

    mesh_filename_type&
    mesh_filename ();

    void
    mesh_filename (const mesh_filename_type& x);

    void
    mesh_filename (::std::auto_ptr< mesh_filename_type > p);

    // space_filename
    // 
    typedef ::xml_schema::string space_filename_type;
    typedef ::xsd::cxx::tree::traits< space_filename_type, char > space_filename_traits;

    const space_filename_type&
    space_filename () const;

    space_filename_type&
    space_filename ();

    void
    space_filename (const space_filename_type& x);

    void
    space_filename (::std::auto_ptr< space_filename_type > p);

    // solution_filename
    // 
    typedef ::xml_schema::string solution_filename_type;
    typedef ::xsd::cxx::tree::traits< solution_filename_type, char > solution_filename_traits;

    const solution_filename_type&
    solution_filename () const;

    solution_filename_type&
    solution_filename ();

    void
    solution_filename (const solution_filename_type& x);

    void
    solution_filename (::std::auto_ptr< solution_filename_type > p);

    // Constructors.
    //
    file (const array_id_type&,
          const mesh_filename_type&,
          const space_filename_type&,
          const solution_filename_type&);

    file (const ::xercesc::DOMElement& e,
          ::xml_schema::flags f = 0,
          ::xml_schema::container* c = 0);

    file (const file& x,
          ::xml_schema::flags f = 0,
          ::xml_schema::container* c = 0);

    virtual file*
    _clone (::xml_schema::flags f = 0,
            ::xml_schema::container* c = 0) const;

    virtual 
    ~file ();

    // Implementation.
    //
    protected:
    void
    parse (::xsd::cxx::xml::dom::parser< char >&,
           ::xml_schema::flags);

    protected:
    ::xsd::cxx::tree::one< array_id_type > array_id_;
    ::xsd::cxx::tree::one< mesh_filename_type > mesh_filename_;
    ::xsd::cxx::tree::one< space_filename_type > space_filename_;
    ::xsd::cxx::tree::one< solution_filename_type > solution_filename_;
  };

  class residual: public ::xml_schema::type
  {
    public:
    // value
    // 
    typedef ::xml_schema::float_ value_type;
    typedef ::xsd::cxx::tree::traits< value_type, char > value_traits;

    const value_type&
    value () const;

    value_type&
    value ();

    void
    value (const value_type& x);

    // Constructors.
    //
    residual (const value_type&);

    residual (const ::xercesc::DOMElement& e,
              ::xml_schema::flags f = 0,
              ::xml_schema::container* c = 0);

    residual (const residual& x,
              ::xml_schema::flags f = 0,
              ::xml_schema::container* c = 0);

    virtual residual*
    _clone (::xml_schema::flags f = 0,
            ::xml_schema::container* c = 0) const;

    virtual 
    ~residual ();

    // Implementation.
    //
    protected:
    void
    parse (::xsd::cxx::xml::dom::parser< char >&,
           ::xml_schema::flags);

    protected:
    ::xsd::cxx::tree::one< value_type > value_;
  };
}

#include <iosfwd>

namespace XMLStructure
{
  ::std::ostream&
  operator<< (::std::ostream&, const structure&);

  ::std::ostream&
  operator<< (::std::ostream&, const element_data&);

  ::std::ostream&
  operator<< (::std::ostream&, const files&);

  ::std::ostream&
  operator<< (::std::ostream&, const newton_residuals&);

  ::std::ostream&
  operator<< (::std::ostream&, const file&);

  ::std::ostream&
  operator<< (::std::ostream&, const residual&);
}

#include <iosfwd>

#include <xercesc/sax/InputSource.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>

namespace XMLStructure
{
  // Parse a URI or a local file.
  //

  ::std::auto_ptr< ::XMLStructure::structure >
  structure_ (const ::std::string& uri,
              ::xml_schema::flags f = 0,
              const ::xml_schema::properties& p = ::xml_schema::properties ());

  ::std::auto_ptr< ::XMLStructure::structure >
  structure_ (const ::std::string& uri,
              ::xml_schema::error_handler& eh,
              ::xml_schema::flags f = 0,
              const ::xml_schema::properties& p = ::xml_schema::properties ());

  ::std::auto_ptr< ::XMLStructure::structure >
  structure_ (const ::std::string& uri,
              ::xercesc::DOMErrorHandler& eh,
              ::xml_schema::flags f = 0,
              const ::xml_schema::properties& p = ::xml_schema::properties ());

  // Parse std::istream.
  //

  ::std::auto_ptr< ::XMLStructure::structure >
  structure_ (::std::istream& is,
              ::xml_schema::flags f = 0,
              const ::xml_schema::properties& p = ::xml_schema::properties ());

  ::std::auto_ptr< ::XMLStructure::structure >
  structure_ (::std::istream& is,
              ::xml_schema::error_handler& eh,
              ::xml_schema::flags f = 0,
              const ::xml_schema::properties& p = ::xml_schema::properties ());

  ::std::auto_ptr< ::XMLStructure::structure >
  structure_ (::std::istream& is,
              ::xercesc::DOMErrorHandler& eh,
              ::xml_schema::flags f = 0,
              const ::xml_schema::properties& p = ::xml_schema::properties ());

  ::std::auto_ptr< ::XMLStructure::structure >
  structure_ (::std::istream& is,
              const ::std::string& id,
              ::xml_schema::flags f = 0,
              const ::xml_schema::properties& p = ::xml_schema::properties ());

  ::std::auto_ptr< ::XMLStructure::structure >
  structure_ (::std::istream& is,
              const ::std::string& id,
              ::xml_schema::error_handler& eh,
              ::xml_schema::flags f = 0,
              const ::xml_schema::properties& p = ::xml_schema::properties ());

  ::std::auto_ptr< ::XMLStructure::structure >
  structure_ (::std::istream& is,
              const ::std::string& id,
              ::xercesc::DOMErrorHandler& eh,
              ::xml_schema::flags f = 0,
              const ::xml_schema::properties& p = ::xml_schema::properties ());

  // Parse xercesc::InputSource.
  //

  ::std::auto_ptr< ::XMLStructure::structure >
  structure_ (::xercesc::InputSource& is,
              ::xml_schema::flags f = 0,
              const ::xml_schema::properties& p = ::xml_schema::properties ());

  ::std::auto_ptr< ::XMLStructure::structure >
  structure_ (::xercesc::InputSource& is,
              ::xml_schema::error_handler& eh,
              ::xml_schema::flags f = 0,
              const ::xml_schema::properties& p = ::xml_schema::properties ());

  ::std::auto_ptr< ::XMLStructure::structure >
  structure_ (::xercesc::InputSource& is,
              ::xercesc::DOMErrorHandler& eh,
              ::xml_schema::flags f = 0,
              const ::xml_schema::properties& p = ::xml_schema::properties ());

  // Parse xercesc::DOMDocument.
  //

  ::std::auto_ptr< ::XMLStructure::structure >
  structure_ (const ::xercesc::DOMDocument& d,
              ::xml_schema::flags f = 0,
              const ::xml_schema::properties& p = ::xml_schema::properties ());

  ::std::auto_ptr< ::XMLStructure::structure >
  structure_ (::xml_schema::dom::auto_ptr< ::xercesc::DOMDocument >& d,
              ::xml_schema::flags f = 0,
              const ::xml_schema::properties& p = ::xml_schema::properties ());
}

#include <iosfwd>

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/framework/XMLFormatter.hpp>

#include <xsd/cxx/xml/dom/auto-ptr.hxx>

namespace XMLStructure
{
  // Serialize to std::ostream.
  //

  void
  structure_ (::std::ostream& os,
              const ::XMLStructure::structure& x, 
              const ::xml_schema::namespace_infomap& m = ::xml_schema::namespace_infomap (),
              const ::std::string& e = "UTF-8",
              ::xml_schema::flags f = 0);

  void
  structure_ (::std::ostream& os,
              const ::XMLStructure::structure& x, 
              ::xml_schema::error_handler& eh,
              const ::xml_schema::namespace_infomap& m = ::xml_schema::namespace_infomap (),
              const ::std::string& e = "UTF-8",
              ::xml_schema::flags f = 0);

  void
  structure_ (::std::ostream& os,
              const ::XMLStructure::structure& x, 
              ::xercesc::DOMErrorHandler& eh,
              const ::xml_schema::namespace_infomap& m = ::xml_schema::namespace_infomap (),
              const ::std::string& e = "UTF-8",
              ::xml_schema::flags f = 0);

  // Serialize to xercesc::XMLFormatTarget.
  //

  void
  structure_ (::xercesc::XMLFormatTarget& ft,
              const ::XMLStructure::structure& x, 
              const ::xml_schema::namespace_infomap& m = ::xml_schema::namespace_infomap (),
              const ::std::string& e = "UTF-8",
              ::xml_schema::flags f = 0);

  void
  structure_ (::xercesc::XMLFormatTarget& ft,
              const ::XMLStructure::structure& x, 
              ::xml_schema::error_handler& eh,
              const ::xml_schema::namespace_infomap& m = ::xml_schema::namespace_infomap (),
              const ::std::string& e = "UTF-8",
              ::xml_schema::flags f = 0);

  void
  structure_ (::xercesc::XMLFormatTarget& ft,
              const ::XMLStructure::structure& x, 
              ::xercesc::DOMErrorHandler& eh,
              const ::xml_schema::namespace_infomap& m = ::xml_schema::namespace_infomap (),
              const ::std::string& e = "UTF-8",
              ::xml_schema::flags f = 0);

  // Serialize to an existing xercesc::DOMDocument.
  //

  void
  structure_ (::xercesc::DOMDocument& d,
              const ::XMLStructure::structure& x,
              ::xml_schema::flags f = 0);

  // Serialize to a new xercesc::DOMDocument.
  //

  ::xml_schema::dom::auto_ptr< ::xercesc::DOMDocument >
  structure_ (const ::XMLStructure::structure& x, 
              const ::xml_schema::namespace_infomap& m = ::xml_schema::namespace_infomap (),
              ::xml_schema::flags f = 0);

  void
  operator<< (::xercesc::DOMElement&, const structure&);

  void
  operator<< (::xercesc::DOMElement&, const element_data&);

  void
  operator<< (::xercesc::DOMElement&, const files&);

  void
  operator<< (::xercesc::DOMElement&, const newton_residuals&);

  void
  operator<< (::xercesc::DOMElement&, const file&);

  void
  operator<< (::xercesc::DOMElement&, const residual&);
}

#include <xsd/cxx/post.hxx>

// Begin epilogue.
//
//
// End epilogue.

#endif // CXX______RESOURCES_XSD_AGROS2D_STRUCTURE_XML_H
