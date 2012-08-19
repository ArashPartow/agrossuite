<?xml version="1.0"?>
<weakform_cpp>
<head>// This file is part of Agros2D.
//
// Agros2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros2D.  If not, see &lt;http://www.gnu.org/licenses/&gt;.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros2d@googlegroups.com, home page: http://hpfem.org/agros2d/
</head>

<includes>#include "general_weakform.h"

#include "util.h"
#include "scene.h"
#include "hermes2d.h"
#include "module.h"
#include "problem.h"

</includes>

<namespaces>using namespace general_weakform;
</namespaces>			
  		
<custom_matrix_form_vol>

template &lt;typename Scalar&gt;
CustomMatrixFormVol&lt;Scalar&gt;::CustomMatrixFormVol(unsigned int i, unsigned int j,
                                                 std::string area,
                                                 Hermes::Hermes2D::SymFlag sym,
                                                 Material *materialSource,
						 Material *materialTarget
						)
    : Hermes::Hermes2D::MatrixFormVol&lt;Scalar&gt;(i, j, area, sym), m_materialSource(materialSource), m_materialTarget(materialTarget), m_sym(sym)
{
//variable_definition	
}

</custom_matrix_form_vol>

<variable_definition> variable_short = m_materialSource->value("variable");
</variable_definition>

<comment_separator>
// **********************************************************************************************
</comment_separator>


<custom_matrix_form_vol_value> 

template &lt;typename Scalar&gt;
Scalar CustomMatrixFormVol&lt;Scalar&gt;::value(int n, double *wt, Hermes::Hermes2D::Func&lt;Scalar&gt; *u_ext[], Hermes::Hermes2D::Func&lt;double&gt; *u,
                                          Hermes::Hermes2D::Func&lt;double&gt; *v, Hermes::Hermes2D::Geom&lt;double&gt; *e, Hermes::Hermes2D::ExtData&lt;Scalar&gt; *ext) const
{
    double result = 0;    
    for (int i = 0; i &lt; n; i++)
    {
       result += wt[i] * //expression
    }
    return result;
}

</custom_matrix_form_vol_value>

<custom_matrix_form_vol_ord>template &lt;typename Scalar&gt;
Hermes::Ord CustomMatrixFormVol&lt;Scalar&gt;::ord(int n, double *wt, Hermes::Hermes2D::Func&lt;Hermes::Ord&gt; *u_ext[], Hermes::Hermes2D::Func&lt;Hermes::Ord&gt; *u,
                                             Hermes::Hermes2D::Func&lt;Hermes::Ord&gt; *v, Hermes::Hermes2D::Geom&lt;Hermes::Ord&gt; *e, Hermes::Hermes2D::ExtData&lt;Hermes::Ord&gt; *ext) const
{
    Hermes::Ord result(0);    
    for (int i = 0; i &lt; n; i++)
    {
       result += wt[i] * //expression
    }	
    return result;
}</custom_matrix_form_vol_ord>

<custom_matrix_form_vol_clone>template &lt;typename Scalar&gt;
CustomMatrixFormVol&lt;Scalar&gt;* CustomMatrixFormVol&lt;Scalar&gt;::clone()
{
    return new CustomMatrixFormVol(this->i, this->j, this->areas[0], this->m_sym, 
                                         this->m_materialSource, this->m_materialTarget);
}
</custom_matrix_form_vol_clone>



<custom_vector_form_vol>

template &lt;typename Scalar&gt;
CustomVectorFormVol&lt;Scalar&gt;::CustomVectorFormVol(unsigned int i, unsigned int j,
                                                 std::string area, 
                                                 Material* materialSource, Material* materialTarget)
    : Hermes::Hermes2D::VectorFormVol&lt;Scalar&gt;(i, area), m_materialSource(materialSource), m_materialTarget(materialTarget), j(j)
{
//variable_definition	
}

</custom_vector_form_vol>

<custom_vector_form_vol_value>

template &lt;typename Scalar&gt;
Scalar CustomVectorFormVol&lt;Scalar&gt;::value(int n, double *wt, Hermes::Hermes2D::Func&lt;Scalar&gt; *u_ext[], Hermes::Hermes2D::Func&lt;double&gt; *v,
                                          Hermes::Hermes2D::Geom&lt;double&gt; *e, Hermes::Hermes2D::ExtData&lt;Scalar&gt; *ext) const
{
    double result = 0;

    for (int i = 0; i &lt; n; i++)
    {
        result += wt[i] * //expression
    }
    return result;
}

</custom_vector_form_vol_value>

<custom_vector_form_vol_ord>

template &lt;typename Scalar&gt;
Hermes::Ord CustomVectorFormVol&lt;Scalar&gt;::ord(int n, double *wt, Hermes::Hermes2D::Func&lt;Hermes::Ord&gt; *u_ext[], Hermes::Hermes2D::Func&lt;Hermes::Ord&gt; *v,
                                             Hermes::Hermes2D::Geom&lt;Hermes::Ord&gt; *e, Hermes::Hermes2D::ExtData&lt;Hermes::Ord&gt; *ext) const
{
    //variable_definition
    Hermes::Ord result(0);    
    for (int i = 0; i &lt; n; i++)
    {
       result += wt[i] * //expression
    }	
    return result;
}

</custom_vector_form_vol_ord>

<custom_vector_form_vol_clone>template &lt;typename Scalar&gt;
CustomVectorFormVol&lt;Scalar&gt;* CustomVectorFormVol&lt;Scalar&gt;::clone()
{
    return new CustomVectorFormVol(this->i, this->j, this->areas[0],
                                         this->m_materialSource, this->m_materialTarget);
}
</custom_vector_form_vol_clone>


<custom_matrix_form_surf>

template &lt;typename Scalar&gt;
CustomMatrixFormSurf&lt;Scalar&gt;::CustomMatrixFormSurf(unsigned int i, unsigned int j,
                                                   std::string area, 
                                                   Boundary *boundary)
    : Hermes::Hermes2D::MatrixFormSurf&lt;Scalar&gt;(i, j, area), m_boundarySource(boundary)
{
    //variable_definition
}

</custom_matrix_form_surf>


<custom_matrix_form_surf_value>

template &lt;typename Scalar&gt;
Scalar CustomMatrixFormSurf&lt;Scalar&gt;::value(int n, double *wt, Hermes::Hermes2D::Func&lt;Scalar&gt; *u_ext[], Hermes::Hermes2D::Func&lt;double&gt; *u, Hermes::Hermes2D::Func&lt;double&gt; *v,
                                           Hermes::Hermes2D::Geom&lt;double&gt; *e, Hermes::Hermes2D::ExtData&lt;Scalar&gt; *ext) const
{
    //variable_definition
    double result = 0;    
    for (int i = 0; i &lt; n; i++)
    {
        result += wt[i] * //expression
    }
    return result;
}

</custom_matrix_form_surf_value>

<custom_matrix_form_surf_ord>

template &lt;typename Scalar&gt;
Hermes::Ord CustomMatrixFormSurf&lt;Scalar&gt;::ord(int n, double *wt, Hermes::Hermes2D::Func&lt;Hermes::Ord&gt; *u_ext[], Hermes::Hermes2D::Func&lt;Hermes::Ord&gt; *u, Hermes::Hermes2D::Func&lt;Hermes::Ord&gt; *v,
                                              Hermes::Hermes2D::Geom&lt;Hermes::Ord&gt; *e, Hermes::Hermes2D::ExtData&lt;Hermes::Ord&gt; *ext) const
{
    //variable_definition
    Hermes::Ord result(0);    
    for (int i = 0; i &lt; n; i++)
    {
       result += wt[i] * //expression
    }	
    return result;

}

</custom_matrix_form_surf_ord>

<custom_matrix_form_surf_clone>template &lt;typename Scalar&gt;
CustomMatrixFormSurf&lt;Scalar&gt;* CustomMatrixFormSurf&lt;Scalar&gt;::clone()
{
    return new CustomMatrixFormSurf(this->i, this->j, this->areas[0],
                                         this->m_boundarySource);
}
</custom_matrix_form_surf_clone>

<custom_vector_form_surf>

template &lt;typename Scalar&gt;
CustomVectorFormSurf&lt;Scalar&gt;::CustomVectorFormSurf(unsigned int i, unsigned int j,
                                                   std::string area, 
                                                   Boundary *boundary)

    : Hermes::Hermes2D::VectorFormSurf&lt;Scalar&gt;(i, area), m_boundarySource(boundary), j(j)
{
//variable_definition
}

</custom_vector_form_surf>

<custom_vector_form_surf_value>

template &lt;typename Scalar&gt;
Scalar CustomVectorFormSurf&lt;Scalar&gt;::value(int n, double *wt, Hermes::Hermes2D::Func&lt;Scalar&gt; *u_ext[], Hermes::Hermes2D::Func&lt;double&gt; *v,
                                           Hermes::Hermes2D::Geom&lt;double&gt; *e, Hermes::Hermes2D::ExtData&lt;Scalar&gt; *ext) const
{
    double result = 0;    
    for (int i = 0; i &lt; n; i++)
    {
        result += wt[i] * //expression
    }
    return result;
}

</custom_vector_form_surf_value>

<custom_vector_form_surf_ord>

template &lt;typename Scalar&gt;
Hermes::Ord CustomVectorFormSurf&lt;Scalar&gt;::ord(int n, double *wt, Hermes::Hermes2D::Func&lt;Hermes::Ord&gt; *u_ext[], Hermes::Hermes2D::Func&lt;Hermes::Ord&gt; *v,
                                              Hermes::Hermes2D::Geom&lt;Hermes::Ord&gt; *e, Hermes::Hermes2D::ExtData&lt;Hermes::Ord&gt; *ext) const
{
    Hermes::Ord result(0);    
    for (int i = 0; i &lt; n; i++)
    {
       result += wt[i] * //expression
    }	
    return result;

}

</custom_vector_form_surf_ord>

<custom_vector_form_surf_clone>template &lt;typename Scalar&gt;
CustomVectorFormSurf&lt;Scalar&gt;* CustomVectorFormSurf&lt;Scalar&gt;::clone()
{
    return new CustomVectorFormSurf(this->i, this->j, this->areas[0],  
                                         this->m_boundarySource);
}
</custom_vector_form_surf_clone>

<custom_essential_form_surf>

template &lt;typename Scalar&gt;
CustomEssentialFormSurf&lt;Scalar&gt;::CustomEssentialFormSurf(Hermes::Hermes2D::Mesh *mesh, Boundary *boundary)
    : Hermes::Hermes2D::ExactSolutionScalar&lt;Scalar&gt;(mesh), m_boundarySource(boundary)
{
//variable_definition
}

</custom_essential_form_surf>

<custom_essential_form_surf_value>

template &lt;typename Scalar&gt;
Scalar CustomEssentialFormSurf&lt;Scalar&gt;::value(double x, double y) const
{
    double result = //expression
    return result;
}

</custom_essential_form_surf_value>

<custom_essential_form_surf_derivatives>

template &lt;typename Scalar&gt;
void CustomEssentialFormSurf&lt;Scalar&gt;::derivatives (double x, double y, Scalar&amp; dx, Scalar&amp; dy) const
{

}

</custom_essential_form_surf_derivatives>

<footer>
template class ClassName&lt;double&gt;;</footer>
</weakform_cpp>
