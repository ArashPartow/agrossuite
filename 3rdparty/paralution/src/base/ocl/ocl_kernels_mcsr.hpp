#ifndef PARALUTION_OCL_KERNELS_MCSR_HPP_
#define PARALUTION_OCL_KERNELS_MCSR_HPP_

namespace paralution {

const char *ocl_kernels_mcsr =
	"// *************************************************************************\n"
	"//\n"
	"//    PARALUTION   www.paralution.com\n"
	"//\n"
	"//    Copyright (C) 2012-2013 Dimitar Lukarski\n"
	"//\n"
	"//    This program is free software: you can redistribute it and/or modify\n"
	"//    it under the terms of the GNU General Public License as published by\n"
	"//    the Free Software Foundation, either version 3 of the License, or\n"
	"//    (at your option) any later version.\n"
	"//\n"
	"//    This program is distributed in the hope that it will be useful,\n"
	"//    but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
	"//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
	"//    GNU General Public License for more details.\n"
	"//\n"
	"//    You should have received a copy of the GNU General Public License\n"
	"//    along with this program.  If not, see <http://www.gnu.org/licenses/>.\n"
	"//\n"
	"// *************************************************************************\n"
	"\n"
	"__kernel void kernel_mcsr_spmv_scalar(const int nrow, __global const int *row_offset, \n"
	"                                      __global const int *col, __global const ValueType *val, \n"
	"                                      __global const ValueType *in, __global ValueType *out) {\n"
	"\n"
	"  int ai = get_global_id(0);\n"
	"  int aj;\n"
	"\n"
	"  if (ai < nrow) {\n"
	"\n"
	"    ValueType sum = val[ai] * in[ai];\n"
	"\n"
	"    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)\n"
	"      sum += val[aj] * in[col[aj]];\n"
	"\n"
	"    out[ai] = sum;\n"
	"\n"
	"  }\n"
	"\n"
	"}\n"
	"\n"
	"\n"
	"__kernel void kernel_mcsr_add_spmv_scalar(const int nrow, __global const int *row_offset, \n"
	"                                          __global const int *col, __global const ValueType *val, \n"
	"                                          const ValueType scalar, __global const ValueType *in,\n"
	"                                          __global ValueType *out) {\n"
	"\n"
	"  int ai = get_global_id(0);\n"
	"  int aj;\n"
	"\n"
	"  if (ai < nrow) {\n"
	"\n"
	"    out[ai] += scalar * val[ai] * in[ai];\n"
	"\n"
	"    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)\n"
	"      out[ai] += scalar * val[aj] * in[col[aj]];\n"
	"\n"
	"  }\n"
	"\n"
	"}\n"
	"\n"
	"\n";
}

#endif // PARALUTION_OCL_KERNELS_MCSR_HPP_
