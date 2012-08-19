<?xml version="1.0"?>
<weakform_h>
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

<includes>#ifndef general_weakform
#define general_weakform

#include "util.h"
#include &lt;weakform/weakform.h&gt;
#include "hermes2d/marker.h"
</includes>

<namespaces>namespace general_weakform
{</namespaces>			
  		
<variable_declaration> mutable Value variable_short;
</variable_declaration>

<custom_matrix_form_vol>template&lt;typename Scalar&gt;
class CustomMatrixFormVol : public Hermes::Hermes2D::MatrixFormVol&lt;Scalar&gt;
{
public:
    CustomMatrixFormVol(unsigned int i, unsigned int j,
                              std::string area,
                              Hermes::Hermes2D::SymFlag sym,
                              Material *materialSource,
			      Material *materialTarget);

    virtual Scalar value(int n, double *wt, Hermes::Hermes2D::Func&lt;Scalar&gt; *u_ext[], Hermes::Hermes2D::Func&lt;double&gt; *u,
                         Hermes::Hermes2D::Func&lt;double&gt; *v, Hermes::Hermes2D::Geom&lt;double&gt; *e, Hermes::Hermes2D::ExtData&lt;Scalar&gt; *ext) const;
    virtual Hermes::Ord ord(int n, double *wt, Hermes::Hermes2D::Func&lt;Hermes::Ord&gt; *u_ext[], Hermes::Hermes2D::Func&lt;Hermes::Ord&gt; *u,
                            Hermes::Hermes2D::Func&lt;Hermes::Ord&gt; *v, Hermes::Hermes2D::Geom&lt;Hermes::Ord&gt; *e, Hermes::Hermes2D::ExtData&lt;Hermes::Ord&gt; *ext) const;   	    
    CustomMatrixFormVol&lt;Scalar&gt;* clone();
private:
    Material *m_materialSource;
    Material *m_materialTarget;	
    Hermes::Hermes2D::SymFlag m_sym;
    //variable_declaration	    	
};</custom_matrix_form_vol> 
  

<custom_vector_form_vol>template&lt;typename Scalar&gt;
class CustomVectorFormVol : public Hermes::Hermes2D::VectorFormVol&lt;Scalar&gt;
{
public:
    CustomVectorFormVol(unsigned int i, unsigned int j,
                              std::string area, 
                              Material *materialSource,
			      Material *materialTarget	
				
    );

    virtual Scalar value(int n, double *wt, Hermes::Hermes2D::Func&lt;Scalar&gt; *u_ext[], Hermes::Hermes2D::Func&lt;double&gt; *v,
                         Hermes::Hermes2D::Geom&lt;double&gt; *e, Hermes::Hermes2D::ExtData&lt;Scalar&gt; *ext) const;
    virtual Hermes::Ord ord(int n, double *wt, Hermes::Hermes2D::Func&lt;Hermes::Ord&gt; *u_ext[], Hermes::Hermes2D::Func&lt;Hermes::Ord&gt; *v,
                            Hermes::Hermes2D::Geom&lt;Hermes::Ord&gt; *e, Hermes::Hermes2D::ExtData&lt;Hermes::Ord&gt; *ext) const;
    CustomVectorFormVol&lt;Scalar&gt;* clone();	
private:		
    Material *m_materialSource;
    Material *m_materialTarget;	
    //variable_declaration
    unsigned int j;
};</custom_vector_form_vol>
  
<custom_matrix_form_surf>template&lt;typename Scalar&gt;
class CustomMatrixFormSurf : public Hermes::Hermes2D::MatrixFormSurf&lt;Scalar&gt;
{
public:
    CustomMatrixFormSurf(unsigned int i, unsigned int j,
                               std::string area, 
                               Boundary *boundary);

    virtual Scalar value(int n, double *wt, Hermes::Hermes2D::Func&lt;Scalar&gt; *u_ext[], Hermes::Hermes2D::Func&lt;double&gt; *u, Hermes::Hermes2D::Func&lt;double&gt; *v,
                         Hermes::Hermes2D::Geom&lt;double&gt; *e, Hermes::Hermes2D::ExtData&lt;Scalar&gt; *ext) const;
    virtual Hermes::Ord ord(int n, double *wt, Hermes::Hermes2D::Func&lt;Hermes::Ord&gt; *u_ext[], Hermes::Hermes2D::Func&lt;Hermes::Ord&gt; *u, Hermes::Hermes2D::Func&lt;Hermes::Ord&gt; *v,
                            Hermes::Hermes2D::Geom&lt;Hermes::Ord&gt; *e, Hermes::Hermes2D::ExtData&lt;Hermes::Ord&gt; *ext) const;
    CustomMatrixFormSurf&lt;Scalar&gt;* clone(); 
private:
    Boundary *m_boundarySource;
    //variable_declaration	
};</custom_matrix_form_surf>

<custom_vector_form_surf>template&lt;typename Scalar&gt;
class CustomVectorFormSurf : public Hermes::Hermes2D::VectorFormSurf&lt;Scalar&gt;
{
public:
    CustomVectorFormSurf(unsigned int i, unsigned int j,
                               std::string area, 
                               Boundary *boundary);

    virtual Scalar value(int n, double *wt, Hermes::Hermes2D::Func&lt;Scalar&gt; *u_ext[], Hermes::Hermes2D::Func&lt;double&gt; *v,
                         Hermes::Hermes2D::Geom&lt;double&gt; *e, Hermes::Hermes2D::ExtData&lt;Scalar&gt; *ext) const;
    virtual Hermes::Ord ord(int n, double *wt, Hermes::Hermes2D::Func&lt;Hermes::Ord&gt; *u_ext[], Hermes::Hermes2D::Func&lt;Hermes::Ord&gt; *v,
                            Hermes::Hermes2D::Geom&lt;Hermes::Ord&gt; *e, Hermes::Hermes2D::ExtData&lt;Hermes::Ord&gt; *ext) const;
    CustomVectorFormSurf&lt;Scalar&gt;* clone();
private:
    Boundary *m_boundarySource;
    //variable_declaration
    unsigned int j;
};</custom_vector_form_surf>

<custom_essential_form_surf>template&lt;typename Scalar&gt;
class CustomEssentialFormSurf : public Hermes::Hermes2D::ExactSolutionScalar&lt;Scalar&gt;
{
public:
    CustomEssentialFormSurf(Hermes::Hermes2D::Mesh *mesh, Boundary *boundary);

    Scalar value(double x, double y) const;
    void derivatives (double x, double y, Scalar&amp; dx, Scalar&amp; dy) const;

    Hermes::Ord ord (Hermes::Ord x, Hermes::Ord y) const
    {
        return Hermes::Ord(Hermes::Ord::get_max_order());
    }
private:
    Boundary *m_boundarySource;  
    //variable_declaration	
};</custom_essential_form_surf>

<footer> }
#endif </footer>	
</weakform_h>
