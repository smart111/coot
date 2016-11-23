
#include "coot-coord-utils.hh"
#include "reduce.hh"

   // Spin-search: OH, SH, NH3, MET methyls

   // OH: SER, TYR, THR

   // look at the Cartesian function position_by_torsion().
   
   // main-chain
   //
   // placeable_main[" H  "] // bisect C(n-1)-N(n) and CA(n)-N(n)

   // CA is placed by tetrahedron - c.f. push_chiral_hydrogen, but use unit vectors to
   //                                    the neighbours

   // Hs on CB: HB2 and HB3:
   // for CYS, ASP, GLU, PHE, HIS, LYS, LEU, MET, ASN, PRO, GLU, ARG, 
   //     SER, TYR, VAL, TRP, TYR
   //    bisect CA-CB and CG-CB, call that b
   //           CA-CG, unit, call that c,
   //           a fragment of bxc and of b gives the delta from CB for HB2 and HB3

   // ALA: CB Hs: HB1, HB2, HB3, BL-A-T: average positions from N-CA-CB-HBx C-CA-CB-HBx

   // CYS: SG H : HB BL-A-T: CA-CB-SG-HG (180)

   // GLU: CG Hs: Bisect as CB Hs, but use CB-CG-CD

   // GLN: CG Hs: Bisect as CB Hs, but use CB-CG-CD

   // PHE Ring Hs: average BL-A-T of ring torsions (180)

   // HIS: average of  BL-A-T of ring torsions (180)

   // ILE: CB HB: on CB from tetrahedron of CA, CG1, CG2.
   //           HG1 and H2: bisect CB, CG, CD1
   //           HD1, HD2, HD3 BL-A-T from CB, CG1, CD1
   //           HG21 HG22, HG33: BL-A-T from CA-CB-CG2-HG2x

   // LYS: CG Hs: Bisect as CB Hs, but use CB-CG-CD
   //      CD Hs: Bisect as CB Hs, but use CG-CD-CE
   //      CE Hs: Bisect as CB Hs, but use CD-CE-NZ
   //      NZ Hs: HZ1,2,3: BL-A-T from  CD-CE-NZ-HZx

   // LEU:

   // MET: CG Hs: Bisect as CB Hs, but use CB-CG-SD
   //         HE1,2,3 BL-A-T from CG, SD, CG

   // PRO: +++

   // GLU: CG Hs: Bisect as CB Hs, but use CB-CG-CD

   // ARG: CG Hs: Bisect as CB Hs, but use CB-CG-CD
   //      CD Hs: Bisect as CB Hs, but use CG-CD-CE
   //      CE Hs: Bisect as CB Hs, but use CD-CE-NZ
   //      +++

   // SER: BL-A-T: CA, CG, OG.

   // THR: CG2 Hs: HG2[1,2,3] BL-A-T: CA, CG, CG.
   //         HG1: BL-A-T on CA, CB, OG (180)  # spin-search

   // VAL: HGxy:  BL-A-T on CA, CB, CG[1,2], HGxy

   // TRP: HD1: average BL-A-T: CD2, CG, CD1 and CE2, NE1, CD1
   //      HE1: average BL-A-T: CG, CD1, NE1 and CD2, CE2, NE1
   //      HZ2: average BL-A-T: CD2, CE2, CZ2 and CZ3, CH2, CZ2
   //      HH2: average BL-A-T: CE2, CZ2, CH2 and CE3, CZ3, CH2
   //      HZ3: average BL-A-T: CZ2, CH2, CZ3 and CD2, CE3, CZ3
   //      HE3: average BL-A-T: CE2, CD2, CE3 and CH2, CZ3, CE3

   // TYR Ring Hs: average BL-A-T of ring torsions (180)
   //          HH: BL-A-T: CE1, CZ, OH # spin-search


clipper::Coord_orth
coot::reduce::position_by_bond_length_angle_torsion(mmdb::Atom *at_1,  // CA
						    mmdb::Atom *at_2,  // CB
						    mmdb::Atom *at_3,  // CG
						    double bl,
						    double angle_rad,
						    double torsion_rad) const {
   
   clipper::Coord_orth at_1_pos = co(at_1);
   clipper::Coord_orth at_2_pos = co(at_2);
   clipper::Coord_orth at_3_pos = co(at_3);

   clipper::Coord_orth new_pos(at_1_pos, at_2_pos, at_3_pos, bl, angle_rad, torsion_rad);

   return new_pos;
}

std::pair<clipper::Coord_orth, clipper::Coord_orth>
coot::reduce::position_pair_by_bisection(mmdb::Atom *at_1,  // CA
					 mmdb::Atom *at_2,  // CB
					 mmdb::Atom *at_3,  // CG
					 double bl,
					 double alpha // angle btwn the two H atoms
					 ) const {

   // bisect the normals
   clipper::Coord_orth at_1_pos = co(at_1);
   clipper::Coord_orth at_2_pos = co(at_2);
   clipper::Coord_orth at_3_pos = co(at_3);

   clipper::Coord_orth vec_1_to_2(at_2_pos-at_1_pos);
   clipper::Coord_orth vec_3_to_2(at_2_pos-at_3_pos);
   clipper::Coord_orth vec_1_to_2_uv(vec_1_to_2.unit());
   clipper::Coord_orth vec_3_to_2_uv(vec_3_to_2.unit());

   // vector from point to mid-atom
   clipper::Coord_orth bisect_delta = 0.5 * (vec_1_to_2_uv + vec_3_to_2_uv);
   clipper::Coord_orth bisect_delta_uv(bisect_delta.unit());

   // vector from CA->CG
   clipper::Coord_orth vec_1_to_3 = at_3_pos - at_1_pos;
   clipper::Coord_orth vec_1_to_3_uv(vec_1_to_3.unit());

   // cpu, the vector between the H atoms is along this vector
   clipper::Coord_orth cpu(clipper::Coord_orth::cross(vec_1_to_3_uv, bisect_delta_uv));

   double scale_fac_bisector = bl * sin(0.5*(M_PI-alpha));
   double scale_fac_cpu      = bl * cos(0.5*(M_PI-alpha));
   
   clipper::Coord_orth Hp1 = at_2_pos + scale_fac_bisector * bisect_delta_uv - scale_fac_cpu * cpu;
   clipper::Coord_orth Hp2 = at_2_pos + scale_fac_bisector * bisect_delta_uv + scale_fac_cpu * cpu;

   return std::pair<clipper::Coord_orth, clipper::Coord_orth> (Hp1, Hp2);
}

clipper::Coord_orth
coot::reduce::position_by_tetrahedron(mmdb::Atom *at_central,
				      mmdb::Atom *at_n_1,
				      mmdb::Atom *at_n_2,
				      mmdb::Atom *at_n_3,
				      double bl) const {

   clipper::Coord_orth at_c_pos = co(at_central);
   clipper::Coord_orth at_n1_pos = co(at_n_1);
   clipper::Coord_orth at_n2_pos = co(at_n_2);
   clipper::Coord_orth at_n3_pos = co(at_n_3);
   clipper::Coord_orth vec_1_to_c(at_c_pos-at_n1_pos);
   clipper::Coord_orth vec_2_to_c(at_c_pos-at_n2_pos);
   clipper::Coord_orth vec_3_to_c(at_c_pos-at_n3_pos);
   clipper::Coord_orth vec_1_to_c_uv(vec_1_to_c.unit());
   clipper::Coord_orth vec_2_to_c_uv(vec_2_to_c.unit());
   clipper::Coord_orth vec_3_to_c_uv(vec_3_to_c.unit());

   clipper::Coord_orth under_pos = 0.3333333333 *
      (vec_1_to_c_uv + vec_2_to_c_uv + vec_3_to_c_uv);
   clipper::Coord_orth under_pos_uv(under_pos.unit());

   clipper::Coord_orth H_pos = at_c_pos + bl * under_pos_uv;
   return H_pos;
}

void
coot::reduce::add_hydrogen_atoms() {

   add_riding_hydrogens();

}

void
coot::reduce::add_riding_hydrogens() {

   int imod = 1;
   mmdb::Model *model_p = mol->GetModel(imod);
   if (model_p) {
      mmdb::Chain *chain_p;
      int n_chains = model_p->GetNumberOfChains();
      for (int ichain=0; ichain<n_chains; ichain++) {
	 chain_p = model_p->GetChain(ichain);
	 int nres = chain_p->GetNumberOfResidues();
	 mmdb::Residue *residue_p;
	 mmdb::Residue *residue_prev_p;
	 mmdb::Atom *at;
	 for (int ires=1; ires<nres; ires++) {
	    residue_p      = chain_p->GetResidue(ires);
	    residue_prev_p = chain_p->GetResidue(ires-1);
	    // what about strange missing residues - where we can place the CA HA
	    // (but not the N's H).
	    if (residue_p && residue_prev_p) {
	       add_riding_hydrogens(residue_p, residue_prev_p);
	    }
	 }
      }
   }
}

// only call this with a valid residue_p
void
coot::reduce::add_riding_hydrogens(mmdb::Residue *residue_p, mmdb::Residue *residue_prev_p) {

   std::string res_name = residue_p->GetResName();
   double bl = 0.97;
   if (res_name == "ALA") {
      add_main_chain_hydrogens(residue_p, residue_prev_p);
      torsion_info_t torsion_1(" N  ", " CA ", " CB ", bl, 109, 180);
      add_methyl_Hs(" HB1", " HB2", " HB3", torsion_1, residue_p);
   }
   if (res_name == "CYS") {
      add_main_chain_hydrogens(residue_p, residue_prev_p);
      add_2_hydrogens(" HB2", " HB3", " CA ", " CB ", " SG ", bl, 107, residue_p);
   }
   if (res_name == "ASP") {
      add_main_chain_hydrogens(residue_p, residue_prev_p);
      add_2_hydrogens(" HB3", " HB3", " CA ", " CB ", " CG ", bl, 107, residue_p);
   }
   if (res_name == "GLU") {
      add_main_chain_hydrogens(residue_p, residue_prev_p);
      add_2_hydrogens(" HB2", " HB3", " CA ", " CB ", " CG ", bl, 107, residue_p);
   }
   if (res_name == "PHE") {
      add_main_chain_hydrogens(residue_p, residue_prev_p);
      add_2_hydrogens(" HB2", " HB3", " CA ", " CB ", " CG ", bl, 107, residue_p);
   }
   if (res_name == "GLY") {
      add_main_chain_hydrogens(residue_p, residue_prev_p, true);
   }
   if (res_name == "HIS") {
      add_main_chain_hydrogens(residue_p, residue_prev_p);
      add_2_hydrogens(" HB2", " HB3", " CA ", " CB ", " CG ", bl, 107, residue_p);
   }
   if (res_name == "ILE") {
      add_main_chain_hydrogens(residue_p, residue_prev_p);
      add_2_hydrogens("HG12", "HG13", " CB ", " CG1", " CD1", bl, 107, residue_p);
      torsion_info_t ti(" CB ", " CG1", " CD1", bl, 109, 180);
      add_methyl_Hs(" HD1", " HD2", " HD3", ti, residue_p);
      torsion_info_t t2(" CA ", " CB ", " CG2", bl, 109, 180);
      add_methyl_Hs(" HG1", " HG2", " HG3", t2, residue_p);
      add_tetrahedral_hydrogen(" HB ", " CB ", " CA ", " CG1", " CG2", bl, residue_p);
   }
   if (res_name == "LYS") {
      add_main_chain_hydrogens(residue_p, residue_prev_p);
      add_2_hydrogens(" HB2", " HB3", " CA ", " CB ", " CG ", bl, 107, residue_p);
      add_2_hydrogens(" HG2", " HG3", " CB ", " CG ", " CD ", bl, 107, residue_p);
      add_2_hydrogens(" HD2", " HD3", " CG ", " CD ", " CE ", bl, 107, residue_p);
      add_2_hydrogens(" HE2", " HE3", " CD ", " CE ", " NZ ", bl, 107, residue_p);
      torsion_info_t ti(" CD ", " CE ", " NZ ", bl, 109, 180);
      add_methyl_Hs(" HZ1", " HZ2", " HZ3", ti, residue_p);
   }
   if (res_name == "LEU") {
      add_main_chain_hydrogens(residue_p, residue_prev_p);
      add_2_hydrogens(" HB2", " HB3", " CA ", " CB ", " CG ", bl, 107, residue_p);
      torsion_info_t t1(" CB ", " CG ", " CD1", bl, 109, 180);
      torsion_info_t t2(" CB ", " CG ", " CD2", bl, 109, 180);
      add_methyl_Hs("HD11", "HD12", "HD13", t1, residue_p);
      add_methyl_Hs("HD21", "HD22", "HD23", t2, residue_p);
      add_tetrahedral_hydrogen(" HG ", " CG ", " CB ", " CD1", " CD2", bl, residue_p);
   }
   if (res_name == "MET") {
      add_main_chain_hydrogens(residue_p, residue_prev_p);
      add_2_hydrogens(" HB2", " HB3", " CA ", " CB ", " CG ", bl, 107, residue_p);
      add_2_hydrogens(" HG2", " HG3", " CB ", " CG ", " SD ", bl, 107, residue_p);
   }
   if (res_name == "MSE") {
      add_main_chain_hydrogens(residue_p, residue_prev_p);
      add_2_hydrogens(" HB1", " HB2", " CA ", " CB ", " CG ", bl, 107, residue_p);
   }
   if (res_name == "ASN") {
      add_main_chain_hydrogens(residue_p, residue_prev_p);
      add_2_hydrogens(" HB2", " HB3", " CA ", " CB ", " CG ", bl, 107, residue_p);
   }
   if (res_name == "PRO") {
      add_main_chain_hydrogens(residue_p, residue_prev_p);
      add_2_hydrogens(" HB2", " HB3", " CA ", " CB ", " CG ", bl, 107, residue_p);
      add_2_hydrogens(" HG2", " HG3", " CB ", " CG ", " CD ", bl, 107, residue_p);
      add_2_hydrogens(" HD2", " HD3", " CG ", " CD ", " N  ", bl, 107, residue_p);
   }
   if (res_name == "GLN") {
      add_main_chain_hydrogens(residue_p, residue_prev_p);
      add_2_hydrogens(" HB2", " HB3", " CA ", " CB ", " CG ", bl, 107, residue_p);
      add_2_hydrogens(" HG2", " HG3", " CB ", " CG ", " CD ", bl, 107, residue_p);
   }
   if (res_name == "ARG") {
      add_main_chain_hydrogens(residue_p, residue_prev_p);
      add_2_hydrogens(" HB2", " HB3", " CA ", " CB ", " CG ", bl, 107, residue_p);
      add_2_hydrogens(" HG2", " HG3", " CB ", " CG ", " CD ", bl, 107, residue_p);
      add_2_hydrogens(" HD2", " HD3", " CG ", " CD ", " NE ", bl, 107, residue_p);
   }
   if (res_name == "SER") {
      add_main_chain_hydrogens(residue_p, residue_prev_p);
   }
   if (res_name == "THR") {
      add_main_chain_hydrogens(residue_p, residue_prev_p);
      torsion_info_t ti(" CA ", " CB ", " OG ", bl, 109, 180);
      add_methyl_Hs(" HG1", " HG2", " HG3", ti, residue_p);
   }
   if (res_name == "VAL") {
      add_main_chain_hydrogens(residue_p, residue_prev_p);
      torsion_info_t t1(" CA ", " CB ", " CG1 ", bl, 109, 180);
      torsion_info_t t2(" CA ", " CB ", " CG2 ", bl, 109, 180);
      add_methyl_Hs("HG11", "HG12", "HG13", t1, residue_p);
      add_methyl_Hs("HG21", "HG22", "HG23", t2, residue_p);
      add_tetrahedral_hydrogen(" HB ", " CB ", " CA ", " CG1", " CG2", bl, residue_p);
   }
   if (res_name == "TRP") {
      add_main_chain_hydrogens(residue_p, residue_prev_p);
      add_2_hydrogens(" HB2", " HB3", " CA ", " CB ", " CG ", bl, 107, residue_p);
   }
   if (res_name == "TYR") {
      add_main_chain_hydrogens(residue_p, residue_prev_p);
      add_2_hydrogens(" HB2", " HB3", " CA ", " CB ", " CG ", bl, 107, residue_p);
   }
}

// is_gly is false by default
void
coot::reduce::add_main_chain_hydrogens(mmdb::Residue *residue_p, mmdb::Residue *residue_prev_p,
				       bool is_gly) {
   add_main_chain_H(residue_p, residue_prev_p);
   if (is_gly) {
      // GLY Hydrogens
   } else {
      add_main_chain_HA(residue_p);
   }
}


// The H on the N
void
coot::reduce::add_main_chain_H(mmdb::Residue *residue_p, mmdb::Residue *residue_prev_p) {

   // Try position by torsion based on O-C-N-H
      double bl = 0.86;
   if (residue_p->isNTerminus()) {
      // NH3+ - needs spin search - these are not riding
   } else {
      std::vector<std::string> alt_confs = util::get_residue_alt_confs(residue_p);
      for (unsigned int i=0; i<alt_confs.size(); i++) {
	 mmdb::Atom *at_ca     = residue_p->GetAtom(" CA ", 0, alt_confs[i].c_str());
	 mmdb::Atom *at_n      = residue_p->GetAtom(" N  ", 0, alt_confs[i].c_str());
	 mmdb::Atom *at_c_prev = residue_prev_p->GetAtom(" C  ", 0, alt_confs[i].c_str());
	 mmdb::Atom *at_o_prev = residue_prev_p->GetAtom(" O  ", 0, alt_confs[i].c_str());
	 if (at_ca && at_n && at_c_prev && at_o_prev) {

	    clipper::Coord_orth at_c_pos  = co(at_c_prev);
	    clipper::Coord_orth at_o_pos  = co(at_o_prev);
	    clipper::Coord_orth at_n_pos  = co(at_n);
	    clipper::Coord_orth at_ca_pos = co(at_ca);
	    double angle = clipper::Util::d2rad(125.0);
	    // clipper::Coord_orth H_pos(at_o_pos, at_c_pos, at_n_pos, bl, angle, M_PI);
	    clipper::Coord_orth H_pos(at_ca_pos, at_c_pos, at_n_pos, bl, angle, M_PI);
	    mmdb::realtype bf = at_n->tempFactor;
	    add_hydrogen_atom(" H  ", H_pos, bf, residue_p);
	    
	 }
      }
   }
}

// The H on the CA
void
coot::reduce::add_main_chain_HA(mmdb::Residue *residue_p) {

   double bl = 0.97;

   // PDBv3 FIXME
   std::vector<std::string> alt_confs = util::get_residue_alt_confs(residue_p);
   for (unsigned int i=0; i<alt_confs.size(); i++) {
      mmdb::Atom *at_ca = residue_p->GetAtom(" CA ", 0, alt_confs[i].c_str());
      mmdb::Atom *at_n1 = residue_p->GetAtom(" C  ", 0, alt_confs[i].c_str());
      mmdb::Atom *at_n2 = residue_p->GetAtom(" N  ", 0, alt_confs[i].c_str());
      mmdb::Atom *at_n3 = residue_p->GetAtom(" CB ", 0, alt_confs[i].c_str());
      if (at_ca && at_n1 && at_n2 && at_n3) {
	 clipper::Coord_orth pos = position_by_tetrahedron(at_ca, at_n1, at_n2, at_n3, bl);
	 mmdb::realtype bf = at_ca->tempFactor;
	 add_hydrogen_atom(" HA ", pos, bf, residue_p);
      }
   }
}

void
coot::reduce::add_hydrogen_atom(std::string atom_name, clipper::Coord_orth &pos,
				mmdb::realtype bf, mmdb::Residue *residue_p) {

   mmdb::Atom *new_H = new mmdb::Atom;
   new_H->SetAtomName(atom_name.c_str());
   new_H->SetElementName(" H"); // PDBv3 FIXME
   new_H->SetCoordinates(pos.x(), pos.y(), pos.z(), 1.0, bf);
   residue_p->AddAtom(new_H);
}


void 
coot::reduce::add_methyl_Hs(const std::string &at_name_1,  // HB1 (for example)
			    const std::string &at_name_2,  // HB2 + 120 degress
			    const std::string &at_name_3,  // HB3 - 120 degree
			    torsion_info_t torsion_1, torsion_info_t torsion_2,
			    mmdb::Residue *residue_p) {

   std::vector<std::string> alt_confs = util::get_residue_alt_confs(residue_p);
   for (unsigned int i=0; i<alt_confs.size(); i++) {
      clipper::Coord_orth p11;
      clipper::Coord_orth p12;
      clipper::Coord_orth p13;
      clipper::Coord_orth p21;
      clipper::Coord_orth p22;
      clipper::Coord_orth p23;
      bool have_1 = false;
      bool have_2 = false;
      mmdb::Atom *at_1 = residue_p->GetAtom(torsion_1.at_name_1.c_str(), 0, alt_confs[i].c_str());
      mmdb::Atom *at_2 = residue_p->GetAtom(torsion_1.at_name_2.c_str(), 0, alt_confs[i].c_str());
      mmdb::Atom *at_3 = residue_p->GetAtom(torsion_1.at_name_3.c_str(), 0, alt_confs[i].c_str());
      if (at_1 && at_2 && at_3) {
	 have_1 = true;
	 p11 = position_by_bond_length_angle_torsion(at_1, at_2, at_3,
						     torsion_1.bond_length,
						     clipper::Util::d2rad(torsion_1.angle_deg),
						     clipper::Util::d2rad(torsion_1.torsion_deg));
	 p12 = position_by_bond_length_angle_torsion(at_1, at_2, at_3,
						     torsion_1.bond_length,
						     clipper::Util::d2rad(torsion_1.angle_deg),
						     clipper::Util::d2rad(torsion_1.torsion_deg + 120));
	 p13 = position_by_bond_length_angle_torsion(at_1, at_2, at_3,
						     torsion_1.bond_length,
						     clipper::Util::d2rad(torsion_1.angle_deg),
						     clipper::Util::d2rad(torsion_1.torsion_deg - 120));
      }
      at_1 = residue_p->GetAtom(torsion_2.at_name_1.c_str(), 0, alt_confs[i].c_str());
      at_2 = residue_p->GetAtom(torsion_2.at_name_2.c_str(), 0, alt_confs[i].c_str());
      at_3 = residue_p->GetAtom(torsion_2.at_name_3.c_str(), 0, alt_confs[i].c_str());
      if (at_1 && at_2 && at_3) {
	 have_2 = true;
	 p21 = position_by_bond_length_angle_torsion(at_1, at_2, at_3,
						     torsion_2.bond_length,
						    clipper::Util::d2rad(torsion_2.angle_deg),
						    clipper::Util::d2rad(torsion_2.torsion_deg));
	 p22 = position_by_bond_length_angle_torsion(at_1, at_2, at_3,
						     torsion_2.bond_length,
						     clipper::Util::d2rad(torsion_2.angle_deg),
						     clipper::Util::d2rad(torsion_2.torsion_deg + 120));
	 p23 = position_by_bond_length_angle_torsion(at_1, at_2, at_3,
						     torsion_2.bond_length,
						     clipper::Util::d2rad(torsion_2.angle_deg),
						     clipper::Util::d2rad(torsion_2.torsion_deg - 120));
      }

      // can construct result differently if we only have either p1 or p2.  THis will do for now.
      // 
      if (have_1) {
	 // this may make them too short. Hmm.
// 	 clipper::Coord_orth pav_1 = 0.5 * (p11 + p21);
// 	 clipper::Coord_orth pav_2 = 0.5 * (p12 + p22);
// 	 clipper::Coord_orth pav_3 = 0.5 * (p13 + p23);
	 
	 clipper::Coord_orth pav_1 = p11;
	 clipper::Coord_orth pav_2 = p12;
	 clipper::Coord_orth pav_3 = p13;
	 mmdb::realtype bf = at_3->tempFactor;
	 add_hydrogen_atom(at_name_1, pav_1, bf, residue_p);
	 add_hydrogen_atom(at_name_2, pav_2, bf, residue_p);
	 add_hydrogen_atom(at_name_3, pav_3, bf, residue_p);
      }
   }
}
void 
coot::reduce::add_methyl_Hs(const std::string &at_name_1,  // HB1 (for example)
			    const std::string &at_name_2,  // HB2 + 120 degress
			    const std::string &at_name_3,  // HB3 - 120 degree
			    torsion_info_t torsion_1,
			    mmdb::Residue *residue_p) {

   std::vector<std::string> alt_confs = util::get_residue_alt_confs(residue_p);
   for (unsigned int i=0; i<alt_confs.size(); i++) {
      clipper::Coord_orth p11;
      clipper::Coord_orth p12;
      clipper::Coord_orth p13;
      bool have_1 = false;
      mmdb::Atom *at_1 = residue_p->GetAtom(torsion_1.at_name_1.c_str(), 0, alt_confs[i].c_str());
      mmdb::Atom *at_2 = residue_p->GetAtom(torsion_1.at_name_2.c_str(), 0, alt_confs[i].c_str());
      mmdb::Atom *at_3 = residue_p->GetAtom(torsion_1.at_name_3.c_str(), 0, alt_confs[i].c_str());
      if (at_1 && at_2 && at_3) {
	 have_1 = true;
	 p11 = position_by_bond_length_angle_torsion(at_1, at_2, at_3,
						     torsion_1.bond_length,
						     clipper::Util::d2rad(torsion_1.angle_deg),
						     clipper::Util::d2rad(torsion_1.torsion_deg));
	 p12 = position_by_bond_length_angle_torsion(at_1, at_2, at_3,
						     torsion_1.bond_length,
						     clipper::Util::d2rad(torsion_1.angle_deg),
						     clipper::Util::d2rad(torsion_1.torsion_deg + 120));
	 p13 = position_by_bond_length_angle_torsion(at_1, at_2, at_3,
						     torsion_1.bond_length,
						     clipper::Util::d2rad(torsion_1.angle_deg),
						     clipper::Util::d2rad(torsion_1.torsion_deg - 120));
      }
      if (have_1) {
	 
	 clipper::Coord_orth pav_1 = p11;
	 clipper::Coord_orth pav_2 = p12;
	 clipper::Coord_orth pav_3 = p13;
	 mmdb::realtype bf = at_3->tempFactor;
	 add_hydrogen_atom(at_name_1, pav_1, bf, residue_p);
	 add_hydrogen_atom(at_name_2, pav_2, bf, residue_p);
	 add_hydrogen_atom(at_name_3, pav_3, bf, residue_p);
      }
   }
}


void
coot::reduce::add_2_hydrogens(const std::string &H_at_name_1,
			      const std::string &H_at_name_2,
			      const std::string &at_name_1,
			      const std::string &at_name_2,
			      const std::string &at_name_3,
			      double bond_length,
			      double angle_between_Hs, // in degrees
			      mmdb::Residue *residue_p) {

   std::vector<std::string> alt_confs = util::get_residue_alt_confs(residue_p);
   for (unsigned int i=0; i<alt_confs.size(); i++) {
      
      mmdb::Atom *at_1 = residue_p->GetAtom(at_name_1.c_str(), 0, alt_confs[i].c_str());
      mmdb::Atom *at_2 = residue_p->GetAtom(at_name_2.c_str(), 0, alt_confs[i].c_str());
      mmdb::Atom *at_3 = residue_p->GetAtom(at_name_3.c_str(), 0, alt_confs[i].c_str());
      if (at_1 && at_2 && at_3) {
	 std::pair<clipper::Coord_orth, clipper::Coord_orth> Hs =
	    position_pair_by_bisection(at_1, at_2, at_3, bond_length,
				       clipper::Util::d2rad(angle_between_Hs));
	 mmdb::realtype bf = at_2->tempFactor;
	 add_hydrogen_atom(H_at_name_1, Hs.first,  bf, residue_p);
	 add_hydrogen_atom(H_at_name_2, Hs.second, bf, residue_p);
      } else {
	 std::cout << "Residue " << residue_spec_t(residue_p) << " " << residue_p->GetResName()
		   << " alt-conf \"" << alt_confs[i] << "\"" << std::endl;
	 std::cout << "Fail to add " << H_at_name_1 << " " << H_at_name_2 << " at_1: " << at_1 << std::endl;
	 std::cout << "            " << H_at_name_1 << " " << H_at_name_2 << " at_2: " << at_2 << std::endl;
	 std::cout << "            " << H_at_name_1 << " " << H_at_name_2 << " at_3: " << at_3 << std::endl;
      }
   }
}


void
coot::reduce::add_tetrahedral_hydrogen(const std::string &H_at_name,
				       const std::string &at_central_name,
				       const std::string &neighb_at_name_1,
				       const std::string &neighb_at_name_2,
				       const std::string &neighb_at_name_3,
				       double bond_length,
				       mmdb::Residue *residue_p) {

   
   std::vector<std::string> alt_confs = util::get_residue_alt_confs(residue_p);
   for (unsigned int i=0; i<alt_confs.size(); i++) {
      mmdb::Atom *at_central = residue_p->GetAtom(at_central_name.c_str(), 0, alt_confs[i].c_str());
      mmdb::Atom *at_n_1 = residue_p->GetAtom(neighb_at_name_1.c_str(), 0, alt_confs[i].c_str());
      mmdb::Atom *at_n_2 = residue_p->GetAtom(neighb_at_name_2.c_str(), 0, alt_confs[i].c_str());
      mmdb::Atom *at_n_3 = residue_p->GetAtom(neighb_at_name_3.c_str(), 0, alt_confs[i].c_str());
      if (at_central && at_n_1 && at_n_2 && at_n_3) {
	 clipper::Coord_orth H_pos = position_by_tetrahedron(at_central, at_n_1, at_n_2, at_n_3,
							     bond_length);
	 mmdb::realtype bf = at_central->tempFactor;
	 add_hydrogen_atom(H_at_name, H_pos,  bf, residue_p);
      }
   }
}