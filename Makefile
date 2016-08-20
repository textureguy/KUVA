#/*******************************
# *
# * Kuva - Graph cut texturing
# * 
# * From:
# *      V.Kwatra, A.Sch�dl, I.Essa, G.Turk, A.Bobick, 
# *      Graphcut Textures: Image and Video Synthesis Using Graph Cuts
# *      http://www.cc.gatech.edu/cpl/projects/graphcuttextures/
# *
# * JP <jeanphilippe.aumasson@gmail.com>
# *
# * main.cpp
# *
# * 01/2006
# *
# *******************************/
#
#  Copyright Jean-Philippe Aumasson, 2005, 2006
#
#  Kuva is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#  
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA



CC     = g++
cC     = $(CC) -c
CFLAG  = -Wall -O3 -ffast-math -fstrict-aliasing -malign-double
# avoid warnings for B. & K. code
GFLAG  = -O3 -ffast-math -fstrict-aliasing -malign-double
LFLAG  = -lX11 -lm -lpthread
DFLAG  = -g
BIN    = kuva
OPATH  = src
VPATH  = src
INSTALL_PATH = /usr/bin
TRASH  = *~ *.bmp *.jpg *.png temp *.sha256  a.out $(BIN) $(VPATH)/*.o $(VPATH)/*~ 

all:		$(BIN)

$(BIN):		main.o args.o  argsgraph.o graph.o maxflow.o
		$(CC) $(OPATH)/main.o $(OPATH)/args.o $(OPATH)/argsgraph.o \
		$(OPATH)/graph.o $(OPATH)/maxflow.o -o $@ $(LFLAG)

main.o:		main.cpp
		$(cC) $(CFLAG) $^ -o $(VPATH)/$@

args.o:		args.cpp
		$(cC) $(CFLAG) $^ -o $(VPATH)/$@

argsgraph.o:	argsgraph.cpp
		$(cC) $(CFLAG) $^ -o $(VPATH)/$@

graph.o:	graph.cpp
		$(cC) $(GFLAG) $^ -o $(VPATH)/$@

maxflow.o:	maxflow.cpp
		$(cC) $(GFLAG) $^ -o $(VPATH)/$@

clean:
		rm -f $(TRASH)
