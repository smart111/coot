/* src/graphics-info-state.cc
 * 
 * Copyright 2002, 2003, 2004, 2005, 2006 by Paul Emsley, The University of York
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc.,  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#if defined _MSC_VER
#include <windows.h>
#endif

#include <fstream>

#include <gtk/gtk.h>
#include "interface.h"
#include "graphics-info.h"
#include "c-interface.h"
#include "cc-interface.hh"


// save state
int
graphics_info_t::save_state_file(const std::string &filename) {

   // std::cout << "saving state" << std::endl;
   std::vector<std::string> commands;
   short int il; // il: interface language

#ifdef USE_PYTHON
   il = 2;
#endif // USE_PYTHON
   
#ifdef USE_GUILE
   il = 1;
#endif // USE_GUILE

   std::string comment_str;
   if (il == 1) { 
      comment_str = "; These commands are the saved state of coot.  You can evaluate them\n";
      comment_str += "; using \"Calculate->Run Script...\".";
   } else { 
      comment_str = "# These commands are the saved state of coot.  You can evaluate them\n";
      comment_str += "# using \"Calculate->Run Script...\".";
   }
   commands.push_back(comment_str);
   
   std::vector<std::string> mod_data_start = save_state_data_and_models(il);
   for (unsigned int im=0; im<mod_data_start.size(); im++)
      commands.push_back(mod_data_start[im]);

   // the first thing:  The window position and size:
   if ( (graphics_x_size != 500) && (graphics_y_size != 500) ) {
      commands.push_back(state_command("set-graphics-window-size",
				       graphics_x_size, graphics_y_size, il));
   }
   commands.push_back(state_command("set-graphics-window-position",
				    graphics_x_position, graphics_y_position, il));
   // now the positions of all the major dialogs:
   if (graphics_info_t::model_fit_refine_x_position > -1)
      commands.push_back(state_command("set-model-fit-refine-dialog-position",
				       model_fit_refine_x_position,
				       model_fit_refine_y_position, il));
   if (graphics_info_t::display_manager_x_position > -1)
      commands.push_back(state_command("set-display-control-dialog-position",
				       display_manager_x_position,
				       display_manager_y_position, il));

   if (graphics_info_t::go_to_atom_window_x_position > -1)
      commands.push_back(state_command("set-go-to-atom-window-position",
				       go_to_atom_window_x_position,
				       go_to_atom_window_y_position, il));
   if (graphics_info_t::delete_item_widget_x_position > -1)
      commands.push_back(state_command("set-delete-dialog-position",
				       delete_item_widget_x_position,
				       delete_item_widget_y_position, il));
   if (graphics_info_t::rotate_translate_x_position > -1)
      commands.push_back(state_command("set-rotate-translate-dialog-position",
				       rotate_translate_x_position,
				       rotate_translate_y_position, il));
   if (graphics_info_t::accept_reject_dialog_x_position > -1)
      commands.push_back(state_command("set-accept-reject-dialog-position",
				       accept_reject_dialog_x_position,
				       accept_reject_dialog_y_position, il));
   if (graphics_info_t::ramachandran_plot_x_position > -1)
      commands.push_back(state_command("set-ramachandran-plot-dialog-position",
				       ramachandran_plot_x_position,
				       ramachandran_plot_y_position, il));

   // Virtual trackball
   if (vt_surface_status() == 1)
      commands.push_back(state_command("vt-surface", 1, il));
   else
      commands.push_back(state_command("vt-surface", 2, il));

   if (sticky_sort_by_date)
      commands.push_back(state_command("set-sticky-sort-by-date",il));

   commands.push_back(state_command("set-clipping-front", clipping_front, il));
   commands.push_back(state_command("set-clipping-back",  clipping_back, il));

   commands.push_back(state_command("set-map-radius", box_radius, il));

   unsigned short int v = 4; // 4 dec pl. if float_to_string_using_dec_pl is fixed.
   // a "flag" to use a different function to generate the string from the float
   commands.push_back(state_command("set-iso-level-increment", iso_level_increment, il, v));
   commands.push_back(state_command("set-diff-map-iso-level-increment", diff_map_iso_level_increment, il, v));

   
   commands.push_back(state_command("set-colour-map-rotation-on-read-pdb",
				    rotate_colour_map_on_read_pdb, il));
   commands.push_back(state_command("set-colour-map-rotation-on-read-pdb-flag",
				    rotate_colour_map_on_read_pdb_flag, il));
   commands.push_back(state_command("set-colour-map-rotation-on-read-pdb-c-only-flag",
				    rotate_colour_map_on_read_pdb_c_only_flag, il));
   commands.push_back(state_command("set-swap-difference-map-colours",
				    swap_difference_map_colours, il));

   commands.push_back(state_command("set-background-colour",
				    background_colour[0],
				    background_colour[1],
				    background_colour[2], il));

   // set_density_size_from_widget (not from widget): box_size
   // show unit cell: per molecule (hmm)
   // commands.push_back(state_command("set-aniso-limit", show_aniso_atoms_radius_flag, il));

   
   commands.push_back(state_command("set-symmetry-size", symmetry_search_radius, il));
   // FIXME
   //    commands.push_back(state_command("set-symmetry-whole-chain", symmetry_whole_chain_flag, il));
   commands.push_back(state_command("set-symmetry-atom-labels-expanded", symmetry_atom_labels_expanded_flag, il));
   commands.push_back(state_command("set-active-map-drag-flag", active_map_drag_flag, il));
   commands.push_back(state_command("set-show-aniso", show_aniso_atoms_flag, il));
   commands.push_back(state_command("set-aniso-probability", show_aniso_atoms_probability, il));
   commands.push_back(state_command("set-smooth-scroll-steps", smooth_scroll_steps, il));
   commands.push_back(state_command("set-smooth-scroll-limit", smooth_scroll_limit, il));
   commands.push_back(state_command("set-font-size", atom_label_font_size, il));
   commands.push_back(state_command("set-rotation-centre-size", rotation_centre_cube_size, il));
   commands.push_back(state_command("set-do-anti-aliasing", do_anti_aliasing_flag, il));
   commands.push_back(state_command("set-default-bond-thickness", default_bond_width, il));

   // cif dictionary
   if (cif_dictionary_filename_vec->size() > 0) { 
      for (unsigned int i=0; i<cif_dictionary_filename_vec->size(); i++) {
	 commands.push_back(state_command("read-cif-dictionary", 
					  single_quote((*cif_dictionary_filename_vec)[i]), il));
      }
   }

   // Torsion restraints were set?
   if (do_torsion_restraints)
      commands.push_back(state_command("set-refine-with-torsion-restraints", do_torsion_restraints, il));
   
   std::vector <std::string> command_strings;

   // because the goto_atom_molecule could be 11 with 8 closed
   // molecules, we need to find which one it will be when the script
   // is read in, so we make a count of the molecules... and update
   // local_go_to_atom_mol when we hit the current go_to_atom_molecule.
   int local_go_to_atom_mol = 0;

   // map sampling rate
   if (map_sampling_rate != 1.5) { // only set it if it is not the default
      command_strings.push_back("set-map-sampling-rate");
      command_strings.push_back(float_to_string(map_sampling_rate));
      commands.push_back(state_command(command_strings, il));
      command_strings.clear();
   }

   // goto atom stuff?

   // Now each molecule:
   //
   // We use molecule_count now so that we can use the toggle
   // functions, which need to know the molecule number.
   // molecule_count is the molecule number on execution of this
   // script.  This is a bit sleezy because it relies on the molecule
   // number not being disturbed by pre-existing molecules in coot.
   // Perhaps we should have toggle-last-map-display
   // toggle-last-mol-display toggle-last-mol-active functions.
   // 
   int molecule_count = 0;
   for (int i=0; i<n_molecules; i++) {
      if (molecules[i].has_map() || molecules[i].has_model()) { 
	 // i.e. it was not Closed...
	 command_strings = molecules[i].save_state_command_strings();
	 if (command_strings.size() > 0) {
	    commands.push_back(state_command(command_strings, il));
	    std::vector <std::string> display_strings;
	    std::vector <std::string>  active_strings;
	    // colour
	    display_strings.resize(0);
	    display_strings.push_back("set-molecule-bonds-colour-map-rotation");
	    display_strings.push_back(int_to_string(molecule_count));
	    display_strings.push_back(float_to_string(molecules[i].bonds_colour_map_rotation));
	    commands.push_back(state_command(display_strings, il));
	    if (molecules[i].has_model()) {
	       if (! molecules[i].drawit) {
		  display_strings.resize(0);
		  display_strings.push_back("set-mol-displayed");
		  display_strings.push_back(int_to_string(molecule_count));
		  display_strings.push_back(int_to_string(0));
		  commands.push_back(state_command(display_strings, il));
	       }
	       if (! molecules[i].atom_selection_is_pickable()) {
		  active_strings.push_back("set-mol-active");
		  active_strings.push_back(int_to_string(molecule_count));
		  active_strings.push_back(int_to_string(0));
		  commands.push_back(state_command(active_strings, il));
	       }
	       // symmetry issues:
	       if (molecules[i].symmetry_as_calphas) {
		  // default would be not CAlphas
		  active_strings.clear();
		  active_strings.push_back("symmetry-as-calphas");
		  active_strings.push_back(int_to_string(molecule_count));
		  active_strings.push_back(int_to_string(1));
		  commands.push_back(state_command(active_strings, il));
	       }
	       if (!molecules[i].show_symmetry) {
		  // default would be to show symmetry
		  active_strings.clear();
		  active_strings.push_back("set-show-symmetry-molecule");
		  active_strings.push_back(int_to_string(molecule_count));
		  active_strings.push_back(int_to_string(0));
		  commands.push_back(state_command(active_strings, il));
	       }
	       if (molecules[i].symmetry_colour_by_symop_flag) {
		  // default is not to colour by symop
		  active_strings.clear();
		  active_strings.push_back("set-symmetry-colour-by-symop");
		  active_strings.push_back(int_to_string(molecule_count));
		  active_strings.push_back(int_to_string(1));
		  commands.push_back(state_command(active_strings, il));
	       }
	       if (molecules[i].symmetry_whole_chain_flag) {
		  // default is not to colour by symop
		  active_strings.clear();
		  active_strings.push_back("set-symmetry-whole-chain");
		  active_strings.push_back(int_to_string(molecule_count));
		  active_strings.push_back(int_to_string(1));
		  commands.push_back(state_command(active_strings, il));
	       }
	    }
	    if (molecules[i].has_map()) { 
	       command_strings = molecules[i].set_map_colour_strings();
	       commands.push_back(state_command(command_strings, il));
	       command_strings = molecules[i].get_map_contour_strings();
	       commands.push_back(state_command(command_strings, il));
	       if (molecules[i].contoured_by_sigma_p()) { 
		  command_strings = molecules[i].get_map_contour_sigma_step_strings();
		  commands.push_back(state_command(command_strings, il));
	       }
	       if (! molecules[i].drawit_for_map) {
		  display_strings.resize(0);
		  display_strings.push_back("set-map-displayed");
		  display_strings.push_back(int_to_string(molecule_count));
		  display_strings.push_back(int_to_string(0));
		  commands.push_back(state_command(display_strings, il));
	       }
	    }
	    if (molecules[i].show_unit_cell_flag) {
	       display_strings.clear();
	       display_strings.push_back("set-show-unit-cell");
	       display_strings.push_back(int_to_string(molecule_count));
	       display_strings.push_back(int_to_string(1));
	       commands.push_back(state_command(display_strings, il));
	    } 
	 }

	 if (i==go_to_atom_molecule_)
	    local_go_to_atom_mol = molecule_count;

	 molecule_count++; 
      }
   }

   // last things to do:

   // show symmetry.  Turn this on after molecules have been read so
   // that we don't get the error popup.
   commands.push_back(state_command("set-show-symmetry-master", int(show_symmetry), il));
   // go to atom
   command_strings.resize(0);
   command_strings.push_back("set-go-to-atom-molecule");
   command_strings.push_back(int_to_string(local_go_to_atom_mol));
   commands.push_back(state_command(command_strings, il));

   command_strings.resize(0);
   if (go_to_atom_residue_ != -9999) { // magic unset value
      command_strings.push_back("set-go-to-atom-chain-residue-atom-name");
      command_strings.push_back(single_quote(go_to_atom_chain_));
      command_strings.push_back(int_to_string(go_to_atom_residue_));
      command_strings.push_back(single_quote(go_to_atom_atom_name_));
      commands.push_back(state_command(command_strings, il));
   }
   
   
   // view things: rotation centre and zoom.
   // 
   commands.push_back(state_command("scale-zoom", float(zoom/100.0), il)); 
   commands.push_back(state_command("set-rotation-centre", X(), Y(), Z(), il));

   // the orientation
   command_strings.clear();
   command_strings.push_back("set-view-quaternion");
   command_strings.push_back(float_to_string_using_dec_pl(quat[0], 5));
   command_strings.push_back(float_to_string_using_dec_pl(quat[1], 5));
   command_strings.push_back(float_to_string_using_dec_pl(quat[2], 5));
   command_strings.push_back(float_to_string_using_dec_pl(quat[3], 5));
   commands.push_back(state_command(command_strings, il));


   if (model_fit_refine_dialog)
      commands.push_back(state_command("post-model-fit-refine-dialog", il));
   if (go_to_atom_window)
      commands.push_back(state_command("post-go-to-atom-window", il));
   if (display_control_window_)
      commands.push_back(state_command("post-display-control-window", il));

   short int istat = 0;
   if (! disable_state_script_writing) { 
      istat = write_state(commands, filename);
      if (istat) {
	 std::string s = "Status file ";
	 s += filename;
	 s += " written.";
	 statusbar_text(s);
      } else {
	 std::string s = "WARNING:: failed to write status file ";
	 s += filename;
	 statusbar_text(s);
      }
   }
   return int(istat);
}


std::vector<std::string>
graphics_info_t::save_state_data_and_models(short int lang_flag) const {

   std::vector<std::string> v;
   for (int i=0; i<n_molecules; i++) {
      if (molecules[i].has_map() || molecules[i].has_model()) {
	 std::string s = ";;molecule-info: ";
	 s += molecules[i].name_for_display_manager();
	 v.push_back(s);
      }
   }

   // add the cif dictionaries:
   if (cif_dictionary_filename_vec->size() > 0) { 
      for (unsigned int i=0; i<cif_dictionary_filename_vec->size(); i++) {
	 std::string s = ";;molecule-info: Dictionary: ";
	 s += (*cif_dictionary_filename_vec)[i];
	 v.push_back(s);
      }
   }
   
   // add a hash at the start for python comments
   if (lang_flag == 2) {
      for (unsigned int i=0; i<v.size(); i++) {
	 v[i] = "#" + v[i];
      }
   }
   return v;
}

// return a list of molecule names to be added to the "Run State
// script?" dialog. The state script is filename, of course
// 
std::vector<std::string>
graphics_info_t::save_state_data_and_models(const std::string &filename,
					    short int lang_flag) const { 

   std::vector<std::string> v;
   std::string mol_prefix = ";;molecule-info:";
   if (lang_flag == 2) { // python
      mol_prefix = "#" + mol_prefix; 
   }
   
   std::ifstream f(filename.c_str());
   if (f) {
      std::string s;
      while (! f.eof()) {
	 getline(f, s);
	 // f >> s;
	 std::string ss(s);
	 // xstd::cout << ss << std::endl;
	 std::string::size_type iprefix = ss.find(mol_prefix);
	 if (iprefix != std::string::npos) {
	    std::string::size_type ispace = ss.find(" ");
	    if (ispace != std::string::npos) {
	       std::string m = ss.substr(ispace);
	       // std::cout << "found molecule :" << m << ":" << std::endl;
	       v.push_back(m);
	    } else {
	       // std::cout << "no space found" << std::endl;
	    } 
	 } else {
	    // std::cout << "no prefix found" << std::endl;
	 } 
      }
   }
   f.close();
   return v;
} 



int
graphics_info_t::save_state() {
#ifdef USE_GUILE
   return save_state_file(save_state_file_name);
#else
#ifdef USE_PYTHON   
   return save_state_file("0-coot.state.py");
#else
   return 0;
#endif // USE_PYTHON   
#endif // USE_GUILE
}

std::string 
graphics_info_t::state_command(const std::string &str,
			       int i1,
			       short int state_lang) const {

   std::vector<std::string> command_strings;
   command_strings.push_back(str);
   command_strings.push_back(int_to_string(i1));
   return state_command(command_strings,state_lang);
}

std::string 
graphics_info_t::state_command(const std::string &str,
			       int i1,
			       int i2,
			       short int state_lang) const {

   std::vector<std::string> command_strings;
   command_strings.push_back(str);
   command_strings.push_back(int_to_string(i1));
   command_strings.push_back(int_to_string(i2));
   return state_command(command_strings,state_lang);
}

std::string 
graphics_info_t::state_command(const std::string &str,
			       float f,
			       short int state_lang) const {

   std::vector<std::string> command_strings;
   command_strings.push_back(str);
   command_strings.push_back(float_to_string(f));
   return state_command(command_strings,state_lang);
}

std::string 
graphics_info_t::state_command(const std::string &str,
			       float f,
			       short int state_lang,
			       short unsigned int t) const {

   std::vector<std::string> command_strings;
   command_strings.push_back(str);
   command_strings.push_back(float_to_string_using_dec_pl(f,t));
   return state_command(command_strings,state_lang);
}

std::string 
graphics_info_t::state_command(const std::string &str,
			       float f1,
			       float f2,
			       float f3,
			       short int state_lang) const {

   std::vector<std::string> command_strings;
   command_strings.push_back(str);
   command_strings.push_back(float_to_string(f1));
   command_strings.push_back(float_to_string(f2));
   command_strings.push_back(float_to_string(f3));
   return state_command(command_strings,state_lang);
}


std::string 
graphics_info_t::state_command(const std::string &str, short int state_lang) const {

   std::vector<std::string> command;
   command.push_back(str);
   return state_command(command,state_lang);
}

// command arg interface
std::string
graphics_info_t::state_command(const std::string &str, 
			       const std::string &str2, 
			       short int state_lang) { 

   std::vector<std::string> command;
   command.push_back(str);
   command.push_back(str2);
   return state_command(command,state_lang);

} 

   
std::string 
graphics_info_t::state_command(const std::vector<std::string> &strs,
			       short int state_lang) const {

   std::string command = "";
   
   if (strs.size() > 0) { 
      if (state_lang == coot::STATE_SCM) {
	 command = "(";
	 for (int i=0; i<(int(strs.size())-1); i++) {
	    command += strs[i];
	    command += " ";
	 }
	 command += strs.back();
	 command += ")";
      }
   
      if (state_lang == coot::STATE_PYTHON) {
	 if (strs.size() > 0) { 
	    command = pythonize_command_name(strs[0]);
	    command += " (";
	    for (int i=1; i<(int(strs.size())-1); i++) {
	       command += strs[i];
	       command += ", ";
	    }
	    if (strs.size() > 1) 
	       command += strs.back();
	    command +=  ")";
	 }
      } 
   }
   return command;
} 

// Return success status.
// 
short int 
graphics_info_t::write_state(const std::vector<std::string> &commands,
			     const std::string &filename) const {
   short int istat = 1;
   // std::cout << "writing state" << std::endl;

   std::ofstream f(filename.c_str());

   if (f) {
      for (unsigned int i=0; i<commands.size(); i++) {
	 f << commands[i] << std::endl;
      }
      f.close();
      std::cout << "State file " << filename << " written." << std::endl;

   } else {
      std::cout << "WARNING: couldn't write to state file " << filename
		<< std::endl;
      istat = 0;
   } 

   return istat;
}


int
graphics_info_t::check_for_unsaved_changes() const {

   int iv = 0;
   for (int imol=0; imol<n_molecules; imol++) {
      if (molecules[imol].Have_unsaved_changes_p()) {
	 GtkWidget *dialog = create_unsaved_changes_dialog();
	 fill_unsaved_changes_dialog(dialog);
	 gtk_widget_show(dialog);
	 iv = 1;
	 break;
      }
   }
   return iv;
}

void
graphics_info_t::fill_unsaved_changes_dialog(GtkWidget *dialog) const {

   GtkWidget *vbox = lookup_widget(GTK_WIDGET(dialog),
				   "unsaved_changes_molecule_vbox");

   for (int imol=0; imol<n_molecules; imol++) {
      if (molecules[imol].Have_unsaved_changes_p()) {
	 std::string labelstr = int_to_string(imol);
	 labelstr += "  ";
	 labelstr += molecules[imol].name_;
	 GtkWidget *label = gtk_label_new(labelstr.c_str());
	 gtk_widget_show(label);
	 gtk_misc_set_alignment(GTK_MISC(label), 0.0f, 0.5f);
	 gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
      }
   }
}



/*  ------------------------------------------------------------------------ */
/*                         history                                           */
/*  ------------------------------------------------------------------------ */

void 
graphics_info_t::add_history_command(const std::vector<std::string> &command_strings) { 
   
   history_list.history_strings.push_back(command_strings);
} 

// Being a maniac, I thought to write out the history in scm and py.
// On reflection, I think I might be a nutter (200402xx?).
//
// Well, it seems I am still a nutter, but instead of keeping 2
// histories, I will keep one history that can be formatted as either
// (20050328).
// 
int
graphics_info_t::save_history() const { 

   int istate = 0; 
   std::string history_file_name("0-coot-history");
   std::vector<std::vector<std::string> > raw_command_strings = history_list.history_list();
   std::vector<std::string> languaged_commands;
   if (python_history) { 
      for (unsigned int i=0; i<raw_command_strings.size(); i++)
	 languaged_commands.push_back(pythonize_command_strings(raw_command_strings[i]));
      std::string file = history_file_name + ".py";
      istate =  write_state(languaged_commands, file);
   }
   if (guile_history) {
      languaged_commands.resize(0);
      for (unsigned int i=0; i<raw_command_strings.size(); i++)
	 languaged_commands.push_back(schemize_command_strings(raw_command_strings[i]));
      std::string file = history_file_name + ".scm";
      istate =  write_state(languaged_commands, file);
   }
   return istate;
}

// static
std::string
graphics_info_t::pythonize_command_strings(const std::vector<std::string> &command_strings) {

   std::string command;
   if (command_strings.size() > 0) { 
      command = pythonize_command_name(command_strings[0]);
      command += " (";
      for (int i=1; i<(int(command_strings.size())-1); i++) {
	 command += command_strings[i];
	 command += ", ";
      }
      command += command_strings.back();
      command += ")";
   }
   // std::cout << "INFO:: python history command: " << command << std::endl;
   return command;
}


// static
std::string
graphics_info_t::schemize_command_strings(const std::vector<std::string> &command_strings) {

   std::string command = "(";
   for (int i=0; i<(int(command_strings.size())-1); i++) {
      command += command_strings[i];
      command += " ";
   }
   command += command_strings.back();
   command += ")";
   // std::cout << "INFO:: history command: " << command << std::endl;
   return command;
}

