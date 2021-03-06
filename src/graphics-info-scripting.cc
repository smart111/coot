
#if defined (USE_PYTHON)
#include "Python.h"  // before system includes to stop "POSIX_C_SOURCE" redefined problems
#endif

#include "compat/coot-sysdep.h"

#include "graphics-info.h"
#include "cc-interface.hh" // for pythonize_command_name()
#include "cc-interface-scripting.hh"
#include "c-interface-scm.hh"

coot::command_arg_t
coot::scripting_function(const std::string &function_name,
			 const std::vector<coot::command_arg_t> &args) {

   coot::command_arg_t r;
   if (graphics_info_t::prefer_python) {
#ifdef USE_PYTHON      
      std::string c = pythonize_command_name(function_name);
      std::vector<std::string> command_strings;
      command_strings.push_back(c);
      for (unsigned int i=0; i<args.size(); i++) { 
	 command_strings.push_back(args[i].as_string());
      }
      std::string s = graphics_info_t::pythonize_command_strings(command_strings);
      PyObject *o = safe_python_command_with_return(s);
      if (o) {
	 if (PyBool_Check(o)) {
	    r.type = coot::command_arg_t::BOOL;
	    r.b = PyInt_AsLong(o);
	 } 
	 if (PyFloat_Check(o)) {
	    r.type = coot::command_arg_t::FLOAT;
	    r.f = PyFloat_AsDouble(o);
	 } 
	 if (PyInt_Check(o)) {
	    r.type = coot::command_arg_t::INT;
	    r.i = PyInt_AsLong(o);
	 } 
	 if (PyString_Check(o)) {
	    r.type = coot::command_arg_t::STRING;
	    r.s = PyString_AsString(o);
	 } 
      } 
#endif      
      
   } else {
#ifdef USE_GUILE
      std::string c = schemize_command_name(function_name);
      std::vector<std::string> command_strings;
      command_strings.push_back(c);
      for (unsigned int i=0; i<args.size(); i++) { 
	 command_strings.push_back(args[i].as_string());
      }
      std::string s = graphics_info_t::schemize_command_strings(command_strings);
      SCM ss = safe_scheme_command(s.c_str());

//       std::cout << "debug:: scripting_function() returns "
//  		<< scm_to_locale_string(display_scm(ss)) 
// 		<< std::endl;
       
      if (scm_is_true(scm_boolean_p(ss))) {
	 r.type = coot::command_arg_t::BOOL;
	 r.b = scm_to_bool(ss);
      }
      if (scm_is_true(scm_integer_p(ss))) {
	 r.type = coot::command_arg_t::INT;
	 r.i = scm_to_int(ss);
      } else { 
	 if (scm_is_true(scm_number_p(ss))) {
	    r.type = coot::command_arg_t::FLOAT;
	    r.f = scm_to_double(ss);
	 }
      }
      if (scm_is_true(scm_string_p(ss))) {
	 r.type = coot::command_arg_t::STRING;
	 r.s = scm_to_locale_string(ss);
      }
#endif 
   } 
   return r;
}


#ifdef USE_PYTHON

PyObject *
graphics_info_t::pyobject_from_graphical_bonds_container(int imol,
							 const graphical_bonds_container &bonds_box) const {

   // imol is added into the atom specs so that the atoms knw the molecule they were part of

   PyObject *r = PyTuple_New(2);

   if (bonds_box.atom_centres_) {
      PyObject *all_atom_positions_py = PyTuple_New(bonds_box.n_consolidated_atom_centres);
      for (int icol=0; icol<bonds_box.n_consolidated_atom_centres; icol++) {
	 PyObject *atom_set_py = PyTuple_New(bonds_box.consolidated_atom_centres[icol].num_points);
	 for (unsigned int i=0; i<bonds_box.consolidated_atom_centres[icol].num_points; i++) {
	    const coot::Cartesian &pt = bonds_box.consolidated_atom_centres[icol].points[i].position;
	    bool is_H_flag     = bonds_box.consolidated_atom_centres[icol].points[i].is_hydrogen_atom;
	    long atom_index = bonds_box.consolidated_atom_centres[icol].points[i].atom_index;
	    PyObject *atom_info_quad_py = PyTuple_New(4);
	    PyObject *coords_py = PyTuple_New(3);
	    PyObject *atom_spec_py = Py_False; // worry about Py_INCREF for atom_spec_py when it's not set
	    std::string s = "attrib-filled-later";
	    // Hmm.
	    // Perhaps we actually want the atom spec (as a python object).
	    // In that case, graphical_bonds_atom_info_t should store the atom, not the residue
	    //
	    mmdb::Atom *at = bonds_box.consolidated_atom_centres[icol].points[i].atom_p;
	    if (at) {
	       coot::atom_spec_t at_spec(at);
	       at_spec.int_user_data = imol;
	       atom_spec_py = atom_spec_to_py(at_spec);

	       // needed?
	       coot::residue_spec_t spec(bonds_box.consolidated_atom_centres[icol].points[i].atom_p);
	       s = spec.format();
	    }

	    PyObject *atom_index_py = PyInt_FromLong(atom_index);
	    PyTuple_SetItem(coords_py, 0, PyFloat_FromDouble(pt.x()));
	    PyTuple_SetItem(coords_py, 1, PyFloat_FromDouble(pt.y()));
	    PyTuple_SetItem(coords_py, 2, PyFloat_FromDouble(pt.z()));
	    PyTuple_SetItem(atom_info_quad_py, 0, coords_py);
	    PyTuple_SetItem(atom_info_quad_py, 1, PyBool_FromLong(is_H_flag));
	    // PyTuple_SetItem(atom_info_quad_py, 2, PyString_FromString(s.c_str())); old
	    PyTuple_SetItem(atom_info_quad_py, 2, atom_spec_py);
	    PyTuple_SetItem(atom_info_quad_py, 3, atom_index_py);
	    PyTuple_SetItem(atom_set_py, i, atom_info_quad_py);
	 }
	 PyTuple_SetItem(all_atom_positions_py, icol, atom_set_py);
      }
      PyTuple_SetItem(r, 0, all_atom_positions_py);
   } else {
      PyObject *empty_py = PyTuple_New(0);
      PyTuple_SetItem(r, 0, empty_py);
   }
   PyObject *bonds_tuple = PyTuple_New(bonds_box.num_colours);
   for (int i=0; i<bonds_box.num_colours; i++) {
      graphical_bonds_lines_list<graphics_line_t> &ll = bonds_box.bonds_[i];
      PyObject *line_set_py = PyTuple_New(bonds_box.bonds_[i].num_lines);
      for (int j=0; j< bonds_box.bonds_[i].num_lines; j++) {
	 const graphics_line_t::cylinder_class_t &cc = ll.pair_list[j].cylinder_class;
	 // int ri = ll.pair_list[j].residue_index; // set to -1 by constructor, overwite if possible
	 int iat_1 = ll.pair_list[j].atom_index_1;
	 int iat_2 = ll.pair_list[j].atom_index_2;

	 PyObject *p0_py   = PyTuple_New(3);
	 PyObject *p1_py   = PyTuple_New(3);
	 PyObject *positions_and_order_py = PyTuple_New(5);
	 PyObject *order_py = PyInt_FromLong(cc);
	 PyObject *atom_index_1_py = PyInt_FromLong(iat_1);
	 PyObject *atom_index_2_py = PyInt_FromLong(iat_2);
	 PyTuple_SetItem(p0_py, 0, PyFloat_FromDouble(ll.pair_list[j].positions.getStart().get_x()));
	 PyTuple_SetItem(p0_py, 1, PyFloat_FromDouble(ll.pair_list[j].positions.getStart().get_y()));
	 PyTuple_SetItem(p0_py, 2, PyFloat_FromDouble(ll.pair_list[j].positions.getStart().get_z()));
	 PyTuple_SetItem(p1_py, 0, PyFloat_FromDouble(ll.pair_list[j].positions.getFinish().get_x()));
	 PyTuple_SetItem(p1_py, 1, PyFloat_FromDouble(ll.pair_list[j].positions.getFinish().get_y()));
	 PyTuple_SetItem(p1_py, 2, PyFloat_FromDouble(ll.pair_list[j].positions.getFinish().get_z()));
	 PyTuple_SetItem(positions_and_order_py, 0, p0_py);
	 PyTuple_SetItem(positions_and_order_py, 1, p1_py);
	 PyTuple_SetItem(positions_and_order_py, 2, order_py);
	 PyTuple_SetItem(positions_and_order_py, 3, atom_index_1_py);
	 PyTuple_SetItem(positions_and_order_py, 4, atom_index_2_py);
	 PyTuple_SetItem(line_set_py, j, positions_and_order_py);
      }
      PyTuple_SetItem(bonds_tuple, i, line_set_py);
   }
   PyTuple_SetItem(r, 1, bonds_tuple);

   return r;
}

PyObject *
graphics_info_t::get_intermediate_atoms_bonds_representation() {

   PyObject *r = Py_False;

   if (moving_atoms_asc) {
      if (moving_atoms_asc->mol) {
	 const graphical_bonds_container &bb = regularize_object_bonds_box;
	 int imol = -1;
	 // maybe imol = imol_moving_atoms;
	 r = pyobject_from_graphical_bonds_container(imol, bb);
      }
   }

   if (PyBool_Check(r)) {
      Py_INCREF(r);
   }

   return r;
}


#endif // USE_PYTHON
