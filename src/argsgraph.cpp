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



  /*************/
 /* GO GRAPH  */
/*************/


void Args::graphCreate( Graph * G, vector< uint_t > pos ) {
  /*
    From a position of a new patch, creates the graph of the overlap:
    a node is a pixel, linked to its neighbors
    border pixels are linked to the Source or Sink.
    pos[0]: X coordinate
    pos[1]: Y coordinate
    pos[2]: #pixels overlapping
  */
  uint_t i, j, k=0;
  vector < uint_t > puf;
  /* initialize nodes set */
  nodes.clear();

  uint_t sources=0, sinks=0, nos=0;;

  /* for all pixels under the (sub)patch */
  for( i=pos[0]+pos[3]; i < pos[0] + pos[4]; i++)
    for( j=pos[1]+pos[5]; j < pos[1] + pos[6]; j++) {

      /* if there's a pixel there: OVERLAP */
      if ( (*img_msk)( i % t_width, j % t_height ) ) {

	/* add the node to the graph  */
	nodes.push_back( G->add_node() );
	
	/* to keep trace of the node index, copy k+1 in img_msk,
	   as it was already > 0, snot a problem */
	(*img_msk)( i % t_width, j % t_height ) = k+1;

	/* EDGES WITH THE SOURCE, SINK, OR NOTHING */

	/* if node linked to SINK (close to texture border)  
	   Criterion: boundary pixel, ie extrema coordinates 
	*/
	if ( borderTexture( pos, i, j ) ) {

	  G->set_tweights( nodes[k], 0, MAX_SHORT );
	  sinks++;
	}
	/* else, if linked to SOURCE (close to an empty area) */
	else if ( borderPatch( pos, i, j ) ) {

	  G->set_tweights( nodes[k], MAX_SHORT, 0 );
	  sources++;
	}
	/* else, no connexion */
	else {

	  /* unuseful..*/
	  //G->set_tweights( nodes[k], 0, 0 );
	  nos++;
	}
	/* EDGES WITH NEIGHBORS */
	/* 
	   for each pixel, convention is: only add edges for top and left pixels 
	   if top pixel snot empty and in the patch, add edge to it
	   idem for left pixel
	*/
	uint_t nodex;
	Graph::captype ncost;
	uint_t home [2];
	uint_t eend [2];
	uint_t x = i % t_width;
	uint_t y = j % t_height;

	/* LEFT NEIGHBOUR */
	/* LEFT NEIGHBOUR */
	/* LEFT NEIGHBOUR */
	if ( i > pos[0]+pos[3] ) {
	  
	  /* if not empty */
	  if ( (*img_msk)( (i-1)%t_width, y ) > 0 ) {

	    /* SEAM NODE at left ? */
	    /* SEAM NODE at left ? */

	    /* If there is a SN at left...ONE MORE NODE (=> k++)  */
	    if ( seah[ x*t_height + y ][0] != 0 ) {

	      uint_t w = x*t_height + y; /* seah index... */

	      k++;
	      /* Add its edges (to both neighbors and SOURCE) */
	      nodes.push_back( G->add_node() );

	      /* see paper for formula... (!!!) */
	      ncost = graphCost( seah[w][1], seah[w][2], seah[w][3], seah[w][4],
				 seah[w][5], seah[w][6], seah[w][7], seah[w][8],
				 seah[w][9], seah[w][10], seah[w][11], seah[w][12]
				 ) + SEAM_BONUS;
	      /* NEED TO COMPUTE THE COST M(s,t,As,At)! */
	      G->set_tweights( nodes[k], ncost, 0 );

	      /* from LEFT pixel to SEAM node... */
	      /* take node index of the neighbour */
	      nodex = (*img_msk)( (i-1) % t_width, y ) - 1;
	      /* ask for cost */
	      ncost = graphCost( seah[w][1], seah[w][2], seah[w][3],
				 (*img_in)(i-1,j,0), (*img_in)(i-1,j,1), (*img_in)(i-1,j,2),
				 seah[w][10], seah[w][11], seah[w][12],
				 (*img_in)(i,j,0), (*img_in)(i,j,1), (*img_in)(i,j,2)
				 );
	      /* here k is the index of the SEAM node => use k-1 for current node */
	      G->add_edge( nodes[k], nodes[nodex], ncost, ncost ); 

	      /* Link SEAM node to CURRENT pixel */
	      ncost = graphCost( seah[w][4], seah[w][5], seah[w][6],
				 (*img_in)(i-1,j,0), (*img_in)(i-1,j,1), (*img_in)(i-1,j,2),
				 seah[w][7], seah[w][8], seah[w][9],
				 (*img_in)(i,j,0), (*img_in)(i,j,1), (*img_in)(i,j,2)
				 );
	      G->add_edge( nodes[k], nodes[k-1], ncost, ncost ); 
	    }
	    else { /* SIMPLER ! only add edge to the left neighbour */
	    
	      /* take node index of the neighbour */
	      nodex = (*img_msk)( (i-1) % t_width, y ) - 1;
	      /* initialize pixel_t values */
	      home[0] = i; home[1] = j;
	      eend[0] = i-1; eend[1] = j;
	      /* ask for cost */
	      ncost = graphCost( home, eend, pos );
	      /* add the edge with the cost found */
	      G->add_edge( nodes[k], nodes[nodex], ncost, ncost ); 
	    }
	  }
	}


	/* TOP NEIGHBOUR */
	/* TOP NEIGHBOUR */
	/* TOP NEIGHBOUR */
	if ( j > pos[1]+pos[5] ) {
	 

	  /* if not empty */
	  if ( (*img_msk)( x, (j-1) % t_height ) > 0 ) {

	    /* SEAM NODE at top ? */
	    /* SEAM NODE at top ? */

	    /* If there is a SN at top...ONE MORE NODE (=> k++)  */
	    if ( seav[ x*t_height + y ][0] != 0 ) {

	      uint_t w = x*t_height + y; /* seah index... */

	      k++;
	      /* Add its edges (to both neighbors and SOURCE) */
	      nodes.push_back( G->add_node() );

	      /* see paper for formula... (!!!) */
	      ncost = graphCost( seav[w][1], seav[w][2], seav[w][3], seav[w][4],
				 seav[w][5], seav[w][6], seav[w][7], seav[w][8],
				 seav[w][9], seav[w][10], seav[w][11], seav[w][12]
				 ) + SEAM_BONUS;
	      /* NEED TO COMPUTE THE COST M(s,t,As,At)! */
	      G->set_tweights( nodes[k], ncost, 0 );

	      /* from LEFT pixel to SEAM node... */
	      /* take node index of the neighbour */
	      nodex = (*img_msk)( x, (j-1) % t_height ) - 1;
	      /* ask for cost */
	      ncost = graphCost( seav[w][1], seav[w][2], seav[w][3],
				 (*img_in)(i,j-1,0), (*img_in)(i,j-1,1), (*img_in)(i,j-1,2),
				 seav[w][10], seav[w][11], seav[w][12],
				 (*img_in)(i,j,0), (*img_in)(i,j,1), (*img_in)(i,j,2)
				 );
	      /* here k is the index of the SEAM node => use k-1 for current node */
	      G->add_edge( nodes[k], nodes[nodex], ncost, ncost ); 
	      /* Link SEAM node to CURRENT pixel */
	      ncost = graphCost( seav[w][4], seav[w][5], seav[w][6],
				 (*img_in)(i,j-1,0), (*img_in)(i,j-1,1), (*img_in)(i,j-1,2),
				 seav[w][7], seav[w][8], seav[w][9],
				 (*img_in)(i,j,0), (*img_in)(i,j,1), (*img_in)(i,j,2)
				 );
	      
	      /* add edge from SEAM node (k) to CURRENT node (k-1) */ 
	      G->add_edge( nodes[k], nodes[k-1], ncost, ncost ); 
	    }
	    else {
	      /* take node index of the neighbour */
	      nodex = (*img_msk)( x, (j-1) % t_height ) - 1;
	      /* initialize pixel_t values */
	      home[0] = i; home[1] = j;
	      eend[0] = i; eend[1] = j-1;
	      /* ask for cost */
	      ncost = graphCost( home, eend, pos );
	      /* add the edge with the cost found */
	      G->add_edge( nodes[k], nodes[nodex], ncost, ncost ); 
	    }
	  }

	}
	/* add the patch pixel */
	puf.push_back( i - pos[0] );
	puf.push_back( 1 - pos[1] );

	/* clear pixel buffer */
	puf.clear();
	/* increment number of pixels */
	k++;
      }
    }
  
  /* if no empty space below, add a SOURCE link to force pixels to be in the cut  */
  /*
  if ( sources == 0 )
  G->set_tweights( nodes[k-1], MAX_SHORT, 0 );
  */  
  /*
    PROBLEM !
    add source link with a middle node ??
    
  */

}


bool Args::borderPatch( vector< uint_t > pos, uint_t x, uint_t y ) {
  /*
    Return true if the given pixel needs to be linked to the SOURCE,
    that is, it's on an overlap are, just close to a non-overlap area,
    still in the patch.
    pos: offset of the patch
    (x, y): pixel coords
    R: given pixel is on an overlap area !
   */

  int i, j;
  for ( i=-1; i < 2; i++ )
    for ( j=-1; j < 2; j++ ) {      
      /* if close to an empty area (inside the patch) */
      if ( (*img_msk)( (x+i) % t_width, (y+j) % t_height, 0 ) == 0 ) {
	return true;
      }
    }
  return false;
}


bool Args::borderTexture( vector< uint_t > pos, uint_t x, uint_t y ) {
  /*
    Return true if the given pixel needs to be linked to the SINK,
    that is, it's on the boundary of the patch, on an overlap area. 
    pos: offset of the patch
    (x, y): pixel coords
    R: given pixel is on an overlap area !
   */
  /* if on a border of the patch */

  if ( x == pos[0]+pos[3] )
    return true;
  uint_t xx = pos[0] + pos[4] - 1 ;
  if ( xx  > t_width ) {
    if ( x == xx - 1 )
      return true;
    }
  else if ( x == xx )
    return true;

  if ( y == pos[1]+pos[5] )
    return true;
  uint_t yy = pos[1] + pos[6] - 1 ;
  if ( yy  > t_height ) {
    if ( y == yy - 1 )
      return true;
    }
  else if ( y == yy )
    return true;
  
  return false;
}


Graph::flowtype Args::graphMaxFlow( Graph * G ) {
  /*
    Computes max flow for the given graph, returning maxflow value.
   */
  Graph::flowtype flow = G->maxflow();

  return flow;
}


int Args::graphCutSeam( Graph * G, vector< uint_t > pos ) {
  /*
    From the graph cut, select pixels to be copied (and copy to the mask!).
  */
  
  uint_t i, j;
  uint_t nodex, sources=0, sinks=0;

  bool lastSource = true; /* last pixel was source */
  bool frst_ov = true; /* first overlap pixel */

  /*
    Iterate over pixels under the patch:
    - if place empty, copy pixel
    - if not, value is the node number, => if in source, copy
  */
  
  for ( i=0; i < pos[4]-pos[3]; i++ ) {
    
    frst_ov = true;

    for ( j=0; j < pos[6]-pos[5]; j++ ) {

      /* real coordinates in the texture image, != path coords */
      uint_t x = (i + pos[0] + pos[3]) % t_width;
      uint_t y = (j + pos[1] + pos[5]) % t_height;
      
      nodex = (*img_msk)( x, y );
      /* if pixel empty, copy patch there */
      if ( nodex == 0 ) { 

	(*img_out)( x, y, 0 ) = (*img_in)( i, j, 0 );
	(*img_out)( x, y, 1 ) = (*img_in)( i, j, 1 );
	(*img_out)( x, y, 2 ) = (*img_in)( i, j, 2 );
	(*img_msk)( x, y ) = 255;
	nb_pixels++;

	if ( nb_pixels >= total_pixels )
	  finished = true;
      }
      else {
	
	if ( frst_ov ) {
	  
	  if ( G->what_segment(nodes[ nodex-1 ] ) == Graph::SOURCE )
	    lastSource = true;
	  else
	    lastSource = false;
	  frst_ov = false;
	}

	/* overlap area, cp only if in SOURCE */
	if ( G->what_segment(nodes[ nodex-1 ] ) == Graph::SOURCE ) {
	  
	  sources++;

	  (*img_out)( x, y, 0 ) = (*img_in)( i, j, 0 );
	  (*img_out)( x, y, 1 ) = (*img_in)( i, j, 1 );
	  (*img_out)( x, y, 2 ) = (*img_in)( i, j, 2 );

	  /* if last pixel was from SINK, draw a black pixel, o/w draw white */
	  if ( !lastSource ) {

	    (*img_err)( x, y ) = 0;
	    lastSource = true;

	    /* ***** ADDING SEAM NODES ****** */

	    /* ADD SEAM NODE ON THE TOP */
	    seav[ x*t_height + y ][0] = 1;
	    /* RVB of top pixel (BG)*/
	    seav[ x*t_height + y ][1] = (*img_out)( x, y - 1, 0 );
	    seav[ x*t_height + y ][2] = (*img_out)( x, y - 1, 1 );
	    seav[ x*t_height + y ][3] = (*img_out)( x, y - 1, 2 );
	    /* RVB of the top pixel (PATCH) */
	    seav[ x*t_height + y ][4] = (*img_in)( i, j - 1, 0 );
	    seav[ x*t_height + y ][5] = (*img_in)( i, j - 1, 1 );
	    seav[ x*t_height + y ][6] = (*img_in)( i, j - 1, 2 );
	    /* RVB of bottom pixel (BG)*/
	    seav[ x*t_height + y ][7] = (*img_out)( x, y, 0 );
	    seav[ x*t_height + y ][8] = (*img_out)( x, y, 1 );
	    seav[ x*t_height + y ][9] = (*img_out)( x, y, 2 );
	    /* RVB of the bottom pixel (PATCH) */
	    seav[ x*t_height + y ][10] = (*img_in)( i, j, 0 );
	    seav[ x*t_height + y ][11] = (*img_in)( i, j, 1 );
	    seav[ x*t_height + y ][12] = (*img_in)( i, j, 2 );

	    /* Look at THE LEFT PIXEL: IF different origin, THEN add seam node */
	    uint_t xx = (i - 1 + pos[0] + pos[3]) % t_width;
	    uint_t yy = (j + pos[1] + pos[5]) % t_height;
	    if ( G->what_segment( nodes[ (*img_msk)( xx, yy ) - 1 ] ) == Graph::SINK ) {
	      
	      /* ADD SEAM NODE ON THE LEFT */
	      seah[ x*t_height + y ][0] = 1;
	      /* RVB of left pixel (BG)*/
	      seah[ x*t_height + y ][1] = (*img_out)( x-1, y, 0 );
	      seah[ x*t_height + y ][2] = (*img_out)( x-1, y, 1 );
	      seah[ x*t_height + y ][3] = (*img_out)( x-1, y, 2 );
	      /* RVB of the left pixel (PATCH) */
	      seah[ x*t_height + y ][4] = (*img_in)( i-1, j, 0 );
	      seah[ x*t_height + y ][5] = (*img_in)( i-1, j, 1 );
	      seah[ x*t_height + y ][6] = (*img_in)( i-1, j, 2 );
	      /* RVB of right pixel (BG)*/
	      seah[ x*t_height + y ][7] = (*img_out)( x, y, 0 );
	      seah[ x*t_height + y ][8] = (*img_out)( x, y, 1 );
	      seah[ x*t_height + y ][9] = (*img_out)( x, y, 2 );
	      /* RVB of the right pixel (PATCH) */
	      seah[ x*t_height + y ][10] = (*img_in)( i, j, 0 );
	      seah[ x*t_height + y ][11] = (*img_in)( i, j, 1 );
	      seah[ x*t_height + y ][12] = (*img_in)( i, j, 2 );
	    }

	  }
	  else /* set white pixel there, erase previous seams */
	    (*img_err)( x, y ) = 255;
	}
	/* superposition => do not nb_pixels++ */
	else  if ( G->what_segment(nodes[ nodex-1 ] ) == Graph::SINK ){
	  /* SINK: DO NOT COPY ANYTHING */
	  sinks++;

	  if ( lastSource ) {

	    /* Draw seam */
	    (*img_err)( x, y ) = 0;
	    lastSource = false;

	    /* ***** ADDING SEAM NODES ****** */

	    /* ADD SEAM NODE ON THE TOP */
	    seav[ x*t_height + y ][0] = 1;
	    /* RVB of top pixel (in PATCH)*/
	    seav[ x*t_height + y ][1] = (*img_in)( i, j-1, 0 );
	    seav[ x*t_height + y ][2] = (*img_in)( i, j-1, 1 );
	    seav[ x*t_height + y ][3] = (*img_in)( i, j-1, 2 );
	    /* RVB of top pixel (in BG)*/
	    seav[ x*t_height + y ][4] = (*img_out)( x, y-1, 0 );
	    seav[ x*t_height + y ][5] = (*img_out)( x, y-1, 1 );
	    seav[ x*t_height + y ][6] = (*img_out)( x, y-1, 2 );
	    /* RVB of the bottom pixel (in PATCH) */
	    seav[ x*t_height + y ][7] = (*img_in)( i, j, 0);
	    seav[ x*t_height + y ][8] = (*img_in)( i, j, 1);
	    seav[ x*t_height + y ][9] = (*img_in)( i, j, 2);
	    /* RVB of the bottom pixel (in BG) */
	    seav[ x*t_height + y ][10] = (*img_out)( x, y, 0 );
	    seav[ x*t_height + y ][11] = (*img_out)( x, y, 1 );
	    seav[ x*t_height + y ][12] = (*img_out)( x, y, 2 );

	    /* Look at THE LEFT PIXEL: IF different origin, THEN add seam node */
	    uint_t xx = (i - 1 + pos[0] + pos[3]) % t_width;
	    uint_t yy = (j + pos[1] + pos[5]) % t_height;
	    if ( G->what_segment( nodes[ (*img_msk)( xx, yy ) - 1 ] ) == Graph::SOURCE ) {
	      
	      /* ADD SEAM NODE ON THE LEFT */
	      seah[ x*t_height + y ][0] = 1;

	      /* RVB of left pixel (in PATCH)*/
	      seah[ x*t_height + y ][1] = (*img_in)( i-1, j, 0 );
	      seah[ x*t_height + y ][2] = (*img_in)( i-1, j, 1 );
	      seah[ x*t_height + y ][3] = (*img_in)( i-1, j, 2 );
	      /* RVB of left pixel (in BG)*/
	      seah[ x*t_height + y ][4] = (*img_out)( x-1, y, 0 );
	      seah[ x*t_height + y ][5] = (*img_out)( x-1, y, 1 );
	      seah[ x*t_height + y ][6] = (*img_out)( x-1, y, 2 );
	      /* RVB of the right pixel (in PATCH) */
	      seah[ x*t_height + y ][7] = (*img_in)( i, j, 0);
	      seah[ x*t_height + y ][8] = (*img_in)( i, j, 1);
	      seah[ x*t_height + y ][9] = (*img_in)( i, j, 2);
	      /* RVB of the right pixel (in BG) */
	      seah[ x*t_height + y ][10] = (*img_out)( x, y, 0 );
	      seah[ x*t_height + y ][11] = (*img_out)( x, y, 1 );
	      seah[ x*t_height + y ][12] = (*img_out)( x, y, 2 );
	    }
	  }

	}	  
      }
    }
  }
 
  cout << " Sources: " << sources << " Sinks: " << sinks << endl;
  return 0;
}



Graph::captype Args::graphCost( uint_t * s, uint_t * t, vector< uint_t > offset ) {
  /*
    Graph cost function.
  */
  if ( cost_fx == C1 )
    return graphCostBasic( s, t, offset, cost_reduction );
  return graphCostGradi( s, t, offset );
}


Graph::captype Args::graphCostBasic( uint_t * s, uint_t * t, vector< uint_t > offset, int reduction ) {
  /*
    Simplest matching quality cost function.
    M(s,t,A,D) = |A(s)-B(s)| + |A(t)-B(t)|
    ( A is the current texture, B is the patch )
    We assume that s and t are pixels overlapping,
    when patch placed at the given offset. 
    offset[0] = patch's top-left corner's X
    offset[1] = patch's top-left corner's Y
   
    The returned value is the mean on each channel (RVB).
  */
  uint_t cr, cv, cb, cost;
  uint_t xs = s[0] % t_width;
  uint_t ys = s[1] % t_height;
  uint_t xt = t[0] % t_width;
  uint_t yt = t[1] % t_height;
  uint_t xsi = s[0] - offset[0];
  uint_t ysi = s[1] - offset[1];
  uint_t xti = t[0] - offset[0];
  uint_t yti = t[1] - offset[1];

  cr = abs( (*img_out)( xs, ys, 0 ) - (*img_in)( xsi, ysi, 0 ) )
    +  abs( (*img_out)( xt, yt, 0 ) - (*img_in)( xti, yti, 0 ) );
  cv = abs( (*img_out)( xs, ys, 1 ) - (*img_in)( xsi, ysi, 1 ) )
    +  abs( (*img_out)( xt, yt, 1 ) - (*img_in)( xti, yti, 1 ) );
  cb = abs( (*img_out)( xs, ys, 2 ) - (*img_in)( xsi, ysi, 2 ) )
    +  abs( (*img_out)( xt, yt, 2 ) - (*img_in)( xti, yti, 2 ) );

  cost = (uint_t) ( ( cr + cv + cb ) / 3 );

  return (Graph::captype) cost / reduction;
}


Graph::captype Args::graphCostGradi( uint_t * s, uint_t * t, vector< uint_t > offset ){
  /*
    Matching cost function using gradient of the pixels.
    Each gradient is the mean of the gradients off each channel.
   */
  uint_t gradTextR, gradTextV, gradTextB;
  uint_t gradPatchR, gradPatchV, gradPatchB;
  uint_t xs = s[0] % t_width;
  uint_t ys = s[1] % t_height;
  uint_t xt = t[0] % t_width;
  uint_t yt = t[1] % t_height;

  gradTextR  = abs( (*img_out)( xs, ys, 0 ) - (*img_out)( xt, yt, 0 ) );
  gradTextV  = abs( (*img_out)( xs, ys, 1 ) - (*img_out)( xt, yt, 1 ) );
  gradTextB  = abs( (*img_out)( xs, ys, 2 ) - (*img_out)( xt, yt, 2 ) );

  xs = s[0] - offset[0];
  ys = s[1] - offset[1];
  xt = t[0] - offset[0];
  yt = t[1] - offset[1];

  gradPatchR  = abs( (*img_in)( xs, ys, 0 ) - (*img_in)( xt, yt, 0 ) );
  gradPatchV  = abs( (*img_in)( xs, ys, 1 ) - (*img_in)( xt, yt, 1 ) );
  gradPatchB  = abs( (*img_in)( xs, ys, 2 ) - (*img_in)( xt, yt, 2 ) );

  uint_t grad = (gradTextR + gradTextV + gradTextB) / 3 
    + ( gradPatchR + gradPatchV + gradPatchB ) / 3;

  grad++; /* to avoid zero division */

  return (Graph::captype) ( graphCostBasic( s, t, offset, 1 ) / sqrt((float)grad) );
}


Graph::captype Args::graphCost( uchar_t s1r, uchar_t s1v, uchar_t s1b, 
				uchar_t s2r, uchar_t s2v, uchar_t s2b,
				uchar_t t1r, uchar_t t1v, uchar_t t1b,
				uchar_t t2r, uchar_t t2v, uchar_t t2b
				) {
  /*
    Graph cost function.
  */
  if ( cost_fx == C1 )
    return graphCostBasic( s1r, s1v, s1b, s2r, s2v, s2b,
			   t1r, t1v, t1b, t2r, t2v, t2b, cost_reduction );
  return graphCostGradi( s1r, s1v, s1b, s2r, s2v, s2b,
			   t1r, t1v, t1b, t2r, t2v, t2b );
}


Graph::captype Args::graphCostBasic( uchar_t s1r, uchar_t s1v, uchar_t s1b, 
				     uchar_t s2r, uchar_t s2v, uchar_t s2b,
				     uchar_t t1r, uchar_t t1v, uchar_t t1b,
				     uchar_t t2r, uchar_t t2v, uchar_t t2b,
				     int reduction ) {

  /*
    Simplest matching quality cost function.
    M(s,t,A,D) = |A(s)-B(s)| + |A(t)-B(t)|
    ( A is the current texture, B is the patch )
    We assume that s and t are pixels overlapping,
    when patch placed at the given offset. 
    offset[0] = patch's top-left corner's X
    offset[1] = patch's top-left corner's Y
   
    The returned value is the mean on each channel (RVB).
  */
  uint_t cr, cv, cb, cost;

  cr = abs( s1r - s2r ) +  abs( t1r - t2r );
  cv = abs( s1v - s2v ) +  abs( t1v - t2v );
  cb = abs( s1b - s2b ) +  abs( t1b - t2b );

  cost = (uint_t) ( ( cr + cv + cb ) / 3 );

  return (Graph::captype) cost / reduction;
}


Graph::captype Args::graphCostGradi( uchar_t s1r, uchar_t s1v, uchar_t s1b, 
				     uchar_t s2r, uchar_t s2v, uchar_t s2b,
				     uchar_t t1r, uchar_t t1v, uchar_t t1b,
				     uchar_t t2r, uchar_t t2v, uchar_t t2b
				     ) {
  /*
    Matching cost function using gradient of the pixels.
    Each gradient is the mean of the gradients off each channel.
   */
  uint_t gradTextR, gradTextV, gradTextB;
  uint_t gradPatchR, gradPatchV, gradPatchB;

  gradTextR  = abs( s1r - t1r );
  gradTextV  = abs( s1v - t1v );
  gradTextB  = abs( s1b - t1b );

  gradPatchR  = abs( s2r - t2r );
  gradPatchV  = abs( s2v - t2v );
  gradPatchB  = abs( s2b - t2b );

  uint_t grad = (gradTextR + gradTextV + gradTextB) / 3 
    + ( gradPatchR + gradPatchV + gradPatchB ) / 3;

  grad++; /* to avoid zero division */

  return (Graph::captype) ( graphCostBasic( s1r, s1v, s1b, s2r, s2v, s2b,
					    t1r, t1v, t1b, t2r, t2v, t2b, 1 ) / sqrt((float)grad) );
}
