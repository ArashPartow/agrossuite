// **************************************************************************
//
//    PARALUTION   www.paralution.com
//
//    Copyright (C) 2015  PARALUTION Labs UG (haftungsbeschränkt) & Co. KG
//                        Am Hasensprung 6, 76571 Gaggenau
//                        Handelsregister: Amtsgericht Mannheim, HRA 706051
//                        Vertreten durch:
//                        PARALUTION Labs Verwaltungs UG (haftungsbeschränkt)
//                        Am Hasensprung 6, 76571 Gaggenau
//                        Handelsregister: Amtsgericht Mannheim, HRB 721277
//                        Geschäftsführer: Dimitar Lukarski, Nico Trost
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// **************************************************************************



// PARALUTION version 1.0.0 


#include "def.hpp"
#include "log.hpp"
#include "time_functions.hpp"
#include "../base/backend_manager.hpp"

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

namespace paralution {

void _paralution_open_log_file(void) {

#ifdef LOG_FILE

  if (_get_backend_descriptor()->log_file != NULL) {
    _paralution_close_log_file();
  }

#endif

  assert(_get_backend_descriptor()->log_file == NULL);

#ifdef LOG_FILE

  _get_backend_descriptor()->log_file = new std::ofstream;

  std::ostringstream str_double;
  str_double.precision(20);
  str_double << paralution_time();
  std::string mid_name= str_double.str();
 
  std::string str_name;

  str_name = "paralution-" + mid_name + ".log";
  
  _get_backend_descriptor()->log_file->open(str_name.c_str(), std::ios::out | std::ios::trunc);
  
#endif

}


void _paralution_close_log_file(void) {

#ifdef LOG_FILE

  _get_backend_descriptor()->log_file->close();
  delete _get_backend_descriptor()->log_file;
  _get_backend_descriptor()->log_file = NULL;

#endif

}

}
