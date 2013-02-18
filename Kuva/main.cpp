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
* main.cpp
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


#include "main.h"
#include "args.h"
#include "graph.h"

using namespace std;

bool stopped=false;


void sighandle( int signum ) {
	/*
	Catch Ctrl-C signal.
	*/
	if ( stopped )
		exit( 0 );
	stopped = true;
}


int init( Args * args, vector<string> vargs ) {
	/*
	Initialize parameters (cmd line, etc.).
	*/  
	/* read cmd line arguments*/
	args->getArgs( vargs );

	/* open images */
	args->openImageIn();
	args->openImageOut();

	if ( args->verbose() ) {
		cout << ":: Verbose mode" << endl;
	}

	return 0;
}

void sleep(unsigned int mseconds)
{
	clock_t goal = mseconds + clock();
	while (goal > clock());
}

int mkTexture( Args * args ) {
	/*
	Build texture and display it.
	*/
	vector< uint_t > vec;

	/* number of pixels set */
	uint_t same_nb=0;
	/* number of continuous iterations with same_nb */
	uint_t same_it=0;
	/* nb of refinement iterations done */
	int refs=0;

	/* init first patch position */
	vec = args->placeInit();

	Graph * G;

	cout << "Press Ctrl-C to interrupt and show texture" << endl;

	/* Display output image */
	args->dispImageOut("Texture");


	while ( !stopped  ) {

		G = new Graph;

		/* If stopping was pressed, wait until restart */

		if ( args->getDispButton() != 0 ) {      

			cout << "STOP!" << endl;

			// wait for button release
			while ( args->getDispButton() != 0 ) {
				sleep( 1 ); 
			}
			cout << "GO!" << endl;      
		}


		/* Display status */
		args->status();
		/* Place patch */
		vec = args->place();

		/* Initialize nodes, edges, etc. */
		args->graphCreate( G, vec );

		/* Computes maxflow */
		args->graphMaxFlow( G );
		/* Copy the cut pixels */
		args->graphCutSeam( G, vec );

		/* We'll use Random placing if at some time no more pixel is filled */
		/* If still no pixel added, increment counter*/
		if ( ( args->switchRandom() ) && ( ! args->end() ) )  {
			if ( args->getNbPixels() == same_nb ) {     
				same_it++;    
			}
			else { /* Pixels added, reset counter, update nb_pixel value */      
				same_it = 0;
				same_nb = args->getNbPixels();
			}    
			/* If limit reached, use random placement */
			if ( same_it >= LIM_PLACE ) {
				if ( args->getPlacement() == P3 ) {
					args->setPlacement( P2 );
					cout << "Switch to entire patch matching." << endl;
				}
				else if ( args->getPlacement() == P2 ) {
					args->setPlacement( P1 );
					cout << "Switched to random placement." << endl;
				}
				same_it = 0;
				same_nb = args->getNbPixels();
			}
		}

		/* if texture fully filled... */
		if ( args->end() ) {
			/* stop if no refinement required */
			if ( ! args->doRef() )
				stopped = true;
			/* else handle iteration, stop if max reached */
			else {
				if ( ! refs ) {
					args->resetPlacement();
					cout << "Start refinement stage." << endl;
				}
				if ( refs >= args->nbRef() )
					stopped = true;
				else {
					refs++;
				}
			}
		}
		delete G;
		args->refreshImageOut();
	}
	args->status();
	cout << endl;

	/* Display erro image */
	args->dispImageErr("Seams");

	/* Save it if specifyed (test in the fx)  */
	args->saveImageOut();

	return 0;
}


int main( int ac, char ** av ) {

	string s;

	/* declare signal handler */
	signal( SIGINT, sighandle );


	/* get command line words as a vector */
	vector<string> vargs;
	for( int i=1; i < ac; i++ ) {
		vargs.push_back( av[i] );
	} 

	/* execution descriptor */
	Args * args;
	args = new Args;

	/* initialize, check for syntax errors, etc.*/
	init ( args, vargs );

	/* build the texture */
	mkTexture( args );

	cout << "- Press a key and Enter to quit -" << endl;

	cin >>  s;

	return 1;
}
