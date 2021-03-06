
void    add_initial_position_restraints(int imol, const std::vector<coot::residue_spec_t> &residue_specs, double weight);
// this removed all initial position restraints, not just those listed.
void remove_initial_position_restraints(int imol, const std::vector<coot::residue_spec_t> &residue_specs);

//! \name  More Refinement Functions
//! \{

//! \brief use unimodal ring torsion restraints (e.g. for carbohydrate pyranose)
void use_unimodal_ring_torsion_restraints(const std::string &res_name);

//! \brief set the Geman-McClure distance alpha value (weight)
void set_refinement_geman_mcclure_alpha(float alpha);

#ifdef USE_GUILE
//! \brief Apply crankshaft peptide rotation optimization to the specified residue
void crankshaft_peptide_rotation_optimization_scm(int imol, SCM residue_spec_smc);
#endif

#ifdef USE_PYTHON
//! \brief Apply crankshaft peptide rotation optimization to the specified residue
void crankshaft_peptide_rotation_optimization_py(int imol, PyObject *residue_spec_py);
#endif

//! \}
