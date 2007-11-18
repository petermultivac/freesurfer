/**
 * @file  mri_modify.cpp
 * @brief modify direction cosine info on the volume.
 *
 * also allows changing the 'xform' filename
 */
/*
 * Original Author: Yasunari Tosa
 * CVS Revision Info:
 *    $Author: nicks $
 *    $Date: 2007/11/18 03:06:17 $
 *    $Revision: 1.5.2.1 $
 *
 * Copyright (C) 2002-2007,
 * The General Hospital Corporation (Boston, MA). 
 * All rights reserved.
 *
 * Distribution, usage and copying of this software is covered under the
 * terms found in the License Agreement file named 'COPYING' found in the
 * FreeSurfer source code root directory, and duplicated here:
 * https://surfer.nmr.mgh.harvard.edu/fswiki/FreeSurferOpenSourceLicense
 *
 * General inquiries: freesurfer@nmr.mgh.harvard.edu
 * Bug reports: analysis-bugs@nmr.mgh.harvard.edu
 *
 */


#include <iostream>
#include <iomanip>

extern "C" {
#include "macros.h"
#include "mri.h"
#include "transform.h"
#include "version.h"
  const char *Progname = "mri_modify";
}

using namespace std;

static double gflip_angle=0;
static float gte=0;
static float gtr=0;
static float gti=0;
static char new_transform_fname[STRLEN];

void print_usage() {
  cout << "Usage: mri_modify <-xras xr xa xs> <-yras yr ya ys> <-zras zr za zs> <-cras cr ca cs> \\ " << endl;
  cout << "                  <-xsize size> <-ysize size> <-zsize size> \\ " << endl;
  cout << "                  <-tr recoverytime> <-te echotime> <-ti inversiontime> <-fa angledegree> \\ " << endl;
  cout << "                  <-xform new_file_name> \\ " << endl;
  cout << "                  involume outvolume" << endl;
}

int get_option(int argc, char *argv[], VOL_GEOM &vg) {
  int  nargs = 0 ;
  char *option ;
  option = argv[1] + 1 ;            /* past '-' */
  if (!strcmp(option, "-help")) {
    print_usage();
    exit(0);
  } else if (!strcmp(option, "xras")) {
    vg.x_r = atof(argv[2]);
    vg.x_a = atof(argv[3]);
    vg.x_s = atof(argv[4]);
    nargs=3;
  } else if (!strcmp(option, "yras")) {
    vg.y_r = atof(argv[2]);
    vg.y_a = atof(argv[3]);
    vg.y_s = atof(argv[4]);
    nargs=3;
  } else if (!strcmp(option, "zras")) {
    vg.z_r = atof(argv[2]);
    vg.z_a = atof(argv[3]);
    vg.z_s = atof(argv[4]);
    nargs=3;
  } else if (!strcmp(option, "cras")) {
    vg.c_r = atof(argv[2]);
    vg.c_a = atof(argv[3]);
    vg.c_s = atof(argv[4]);
    nargs=3;
  } else if (!strcmp(option, "xsize")) {
    vg.xsize = atof(argv[2]);
    nargs=1;
  } else if (!strcmp(option, "ysize")) {
    vg.ysize = atof(argv[2]);
    nargs=1;
  } else if (!strcmp(option, "zsize")) {
    vg.zsize = atof(argv[2]);
    nargs=1;
  } else if (!strcmp(option, "tr")) {
    gtr=atof(argv[2]);
    nargs=1;
  } else if (!strcmp(option, "te")) {
    gte=atof(argv[2]);
    nargs=1;
  } else if (!strcmp(option, "ti")) {
    gti=atof(argv[2]);
    nargs=1;
  } else if (!strcmp(option, "fa")) {
    // mri stores it as radian
    gflip_angle=RADIANS(atof(argv[2]));
    nargs=1;
  } else if (!strcmp(option, "xform")) {
    // get new transform file name
    strcpy(new_transform_fname,argv[2]);
  }
  return nargs;
}

int main(int argc, char *argv[]) {
  int nargs;
  /* rkt: check for and handle version tag */
  nargs = handle_version_option 
    (argc, argv, 
     "$Id: mri_modify.cpp,v 1.5.2.1 2007/11/18 03:06:17 nicks Exp $", 
     "$Name:  $");
  if (nargs && argc - nargs == 1)
    exit (0);
  argc -= nargs;

  VOL_GEOM vg = {}; // all values are initialized to be zero to detect change
  new_transform_fname[0]=0; // null xform filename (assume no change)

  // argument handling
  for ( ; argc > 1 && ISOPTION(*argv[1]) ; argc--, argv++) {
    nargs = get_option(argc, argv, vg) ;
    argc -= nargs ;
    argv += nargs ;
  }
  if (argc < 3) {
    print_usage();
    exit(-1);
  }

  char *invol = argv[argc-2];
  char *outvol = argv[argc-1];

  cerr << "Input volume  is : " << invol << endl;
  cerr << "Output volume is : " << outvol << endl;

  MRI *mri = MRIread(invol);
  if (!mri) {
    cerr << "could not open " << invol << endl;
    exit(-1);
  }
  //////////////////////////////////////////////
  VOL_GEOM vgIn;
  getVolGeom(mri, &vgIn);
  VOL_GEOM vgOut;
  copyVolGeom(&vgIn, &vgOut);
  // modify only those which are non-zero in the options
  // x_ras
  if (!FZERO(vg.x_r) || !FZERO(vg.x_a) || !FZERO(vg.x_s)) {
    // check consistency
    if (!FZERO(vg.x_r*vg.x_r+vg.x_a*vg.x_a+vg.x_s*vg.x_s - 1)) {
      cerr << "x_(ras) must have the unit length" << endl;
      exit(-1);
    }
    vgOut.x_r = vg.x_r;
    vgOut.x_a = vg.x_a;
    vgOut.x_s = vg.x_s;
  }
  // y_ras
  if (!FZERO(vg.y_r) || !FZERO(vg.y_a) || !FZERO(vg.y_s)) {
    // check consistency
    if (!FZERO(vg.y_r*vg.y_r+vg.y_a*vg.y_a+vg.y_s*vg.y_s - 1)) {
      cerr << "y_(ras) must have the unit length" << endl;
      exit(-1);
    }
    vgOut.y_r = vg.y_r;
    vgOut.y_a = vg.y_a;
    vgOut.y_s = vg.y_s;
  }
  // z_ras
  if (!FZERO(vg.z_r) || !FZERO(vg.z_a) || !FZERO(vg.z_s)) {
    // check consistency
    if (!FZERO(vg.z_r*vg.z_r+vg.z_a*vg.z_a+vg.z_s*vg.z_s - 1)) {
      cerr << "z_(ras) must have the unit length" << endl;
      exit(-1);
    }
    vgOut.z_r = vg.z_r;
    vgOut.z_a = vg.z_a;
    vgOut.z_s = vg.z_s;
  }
  // c_ras
  if (!FZERO(vg.c_r) || !FZERO(vg.c_a) || !FZERO(vg.c_s)) {
    vgOut.c_r = vg.c_r;
    vgOut.c_a = vg.c_a;
    vgOut.c_s = vg.c_s;
  }
  // xsize
  if (!FZERO(vg.xsize))
    vgOut.xsize = vg.xsize;
  // ysize
  if (!FZERO(vg.ysize))
    vgOut.ysize = vg.ysize;
  // zsize
  if (!FZERO(vg.zsize))
    vgOut.zsize = vg.zsize;

  useVolGeomToMRI(&vgOut,mri);

  // now TR, TE, TI, flip_angle
  if (gtr)
    mri->tr = gtr;
  if (gte)
    mri->te = gte;
  if (gti)
    mri->ti = gti;
  if (gflip_angle)
    mri->flip_angle = gflip_angle;

  // stuff-in the new transform filename, if one was grabbed from command-line
  if (new_transform_fname[0])
    strcpy(mri->transform_fname,new_transform_fname);

  // write it out
  MRIwrite(mri, outvol);

  return 0;
}
