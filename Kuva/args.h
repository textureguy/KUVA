/*******************************
*
* Kuva - Graph cut texturing
* 
* From:
*      V.Kwatra, A.Schödl, I.Essa, G.Turk, A.Bobick, 
*      Graphcut Textures: Image and Video Synthesis Using Graph Cuts
*      http://www.cc.gatech.edu/cpl/projects/graphcuttextures/
*
* JP <jeanphilippe.aumasson@gmail.com>
*
* args.hpp
*
* 01/2006
*
*******************************/
/*

Copyright Jean-Philippe Aumasson, 2005, 2006

Kuva is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/


#ifndef K_ARGS
#define K_ARGS


#include "main.h"
#include "graph.h"

#include <stdlib.h>
#include <math.h>

#define MAX_UINT32 0xffffffff
#define MAX_SHORT  16384
#define COST_REDUC 20

/* number of calls to place() after which, if no pixel added,
use a fastest place function (ie placeRandom) */
#define LIM_PLACE 10

#define OPT_HELP "-h" /* display usage */  
#define OPT_OUTFILE "-o" /* to specify output */
#define OPT_VERBOSE "-v" /* verbose mode */
#define OPT_ROTATIO "-r" /* to use rotations while texturing */
#define OPT_MIRROR "-m" /* to use mirror transform. */
#define OPT_XCOEFSIZ "-cx" /* coef to apply to define t_width */
#define OPT_YCOEFSIZ "-cy" /* coef to apply to define t_height */
#define OPT_COSTRED "-C"  /* cost reduction (default: COST_REDUC) */
#define OPT_IPLACE0 "-pr" /* initial random placed */
#define OPT_IPLACE1 "-pc" /* initially place patch in the topleft corner*/
#define OPT_RANDPL "-sr" /* switch to "faster"  placing algo */
#define OPT_RATIO "-ra" /* give ratio value */
#define OPT_REFIN "-re" /* set refinement stage */
#define OPT_BMP "-BMP" /* output in BMP format */
#define OPT_JPG "-JPG" /* output in BMP format */
#define OPT_PNG "-PNG" /* output in BMP format */

#define OPT_PLACE_RANDOM "-P1"
#define OPT_PLACE_ENTMAT "-P2"
#define OPT_PLACE_SUBMAT "-P3"

/* Number of tests for matching placements */
#define PLACE_ENTM_TESTS 100
#define PLACE_SUBM_TESTS 100

/* Cost functions */
#define OPT_COST1 "-C1"
#define OPT_COST2 "-C2"

#define XCOEF 3
#define YCOEF 3

/* Placements: Random, Entire matching Sub-matching */
#define P1 1
#define P2 2
#define P3 3

/* Cost function used */
#define C1 0  /* basic function */
#define C2 1  /* better function */

/* Output formats */
#define K_BMP 0x00
#define K_JPG 0x01
#define K_PNG 0x02

/* Masks for previous seams */
#define SEAM_LEFT 1
#define SEAM_TOP 2

/* Initial placing */
#define PI0 0
#define PI1 1

/* proportion of overlap needed for a new placement */
#define OVERLAP_RATIO 0.05

/* bonus on the seam->source edges */
#define SEAM_BONUS 1

/* number of refinement stages */
#define REF_ITERS 10

/* stop & go key */
#define KEY_STOP cimg_library::cimg::keySPACE

using namespace std;

class Args {

public:
	typedef uint_t pixel_t [2];

protected:

	string file_in; /* input file path   */
	string file_out; /* output file path */
	int out_format; /* output format     */

	bool k_ver; /* verbose mode          */
	bool k_rot; /* set rotation          */
	bool k_mir; /* set mirror            */
	char k_pin; /* initial position      */
	bool k_ran; /* switch to random placemt 
				when no more advance */
	bool k_ref; /* process to refinement */

	uint_t p_width; /* patch width       */
	uint_t p_height; /* patch height     */
	uint_t t_width; /* texture width     */
	uint_t t_height; /* texture height   */

	int xcoef;
	int ycoef;
	int cost_reduction;
	int nb_refinements;
	float ratio;

	char placement; /* placement         */
	char placement0; /* initial placment */
	char cost_fx; /* cost function       */

	/* total number of pixels on the texture */
	uint_t total_pixels;
	/* nb pixels set */
	uint_t nb_pixels;
	/* set to true when no more empty pixel */
	bool finished;

	/* current set of nodes */
	vector < Graph::node_id > nodes;

	/* INPUT AND OUTPUT IMAGES */

	/* original patch */
	cimg_library::CImg< uchar_t> * img_in;
	/* final texture ( larger ), @see XCOEF, YCOEF */
	cimg_library::CImg< uchar_t > * img_out;
	/* binary mask to mark pixels already colored */
	cimg_library::CImg< uint_t > * img_msk;
	/* error image, to draw seams */
	cimg_library::CImg< uchar_t > * img_err;
	/* image to remember top (vertical) seams  (continuous indexing) */
	vector < vector< uchar_t> > seav;
	/* image to remember left (horiz.à seams (continuous indexing) */
	vector < vector< uchar_t > > seah;

	/* DISPLAYS */

	cimg_library::CImgDisplay * disp_in;
	cimg_library::CImgDisplay * disp_out;
	cimg_library::CImgDisplay * disp_err;

	/* list of 256 first squares, to speed up placement cost computation */
	int squares [256]; 

public:


	/* CONSTRUCTOR  */

	Args() {

		/* init options */
		k_ver = false;
		k_rot = false;
		k_mir = false;
		k_ran = false;
		k_ref = false;
		k_pin = PI0;

		/* init parameters */
		placement = P3;
		placement0 = P2;
		cost_fx = C2;
		file_in = "";
		file_out = "";
		out_format = K_BMP;
		p_width = p_height = 0;

		nb_refinements = 0;

		total_pixels = 0;
		nb_pixels = 0;
		finished = false;

		xcoef = XCOEF;
		ycoef = YCOEF;
		cost_reduction = COST_REDUC;
		ratio = OVERLAP_RATIO;

		/* initialize square */
		for( int i=1; i < 256; i++ ) {
			squares[ i ] = i * i;
		}


		/* init random seed */
		initRandom();
	};


	/* DESTRUCTOR */

	~Args() {}


	/* GETTERS / SETTERS */

	int getTextureWidth() { return this->t_width; };
	void setTextureWidth( int w ) { this->t_width = w; };

	int getTextureHeight() { return this->t_height; };
	void setTextureHeight( int h ) { this->t_height = h; };

	string textureFileIn() { return this->file_in; };
	void setFileIn( string s ) { this->file_in = s; };

	string textureFileOut() { return this->file_out; };
	void setFileOut( string s ) { this->file_out = s; };

	uint_t getNbPixels() { return this->nb_pixels; };

	void setPlacement( char p ) { placement = p; };
	char getPlacement() { return placement; };

	unsigned getDispKey() { return disp_out->key; };
	void setDispKey( unsigned k ) { disp_out->key = k; };

	unsigned getDispButton() { return disp_out->button; };
	void setDispButton( unsigned k ) { disp_out->button = k; };

	void resetPlacement() { placement = placement0; };

	bool switchRandom() { return k_ran; };

	bool doRef() { return k_ref; };

	int nbRef() { return nb_refinements; };

	/* OTHER METHODS' PRIMITIVES */

	void graphCreate( Graph * G, vector< uint_t > pos );

	Graph::flowtype graphMaxFlow( Graph * G );

	int graphCutSeam( Graph * G, vector< uint_t > pos );

	vector< uint_t > placeInit();

	vector< uint_t > place();

	void getArgs( vector< string > vargs );

	void refreshImageOut();

	void dispImageIn( string title );

	void dispImageOut( string title );

	void dispImageErr( string title );

	void openImageIn();

	void openImageOut();

	void saveImageOut();

	void fatal( string mess );

	void nonfatal( string mess );

	void usage();

	bool verbose();

	void status();

	bool end();


protected:

	Graph::captype graphCost( uint_t * s, uint_t * t,  vector< uint_t > offset );

	Graph::captype graphCostBasic( uint_t * s, uint_t * t,  vector< uint_t > offset, int reduction );

	Graph::captype graphCostGradi( uint_t * s, uint_t * t,  vector< uint_t > offset );

	Graph::captype graphCost( uchar_t s1r, uchar_t s1v, uchar_t s1b, 
		uchar_t s2r, uchar_t s2v, uchar_t s2b,
		uchar_t t1r, uchar_t t1v, uchar_t t1b,
		uchar_t t2r, uchar_t t2v, uchar_t t2b
		);

	Graph::captype graphCostBasic( uchar_t s1r, uchar_t s1v, uchar_t s1b, 
		uchar_t s2r, uchar_t s2v, uchar_t s2b,
		uchar_t t1r, uchar_t t1v, uchar_t t1b,
		uchar_t t2r, uchar_t t2v, uchar_t t2b,
		int reduction );

	Graph::captype graphCostGradi( uchar_t s1r, uchar_t s1v, uchar_t s1b, 
		uchar_t s2r, uchar_t s2v, uchar_t s2b,
		uchar_t t1r, uchar_t t1v, uchar_t t1b,
		uchar_t t2r, uchar_t t2v, uchar_t t2b
		);

	bool borderPatch( vector< uint_t > pos, uint_t x, uint_t y );

	bool borderTexture( vector< uint_t > pos, uint_t x, uint_t y );

	void initRandom();

	vector< uint_t > placeRandom();

	vector< uint_t > placeEntireMatching();

	vector< uint_t > placeSubMatching();

};

#endif


