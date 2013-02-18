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
* args.cpp
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


#include "args.h"
#include "graph.h"



/************/
/* POSITION */
/************/


vector< uint_t > Args::placeInit() {
	/*
	Initialisation of the texture, to place the patch
	at a random position, such that the full image 
	is displayed.
	*/
	vector< uint_t > pos;
	uint_t x, y;
	if ( k_pin == PI0 ) {
		x = (int)( ( t_width - p_width ) * ( (float)rand() / RAND_MAX ) );
		y = (int)( ( t_height - p_height ) * ( (float)rand() / RAND_MAX ) );
	}
	else { /* top-left corner */
		x = y = 0;
	}
	pos.push_back( x );
	pos.push_back( y );

	for ( uint_t i=x; i < x+p_width; i++ )
		for ( uint_t j=y; j < y+p_height; j++ ) {

			(*img_out) (i % t_width, j % t_height, 0) = (*img_in) (i-pos[0], j-pos[1], 0);
			(*img_out) (i % t_width, j % t_height, 1) = (*img_in) (i-pos[0], j-pos[1], 1);
			(*img_out) (i % t_width, j % t_height, 2) = (*img_in) (i-pos[0], j-pos[1], 2);

			(*img_msk) ( i, j ) = 255; 
			nb_pixels++; 
		}

		return pos;
}


vector< uint_t > Args::placeRandom() {
	/*
	Return a position of the top-left corner for
	a new patch, using random placement.
	( checks that there is some overlapping )
	1. find random coordinates
	2. check in im_msk if overlaps enough
	3. return (x,y) top left coordinate of the patch
	[0]: X coordinate
	[1]: Y coordinate
	[2]: #pixels overlapping
	*/
	vector< uint_t > pos;
	uint_t i, j, k=0, ok = 0;
	uint_t bound = (int) ( p_width * p_height * ratio );
	uint_t x, y;

	while ( !ok ) {

		k = 0;
		/* choose random coordinates */
		x = (int)( t_width * ( (float)rand() / RAND_MAX ) );
		y = (int)( t_height * ( (float)rand() / RAND_MAX ) );

		/* test if enough overlaps */
		for ( i=x; i < x+p_width; i++ )
			for ( j=y; j < y+p_height; j++ ) {
				if ( (*img_msk) ( i % t_width, j % t_height ) != 0 )
					k++;
			}
			if ( k >= bound ) {
				ok = 1;
			}
	}  
	/* set coordinates vector */
	pos.push_back( x );
	pos.push_back( y );
	pos.push_back( k );
	/* add width limits */
	pos.push_back( 0 );
	pos.push_back( p_width );
	/* add height limits */
	pos.push_back( 0 );
	pos.push_back( p_height );

	return pos;
}


vector< uint_t > Args::placeEntireMatching() {
	/*
	Return a position of the top-left corner for
	a new patch, using Entire Matching placement.
	[0]: X coordinate
	[1]: Y coordinate
	[2]: #pixels overlapping
	*/
	vector< uint_t > pos;
	uint_t i, j, k=0, ok=0, tests=0;
	uint_t fullarea = p_width * p_height;
	uint_t bound = (uint_t) ( fullarea * ratio );
	float cost = 100000000, ncost;
	uint_t sumr=0, sumv=0, sumb=0;
	uint_t bestx=0, besty=0;
	uint_t x, y;
	bool holes = false;

	while ( tests < PLACE_ENTM_TESTS ) {

		tests++;
		ok = 0;

		while ( !ok ) {

			/* initialize */
			k = 0;
			sumr = sumv = sumb = 0;

			/* choose random coordinates */
			x = (int)( t_width * ( (float)rand() / RAND_MAX ) );
			y = (int)( t_height * ( (float)rand() / RAND_MAX ) );    
			/* test if enough overlaps */
			for ( i=x; i < x+p_width; i++ )
				for ( j=y; j < y+p_height; j++ ) {
					/* if this pixel is already filled, add one more pixel overlapping */
					if ( (*img_msk) ( i % t_width, j % t_height ) != 0 )
						k++;
				}
				/* k = #pixels overlapped */
				if ( k >= bound ) {
					ok = 1;
				}
				if ( k == fullarea ) {
					holes = false;
				}
				else {
					holes = true;
				}
		}

		if ( ( end() ) || ( holes ) ) {

			/* one good placement found, computes COST  */
			for ( i=x; i < x+p_width; i++ )
				for ( j=y; j < y+p_height; j++ ) {
					/* here were lays a pixel of the current texture,
					add its cost to the sum
					*/
					if ( (*img_msk) ( i % t_width, j % t_height) != 0 ) {

						int ii = i % t_width, jj = j % t_height;
						sumr += squares[ abs( (*img_in)(i-x, j-y, 0) - (*img_out)(ii, jj, 0) ) ];
						sumv += squares[ abs( (*img_in)(i-x, j-y, 1) - (*img_out)(ii, jj, 1) ) ];
						sumb += squares[ abs( (*img_in)(i-x, j-y, 2) - (*img_out)(ii, jj, 2) ) ];
					}	    
				}

				/* compute final cost */
				sumr = (int)( sumr / k ); 
				sumv = (int)( sumv / k ); 
				sumb = (int)( sumb / k );

				/* reduce complexity when empty areas remaining */
				ncost = ( sumr + sumv + sumb ) / 3;
				if ( holes ) {
					ncost = (uint_t) ( ncost * 0.75 );
				}
				//    cout << "cost = " << ncost << endl;

				if ( cost > ncost ) {
					cost = ncost;
					bestx = x;
					besty = y;
				}
		}
		else {
			//tests--;
		}
	}

	/* add coordinates on output texture */
	pos.push_back( bestx );
	pos.push_back( besty );
	/* nb of pixels overlapping */
	pos.push_back( k );
	/* add width limits */
	pos.push_back( 0 );
	pos.push_back( p_width );
	/* add height limits */
	pos.push_back( 0 );
	pos.push_back( p_height );

	return pos;
}


vector< uint_t > Args::placeSubMatching() {
	/*
	Return a position of the top-left corner for
	a new patch, using Sub Matching placement.
	*/

	/* 
	1. Pick a random non-empty area in output image
	2. Look for the better patch position inside it
	*/

	uint_t top_leftx, top_lefty, bot_rightx, bot_righty;
	bool empty = true;
	uint_t i, j, k=0, bound=0;


	while ( empty ) {

		/* pick random coordinates in output image */
		top_leftx = (int)( t_width * ( (float)rand() / RAND_MAX ) );
		top_lefty = (int)( t_height * ( (float)rand() / RAND_MAX ) );
		bot_rightx = top_leftx + (int)( (t_width-top_leftx) * ( (float)rand() / RAND_MAX ) );
		bot_righty = top_lefty + (int)( (t_width-top_leftx) * ( (float)rand() / RAND_MAX ) );

		bound = (uint_t) ( (bot_rightx - top_leftx) *  (bot_righty - top_lefty) * 0.1 );

		for ( i=top_leftx; i < bot_rightx; i++ )
			for ( j=top_lefty; j < bot_righty; j++ ) {
				/* if non empty area, accept this area*/
				if ( (*img_msk)(i % t_width, j % t_height) != 0 ) {
					k++;
				}
			}
			if ( k > bound )
				empty = false;
	}

	bound =  (bot_rightx - top_leftx) *  (bot_righty - top_lefty);

	/* Look for best patch position */

	vector< uint_t > pos;
	uint_t  ok=0, tests=0;
	float cost = 100000000, ncost;
	uint_t sumr=0, sumv=0, sumb=0;
	uint_t bestx=0, besty=0;
	uint_t x, y;

	/* do not make more tests than possible translations */
	while ( ( tests < PLACE_SUBM_TESTS ) && ( tests < bound ) ) {

		//    cout << "*\n";
		tests++;
		ok = 0;
		empty = true;

		/* Look for a patch position */
		while ( !ok ) {

			/* initialize */
			k = 0;
			sumr = sumv = sumb = 0;

			/* choose random offset in the selected area of the output image */
			x = (int)( (bot_rightx-top_leftx) * ( (float)rand() / RAND_MAX ) );
			y = (int)( (bot_righty-top_lefty) * ( (float)rand() / RAND_MAX ) );
			//      cout << ": " << x << ", " << y << endl;

			/* test if enough overlaps, between SELECTED AREA (!)  and patch  */
			for ( i=x+top_leftx; i < bot_rightx; i++ )
				for ( j=y+top_lefty; j < bot_righty; j++ ) {
					/* if this pixel is already filled, add one more pixel overlapping */
					if ( (*img_msk) ( i % t_width, j % t_height ) != 0 ) {
						k++;
					}
				}
				if ( k )
					ok = 1;
		}

		/* one good placement found, computes COST  */
		for ( i=x+top_leftx; i < bot_rightx; i++ )
			for ( j=y+top_lefty; j < bot_righty; j++ ) {

				/* here were lays a pixel of the current texture,
				add its cost to the sum
				*/
				if ( (*img_msk) ( i % t_width, j % t_height) != 0 ) {

					int ii = i % t_width, jj = j % t_height;
					int xx = ( i - top_leftx ) % t_width, yy = ( j - top_lefty ) % t_height;

					sumr += squares[ abs( (*img_in)(xx, yy, 0) - (*img_out)(ii, jj, 0) ) ];
					sumv += squares[ abs( (*img_in)(xx, yy, 1) - (*img_out)(ii, jj, 1) ) ];
					sumb += squares[ abs( (*img_in)(xx, yy, 2) - (*img_out)(ii, jj, 2) ) ];
				}	    
			}

			/* compute final cost */
			sumr = (int)( sumr / k ); 
			sumv = (int)( sumv / k ); 
			sumb = (int)( sumb / k );

			ncost = ( sumr + sumv + sumb ) / 3;

			if ( cost > ncost ) {
				cost = ncost;
				bestx = ( x + top_leftx ) % t_width;
				besty = ( y + top_lefty ) % t_height;
			}

	}

	pos.push_back( bestx );
	pos.push_back( besty );
	/* nb of pixels overlapping */
	pos.push_back( 0 );
	/* add width limits */
	pos.push_back( 0 );
	pos.push_back( p_width ); //(bot_rightx-bestx) % t_width );
	/* add height limits */
	pos.push_back( 0 );
	pos.push_back( p_height ); //(bot_righty-besty) % t_height );

	return pos;
}


vector < uint_t > Args::place() {
	/*
	Return a position for a new patch.
	*/

	/* If required, randomly rotate image */
	if ( k_rot ) {
		img_in->rotate( 90 * (int) ( 4* ( (float)rand() / RAND_MAX) )  );    
	}
	/* If required, randomly mirror image */
	if ( k_mir ) {
		int g = (int) ( 4* ( (float)rand() / RAND_MAX) );    
		if ( g == 0 )
			img_in->mirror( 'x' );
		else if ( g ==1 )
			img_in->mirror( 'y' );
	}

	if ( placement == P1 )
		return placeRandom();
	else if ( placement == P2 )
		return placeEntireMatching();
	return placeSubMatching();
}


/********************/
/* DO YOU READ ME ? */
/********************/


void Args::getArgs( vector<string> vargs ) {
	/*
	Read command line and update args.
	*/
	vector<string>::const_iterator p; 
	bool placed = false; /* set when placement indicator met */
	bool filein = false; /* set when file met */

	for ( p=vargs.begin(); p != vargs.end(); p++ ) {

		if ( *p == OPT_HELP ) {

			usage();
		}
		else if ( *p == OPT_OUTFILE ) {

			p++;
			if ( p == vargs.end() ) 
				fatal( "missing argument" );

			setFileOut( *p );	
		}
		else if ( *p == OPT_BMP ) { out_format = K_BMP; }
		else if ( *p == OPT_JPG ) { out_format = K_JPG; }
		else if ( *p == OPT_PNG ) { out_format = K_PNG; }
		else if ( *p == OPT_COST1 ) { cost_fx = C1; }
		else if ( *p == OPT_COST2 ) { cost_fx = C2; }
		else if ( *p == OPT_IPLACE0 ) { k_pin = PI0; }
		else if ( *p == OPT_IPLACE1 ) { k_pin = PI1; }
		else if ( *p == OPT_RANDPL ) { k_ran = true; }
		else if ( *p == OPT_REFIN ) { 

			p++;
			if ( p == vargs.end() ) 
				fatal( "missing argument" );
			k_ref = true; 
			nb_refinements = (int) atoi( (*p).c_str() );
			nb_refinements *= REF_ITERS;

		}
		else if ( *p == OPT_COSTRED ) {

			p++;
			if ( p == vargs.end() ) 
				fatal( "missing argument" );

			cost_reduction = (int) atoi( (*p).c_str() );
			if ( ( cost_reduction < 10 ) || ( cost_reduction > 30 ) )
				nonfatal( "warning: cost reduction may be too large or too small." );
		}
		else if ( *p == OPT_RATIO ) {

			p++;
			if ( p == vargs.end() ) 
				fatal( "missing argument" );

			ratio = (float) atof( (*p).c_str() );
			if ( ( ratio < 0 ) || ( ratio > 1 ) )
				nonfatal( "warning: cost reduction may be too large or too small." );
		}
		else if ( *p == OPT_PLACE_RANDOM ) {

			if ( placed )
				fatal("syntax error, you should not specify several placement functions.");
			placed = true;
			placement = P1;
		}
		else if ( *p == OPT_PLACE_ENTMAT ) {

			if ( placed )
				fatal("syntax error, you should not specify several placement functions.");
			placed = true;
			placement = P2;
		}
		else if ( *p == OPT_PLACE_SUBMAT ) {

			if ( placed )
				fatal("syntax error, you should not specify several placement functions.");
			placed = true;
			placement = P3;
		}
		else if ( *p == OPT_MIRROR ) { k_mir = true; }
		else if ( *p == OPT_ROTATIO ) { k_rot = true; }
		else if ( *p == OPT_VERBOSE ) { k_ver = true; }
		else if ( *p == OPT_XCOEFSIZ ) {

			p++;
			if ( p == vargs.end() ) 
				fatal( "missing argument" );
			xcoef = (int) atoi( (*p).c_str() );
		}
		else if ( *p == OPT_YCOEFSIZ ) {

			p++;
			if ( p == vargs.end() ) 
				fatal( "missing argument" );
			ycoef = (int) atoi( (*p).c_str() );
		}
		else {
			if ( filein )
				fatal("unknown argument.");
			setFileIn( *p );
			filein = true;
		}
	}

	if ( !filein ) {

		nonfatal("please specify an input file.");
		usage();
	}

	placement0 = placement;

}


/************/
/* FIAT LUX */
/************/

void Args::refreshImageOut() {


	disp_out->render( *img_out );
	disp_out->resize( *disp_out );
}

void Args::dispImageIn( string title ) {
	/* 
	Display input image (must be opened)  
	*/
	if ( p_width ) {
		disp_in = new cimg_library::CImgDisplay( *img_in, title.c_str() );
	}
}


void Args::dispImageOut( string title ) {
	/* 
	Display output image (must be opened)
	*/
	if ( t_width ) {
		disp_out = new cimg_library::CImgDisplay( *img_out, title.c_str() );
	}
}


void Args::dispImageErr( string title ) {
	/* 
	Display error image (must be opened)
	*/

	int k, l;
	/* set the background and improve the seams look */
	for ( int i=0; i < img_err->dimx(); i++ ) 
		for ( int j=0; j < img_err->dimy(); j++ ) {
			if ( (*img_err)( i, j ) != 0 ) {
				(*img_err)( i, j, 0 ) = (*img_out)( i, j, 0);
				(*img_err)( i, j, 1 ) = (*img_out)( i, j, 1);
				(*img_err)( i, j, 2 ) = (*img_out)( i, j, 2);
			}

			else { // enlarge seam
				for ( k=0; k < 2; k++ )
					for ( l=0; l < 2; l++ ) {
						if ( (i-1+k>=0) && (i-1+k<img_err->dimx()) )
							if ( (j-1+l>=0) && (j-1+l<img_err->dimy()) ) {		
								(*img_err)( i-1+k, j-1+l, 0 ) = 0;
								(*img_err)( i-1+k, j-1+l, 1 ) = 0;
								(*img_err)( i-1+k, j-1+l, 2 ) = 255;
							}
					}	    
			}
		}

		if ( t_width ) {
			disp_err = new cimg_library::CImgDisplay( *img_err, title.c_str() );
		}
}


/************/
/* CREATION */
/************/


void Args::openImageIn() { 
	/* 
	Open the given image. 
	*/
	if ( file_in != "" )
		img_in = new cimg_library::CImg< uchar_t >( file_in.c_str() ); 
	p_width = img_in->dimx();
	p_height = img_in->dimy();
}


void Args::openImageOut() { 
	/* 
	Create the ouput image (2D, 3 channels), and the mask.
	*/
	if ( p_width ) {
		img_out = new cimg_library::CImg< uchar_t > ( xcoef * p_width, 
			ycoef * p_height, 1, 3 ); 
		img_msk = new cimg_library::CImg< uint_t > ( xcoef * p_width,
			ycoef * p_height );
		img_err = new cimg_library::CImg< uchar_t > ( xcoef * p_width,
			ycoef * p_height, 1, 3 );

		/* Fill work image */
		img_msk->fill( 0 );
		img_err->fill( 255 );

		/* Set dimensions */
		t_width = img_out->dimx();
		t_height = img_out->dimy();
		total_pixels = t_width * t_height;

		/* Initialize old seam databases */
		for( size_t i=0; i < t_width * t_height; i++ ) {

			vector< uchar_t > v( 13 );
			v[0] = 0; /* 0 means "no seam node", 1 o/w */
			seav.push_back( v );
			seah.push_back( v );
		}
	}
} 


void Args::saveImageOut() {
	/*
	Save texture.
	*/
	if ( file_out != "" ) {  

		switch( out_format ) {

	case K_BMP: img_out->save_bmp( (file_out + ".bmp").c_str() ); break;
	case K_JPG: img_out->save_jpeg( (file_out + ".jpg").c_str() ); break;
	case K_PNG: img_out->save_png( (file_out + ".png").c_str() ); break;
	default: break;
		}
	}

}

/*************/
/* INCIDENTS */
/*************/


void Args::fatal( string mess ) {
	/* 
	Display error message on cerr and exit. 
	*/
	cerr << "kuva: "<< mess << endl;
	exit( -1 );
}

void Args::nonfatal( string mess ) {
	/* 
	Display error message on cerr and exit. 
	*/
	cerr << "kuva: "<< mess << endl;
}


void Args::usage() {
	/* 
	Display usage informations and exit.
	*/
	cout << "Usage: kuva input [parameters]\n\n";

	cout << "Cost function (to label edges):" << endl;
	cout << "\t-C1\tBasic function." << endl;
	cout << "\t-C2\tFunction using gradient (default)." << endl;

	cout << "Placement algorithm (to place patch):" << endl;
	cout << "\t-P1\tRandom placement." << endl;
	cout << "\t-P2\tEntire patch matching ." << endl;
	cout << "\t-P3\tSub-patch matching (default)." << endl;

	cout << "Output type:" << endl;
	cout << "\tSpecify either -BMP (default), -JPG, or -PNG." << endl; 

	cout << "Transformations:" << endl;
	cout << "\t-m\tRandomly mirror the patch while texturing." << endl;
	cout << "\t-r\tRandomly rotate the patch while texturing." << endl;

	cout << "Other options:" << endl;
	cout << "\t-C n\tSpecify reduction applyed to edges costs (default: 20)." << endl;
	cout << "\t-cx n\tSpecify texture width coefficient." << endl;
	cout << "\t-cy n\tSpecify texture height coefficient." << endl;
	cout << "\t-h\tDisplay this help informations." << endl;
	cout << "\t-o f\tOutput texture to the given file (do not give extension)." << endl;
	cout << "\t-pc\tInitial position at top-left corner." << endl;
	cout << "\t-pr\tInitial random position (default)." << endl;
	cout << "\t-ra n\tSpecify minimal overlap ratio (in ]0;1[ )" << endl;
	cout << "\t-re\tProcess to a refinement stage after the whole image is filled." << endl;
	cout << "\t-sr\tSwitch to a faster placement algorithm when no advance." << endl;
	cout << "\t-v\tVerbose mode" << endl;
	cout << endl << "While computing texture, press Ctrl-C to interrupt process, "
		<< "and display current texture." << endl;
	exit( 0 );
}


bool Args::verbose() {
	/*
	True if verbose mode.
	*/
	return k_ver;
}


void Args::initRandom() {
	/*
	Initialize pseudo-random number generator,
	using process' PID.
	*/
	srand( (int)_getpid() );
}


void Args::status() {
	/*
	Display progress bar evolution.
	*/
	cout << "\r" << nb_pixels << "/" << total_pixels;;
	cout.flush();
}


bool Args::end() {

	return finished;
}

