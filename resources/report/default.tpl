<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
	<meta name="generator" content="Agros2D" />
	<link href="{{STYLESHEET}}" rel="stylesheet" type="text/css" media="screen" />
</head>
<body>
<h1>{{NAME}}</h1>

{{#DESCRIPTION_SECTION}}
<h2>{{DESCRIPTION_LABEL}}</h2>
<p>{{DESCRIPTION}}</p>
{{/DESCRIPTION_SECTION}}

{{#PROBLEM_SECTION}}
<h2>{{PROBLEM_LABEL}}</h2>
<table>
	<tr><td>{{PROBLEM_NAME_LABEL}}</td><td>{{PROBLEM_NAME}}</td></tr>
	<tr><td>{{PROBLEM_DATE_LABEL}}</td><td>{{PROBLEM_DATE}}</td></tr>
	<tr><td>{{PROBLEM_FILENAME_LABEL}}</td><td>{{PROBLEM_FILENAME}}</td></tr>
	<tr><td>{{PROBLEM_PROBLEMTYPE_LABEL}}</td><td>{{PROBLEM_PROBLEMTYPE}}</td></tr>
	<tr><td>{{PROBLEM_PHYSICFIELD_LABEL}}</td><td>{{PROBLEM_PHYSICFIELD}}</td></tr>
	<tr><td>{{PROBLEM_ANALYSISTYPE_LABEL}}</td><td>{{PROBLEM_ANALYSISTYPE}}</td></tr>
	<tr><td>{{PROBLEM_NUMBEROFREFINEMENTS_LABEL}}</td><td>{{PROBLEM_NUMBEROFREFINEMENTS}}</td></tr>
	<tr><td>{{PROBLEM_POLYNOMIALORDER_LABEL}}</td><td>{{PROBLEM_POLYNOMIALORDER}}</td></tr>
	<tr><td>{{PROBLEM_NEWTONTOLERANCE_LABEL}}</td><td>{{PROBLEM_NEWTONTOLERANCE}} %</td></tr>
	<tr><td>{{PROBLEM_NEWTONSTEPS_LABEL}}</td><td>{{PROBLEM_NEWTONSTEPS}}</td></tr>
	{{#PROBLEM_ADAPTIVITY_SECTION}}
	<tr><td>{{PROBLEM_ADAPTIVITYTYPE_LABEL}}</td><td>{{PROBLEM_ADAPTIVITYTYPE}}</td></tr>
	<tr><td>{{PROBLEM_ADAPTIVITYSTEPS_LABEL}}</td><td>{{PROBLEM_ADAPTIVITYSTEPS}}</td></tr>
	<tr><td>{{PROBLEM_ADAPTIVITYTOLERANCE_LABEL}}</td><td>{{PROBLEM_ADAPTIVITYTOLERANCE}} %</td></tr>
	<tr><td>{{PROBLEM_MAXDOFS_LABEL}}</td><td>{{PROBLEM_MAXDOFS}}</td></tr>
	{{/PROBLEM_ADAPTIVITY_SECTION}}
	{{#PROBLEM_HARMONIC_SECTION}}
	<tr><td>{{PROBLEM_FREQUENCY_LABEL}}</td><td>{{PROBLEM_FREQUENCY}} Hz</td></tr>
	{{/PROBLEM_HARMONIC_SECTION}}
	{{#PROBLEM_TRANSIENT_SECTION}}
	<tr><td>{{PROBLEM_TIMESTEP_LABEL}}</td><td>{{PROBLEM_TIMESTEP}} s</td></tr>
	<tr><td>{{PROBLEM_TIMETOTAL_LABEL}}</td><td>{{PROBLEM_TIMETOTAL}} s</td></tr>
	<tr><td>{{PROBLEM_INITITALCONDITION_LABEL}}</td><td>{{PROBLEM_INITITALCONDITION}}</td></tr>
	{{/PROBLEM_TRANSIENT_SECTION}}
</table>
{{/PROBLEM_SECTION}}

{{#STARTUPSCRIPT_SECTION}}
<h2>{{STARTUPSCRIPT_LABEL}}</h2>
<pre>{{STARTUPSCRIPT}}</pre>
{{/STARTUPSCRIPT_SECTION}}

{{#PHYSICALPROPERTIES_SECTION}}
<h2>{{PHYSICALPROPERTIES_LABEL}}</h2>
<h3>{{MATERIALS_LABEL}}</h3>
	{{#MATERIAL_SECTION}}
	<h4>{{MATERIAL_NAME}}</h4>
	<table>
	{{#MATERIAL_KEYS_SECTION}}
	<tr><td>{{MATERIAL_KEY}}</td><td>{{MATERIAL_KEY_VALUE}} {{MATERIAL_KEY_UNIT}}</td></tr>
	{{/MATERIAL_KEYS_SECTION}}
	</table>
	{{/MATERIAL_SECTION}}
<h3>{{BOUNDARIES_LABEL}}</h3>
	{{#BOUNDARY_SECTION}}
	<h4>{{BOUNDARY_NAME}}</h4>
	<table>
	{{#BOUNDARY_KEYS_SECTION}}
	<tr><td>{{BOUNDARY_KEY}}</td><td>{{BOUNDARY_KEY_VALUE}} {{BOUNDARY_KEY_UNIT}}</td></tr>
	{{/BOUNDARY_KEYS_SECTION}}
	</table>
	{{/BOUNDARY_SECTION}}
{{/PHYSICALPROPERTIES_SECTION}}

{{#GEOMETRY_SECTION}}
<h2>{{GEOMETRY_LABEL}}</h2>
<img src="{{FIGURE_GEOMETRY}}">
<p>{{FIGURE_GEOMETRY_DESCRIPTION}}</p>
<h3>{{GEOMETRY_NODES_LABEL}}</h3>
<table>
	<tr><th>{{GEOMETRY_XLABEL}}</th><th>{{GEOMETRY_YLABEL}}</th></tr>
	{{#GEOMETRY_NODE_SECTION}}
	<tr><td>{{GEOMETRY_NODEX}}</td><td>{{GEOMETRY_NODEY}}</td></tr>
	{{/GEOMETRY_NODE_SECTION}}
</table>
<h3>{{GEOMETRY_EDGES_LABEL}}</h3>
<table>
	<tr><th>{{GEOMETRY_STARTNODE}}</th><th>{{GEOMETRY_ENDNODE}}</th><th>{{GEOMETRY_ANGLE}}</th><th>{{GEOMETRY_MARKER}}</th></tr>
	{{#GEOMETRY_EDGE_SECTION}}
	<tr><td>{{GEOMETRY_EDGE_STARTNODE}}</td><td>{{GEOMETRY_EDGE_ENDNODE}}</td><td>{{GEOMETRY_EDGE_ANGLE}}</td><td>{{GEOMETRY_EDGE_MARKER}}</td></tr>
	{{/GEOMETRY_EDGE_SECTION}}
</table>
<h3>{{GEOMETRY_LABELS_LABEL}}</h3>
<table>
	<tr><th>{{GEOMETRY_XLABEL}}</th><th>{{GEOMETRY_YLABEL}}</th><th>{{GEOMETRY_ARRAY}}</th><th>{{GEOMETRY_MARKER}}</th><th>{{GEOMETRY_POLYNOMIALORDER}}</th></tr>
	{{#GEOMETRY_LABEL_SECTION}}
	<tr><td>{{GEOMETRY_LABELX}}</td><td>{{GEOMETRY_LABELY}}</td><td>{{GEOMETRY_LABEL_ARRAY}}</td><td>{{GEOMETRY_LABEL_MARKER}}</td><td>{{GEOMETRY_LABEL_POLYNOMIALORDER}}</td></tr>
	{{/GEOMETRY_LABEL_SECTION}}
</table>
{{/GEOMETRY_SECTION}}

{{#MESHANDSOLVER_SECTION}}
<h2>{{MESHANDSOLVER_LABEL}}</h2>
<h3>{{SOLVER_LABEL}}</h3>
<table>
	<tr><td>{{SOLVER_NODES_LABEL}}</td><td>{{SOLVER_NODES}}</td></tr>
	<tr><td>{{SOLVER_ELEMENTS_LABEL}}</td><td>{{SOLVER_ELEMENTS}}</td></tr>
	<tr><td>{{SOLVER_DOFS_LABEL}}</td><td>{{SOLVER_DOFS}}</td></tr>
	<tr><td>{{SOLVER_TIMEELAPSED_LABEL}}</td><td>{{SOLVER_TIMEELAPSED}}</td></tr>
	{{#SOLVER_ADAPTIVITY_SECTION}}
	<tr><td>{{SOLVER_ADAPTIVEERROR_LABEL}}</td><td>{{SOLVER_ADAPTIVEERROR}}</td></tr>
	<tr><td>{{SOLVER_ADAPTIVESTEPS_LABEL}}</td><td>{{SOLVER_ADAPTIVESTEPS}}</td></tr>
	{{/SOLVER_ADAPTIVITY_SECTION}}
</table>
{{/MESHANDSOLVER_SECTION}}

{{#FIGURE_SECTION}}
<h2>{{FIGURE_LABEL}}</h2>
<img src="{{FIGURE_MESH}}">
<p>{{FIGURE_MESH_DESCRIPTION}}</p>
<img src="{{FIGURE_ORDER}}">
<p>{{FIGURE_ORDER_DESCRIPTION}}</p>
<img src="{{FIGURE_SCALARVIEW}}">
<p>{{FIGURE_SCALARVIEW_DESCRIPTION}}</p>
<img src="{{FIGURE_CONTOURVIEW}}">
<p>{{FIGURE_CONTOURVIEW_DESCRIPTION}}</p>
<img src="{{FIGURE_VECTORVIEW}}">
<p>{{FIGURE_VECTORVIEW_DESCRIPTION}}</p>
{{/FIGURE_SECTION}}

{{#SCRIPT_SECTION}}
<h2>{{SCRIPT_LABEL}}</h2>
<pre>{{SCRIPT}}</pre>
{{/SCRIPT_SECTION}}

<p id="footer">{{FOOTER}}</p>
</body>
</html>
