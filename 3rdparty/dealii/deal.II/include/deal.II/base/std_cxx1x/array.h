// ---------------------------------------------------------------------
// $Id: array.h 31349 2013-10-20 19:07:06Z maier $
//
// Copyright (C) 2009 - 2013 by the deal.II authors
//
// This file is part of the deal.II library.
//
// The deal.II library is free software; you can use it, redistribute
// it, and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// The full text of the license can be found in the file LICENSE at
// the top level of the deal.II distribution.
//
// ---------------------------------------------------------------------

#ifndef __deal2__std_cxx1x_array_h
#define __deal2__std_cxx1x_array_h


#include <deal.II/base/config.h>

#ifdef DEAL_II_USE_CXX11

#  include <array>
DEAL_II_NAMESPACE_OPEN
namespace std_cxx1x
{
  using std::array;
}
DEAL_II_NAMESPACE_CLOSE

#else

#include <boost/array.hpp>
DEAL_II_NAMESPACE_OPEN
namespace std_cxx1x
{
  using boost::array;
}
DEAL_II_NAMESPACE_CLOSE

#endif

#endif