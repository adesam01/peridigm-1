//@HEADER
// ************************************************************************
//
//                             Peridigm
//                 Copyright (2011) Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions?
// David J. Littlewood   djlittl@sandia.gov
// John A. Mitchell      jamitch@sandia.gov
// Michael L. Parks      mlparks@sandia.gov
// Stewart A. Silling    sasilli@sandia.gov
//
// ************************************************************************
//@HEADER

#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <string.h>
#include <vector>


#include "mesh_output/Field.h"
#include "mesh_input/quick_grid/QuickGrid.h"
#include "bond_volume/quick_grid/calculators.h"
#include "material_utilities.h"
#include "pdneigh/NeighborhoodList.h"
#include "pdneigh/PdZoltan.h"
#include "pdneigh/BondFilter.h"
#include "utilities/Array.h"
#include <Teuchos_ParameterList.hpp>
#include <Teuchos_UnitTestHarness.hpp>


#include "Epetra_ConfigDefs.h"
#ifdef HAVE_MPI
#include "mpi.h"
#include "Epetra_MpiComm.h"
#else
#include "Epetra_SerialComm.h"
#endif


using std::size_t;
using std::tr1::shared_ptr;
using UTILITIES::Array;
using UTILITIES::Vector3D;
using std::pair;
using std::cout;
using std::endl;

static int nx;
static int ny;
static int nz;
const size_t numProcs=1;
const size_t myRank=0;
double horizon;
std::string neighborhoodType;
std::string type;
//using namespace std;

void probe_shear
(
		MATERIAL_EVALUATION::PURE_SHEAR mode,
		Array<int> neighborhoodPtr,
		Array<double> X,
		Array<double> xPtr,
		Array<double> Y,
		Array<double> yPtr,
		Array<double> bondVolume,
		double horizon,
		double gamma,
		double m_code
);

string findNodeValue( string& str, const std::string &nodename){

      const char delimiter[2] = ":";
      char *token;
      char *first;
      char *last;
      char *destination = 0;

      
      const char* node_name = &nodename[0];
      
     
      char* tempStringContainer = (char*)malloc(sizeof(char)*(str.length() + 1));
     

      for (unsigned int i = 0; i < str.length(); i++){
           tempStringContainer[i] = str[i];
      }

      tempStringContainer[str.length()] = '\0';
     
      token = strtok( tempStringContainer, delimiter);

      while (token != NULL){
                                   
             if (strstr( token, node_name) == NULL) {

                if (strrchr(token, '"') != NULL){

                   first = strchr(token, '"');
                   last  = strrchr(token, '"');

                   int first_occurrence = first - token + 1;
                   int last_occurrence = last - token + 1;

                   destination = (char*)malloc(sizeof(char)*(last_occurrence - first_occurrence));

                   int t = 0;
                                         
                   for (int i = first_occurrence; i < (last_occurrence - 1); i++){
                                             
                        destination[t] = token[i];
                        t++;

                   }

                   destination[t] ='\0';

               }


               else{
                
                                   
                 if ( (strrchr( token, ',') != NULL)  && (token[strlen(token) - 1] == ',') ) {
                       destination = (char*)malloc(sizeof(char)*strlen(token));
                                        
                       strncpy( destination, token, strlen(token) - 1 );
                       destination[strlen(token) - 1] = '\0';
                        
                 }
                 else{
                      destination = (char*)malloc(sizeof(char)*(strlen(token) + 1));

                      strncpy( destination, token, (strlen(token) + 1) );
                      destination[strlen(token) + 1] = '\0';
                      
                 }

               }   
               
             }
                           
             token = strtok( NULL, delimiter);
        }

     
       free(tempStringContainer);
      
       string ss(destination);

       return ss;
       
}


void set_static_data(const std::string& json_filename)
{
	
        std::ifstream inFile;
        string str;

        size_t found;
        

        inFile.open(json_filename.c_str());

         if(!inFile.is_open())
             cout << "\n**** Warning:  This test can only be run from the directory where it resides (otherwise it won't find the input files) ****\n" << endl;

         while(inFile.good()){
                getline(inFile, str);

               if( !(str.size() == 0) ){

                  // Discretization node starts here!

                   if  ( (found = str.find("Discretization")) !=  string::npos){

                          getline(inFile, str);

                          while ((str.size() == 0)) getline(inFile, str);

                                if ((found = str.find("Horizon")) !=  string::npos){

                                  
                                   string ss = findNodeValue(str, "Horizon");
                                   
                                   horizon = atof( ss.c_str());
                                  

                               }

                       getline(inFile, str);
                       while ((str.size() == 0)) getline(inFile, str);

                       if ((found = str.find("NeighborhoodType")) !=  string::npos){
                            
                            neighborhoodType = findNodeValue(str, "NeighborhoodType");
                                   
                       }

                        getline(inFile, str);
                        while ((str.size() == 0)) getline(inFile, str);

                        if ((found = str.find("Type")) !=  string::npos){
 
                             type = findNodeValue(str, "Type");
                             
                       }


                   }     // Discretization node ends here!



                   if  ( (found = str.find("Number Points X")) !=  string::npos){

                          string ss= findNodeValue(str, "Number Points X");
                          nx = atoi( ss.c_str());

                   }

                    if  ( (found = str.find("Number Points Y")) !=  string::npos){

                         
                          string ss= findNodeValue(str, "Number Points Y");   
                          ny = atoi( ss.c_str());
                         
                   }

                    if  ( (found = str.find("Number Points Z")) !=  string::npos){
                          
                           string ss= findNodeValue(str, "Number Points Z");      
                           nz = atoi( ss.c_str());
                           
                   }
                  
                }

           
         }

         inFile.close();


}




void write_table_1_header(const std::string& output_tex_table){
	std::stringstream table_out;

	table_out << "\\begin{table}[ht]" << "\n";
	table_out << "\\centering" << "\n";
	table_out << "\\bigskip" << "\n";
	table_out << "\\begin{tabular}{|c|c|c|c|}" << "\n";
	table_out << "\\hline" << "\n";
	table_out << "$n$ "
			    << "& $\\frac{|m-m_n|}{m}$ "
			    << "& $\\frac{\\Vert e^d\\Vert^2-\\Vert e^d_n\\Vert^2}{\\Vert e^d\\Vert^2}$ "
			    << "& $\\frac{\\Vert e^d\\Vert^2}{\\Vert e^d_n\\Vert^2}$ \\\\" << "\n";
	table_out << "\\hline" << "\n";


	std::ofstream file_stream;
	file_stream.open(output_tex_table.c_str(),std::ios::app|std::ios::out);

	file_stream << table_out.str();
	file_stream.close();

}

void close_table_1(const std::string& output_tex_table) {
	std::stringstream table_out;

	table_out << "\\hline" << "\n";
	table_out << "\\end{tabular}" << "\n";
	table_out << "\\end{table}" << "\n";
	std::ofstream file_stream;
	file_stream.open(output_tex_table.c_str(),std::ios::app|std::ios::out);
	file_stream << table_out.str();
	file_stream.close();
}


QUICKGRID::QuickGridData getGrid(const string& _json_filename) {
	shared_ptr<QUICKGRID::QuickGridMeshGenerationIterator> g;
	g = QUICKGRID::getMeshGenerator(numProcs,_json_filename);
	QUICKGRID::QuickGridData decomp =  QUICKGRID::getDiscretization(myRank, *g);

	// This load-balances
	decomp = PDNEIGH::getLoadBalancedDiscretization(decomp);
	return decomp;
}

void scf_probe_two( Array<int> &neighborhoodPtr, Array<double> xPtr, Array<double> &X, Array<double> &Y, Array<double> &bondVolume, shared_ptr<BOND_VOLUME::QUICKGRID::Bond_Volume_Calculator> c, double horizon, Array<double> &yPtr, PDNEIGH::NeighborhoodList list, size_t gId, size_t num_neigh, double &theta, double &m_code, double gamma){

      const int *neighborhood = list.get_neighborhood(gId);
		
      for(size_t j=0;j<num_neigh+1;j++,neighborhood++)
	  neighborhoodPtr[j]=*neighborhood;

      X.set(0.0);
      Y.set(0.0);
      bondVolume.set(0.0);

      BOND_VOLUME::QUICKGRID::compute_bond_volume(X.get(),neighborhoodPtr.get(),xPtr.get(),bondVolume.get(),c.get());

      double m_analytical = 4.0 * M_PI * pow(horizon,5) / 5.0;
      m_code = MATERIAL_EVALUATION::WITH_BOND_VOLUME::computeWeightedVolume(X.get(),xPtr.get(),bondVolume.get(),neighborhoodPtr.get(),horizon);
      double rel_diff = std::abs(m_analytical-m_code)/m_analytical;
      std::cout << std::scientific;
      std::cout.precision(3);
      std::cout << "ut_scf::analytical value for weighted volume on sphere = " << m_analytical << std::endl;
      std::cout << "ut_scf::code computed weighted volume on sphere = " << m_code << std::endl;
      std::cout << "ut_scf::% relative error weighted volume = " << 100*rel_diff << std::endl;


      MATERIAL_EVALUATION::set_pure_shear(neighborhoodPtr.get(),X.get(),xPtr.get(),yPtr.get(),MATERIAL_EVALUATION::XY,gamma);
      theta = MATERIAL_EVALUATION::WITH_BOND_VOLUME::computeDilatation(neighborhoodPtr.get(),X.get(),xPtr.get(),Y.get(),yPtr.get(),bondVolume.get(),m_code,horizon);

}


void scf_probe_one( shared_ptr<BOND_VOLUME::QUICKGRID::Bond_Volume_Calculator> &c, double &horizon, QUICKGRID::QuickGridData &gridData, double &cell_diagonal, const string& json_filename){
  
     c = BOND_VOLUME::QUICKGRID::get_Bond_Volume_Calculator(json_filename);
     horizon = c->get_horizon();
     cell_diagonal=c->get_cell_diagonal();
     gridData = getGrid(json_filename);

     // This load-balances
     gridData = PDNEIGH::getLoadBalancedDiscretization(gridData);

}


void probe_shear
(
	MATERIAL_EVALUATION::PURE_SHEAR mode,
	Array<int> neighborhoodPtr,
	Array<double> X,
	Array<double> xPtr,
	Array<double> Y,
	Array<double> yPtr,
	Array<double> bondVolume,
	double horizon,
	double gamma,
	double m_code
)
{

	/*
	 * This is the reference value for the weighted volume
	 */
	double m_analytical = 4.0 * M_PI * pow(horizon,5) / 5.0;
	double m_err = std::fabs(m_analytical-m_code)/m_analytical;
	/*
	 * NOTE: X is center of sphere and there no displacement at this point
	 * therefore, Y=X
	 */
	
	/*
	 * compute shear correction factor
	 */
	/*
	 * This is the reference value for ed_squared
	 */
	double reference = 4.0 * M_PI * gamma * gamma * pow(horizon,5) / 75.0;
	double ed2 = MATERIAL_EVALUATION::WITH_BOND_VOLUME::compute_norm_2_deviatoric_extension(neighborhoodPtr.get(),X.get(),xPtr.get(),X.get(),yPtr.get(),bondVolume.get(),m_code,horizon);
	double scf = reference/ed2;
	double ed_err = fabs(reference-ed2)/reference;
	std::cout << "ut_scf::probe_shear MODE = " << mode << std::endl;
	std::cout << "ut_scf::ed = " << reference << std::endl;
	cout.precision(2);
	std::cout << std::scientific << "ut_scf::probe_shear computed % ed_err in pure shear = " << 100*ed_err << std::endl;
	std::cout << "ut_scf::probe_shear computed scf in pure shear = " << scf << std::endl;
	/*
	 * For this nearly perfect 'sphere', the shear correction factor should be very close to '1.0'
	 */

	std::stringstream table_1_out;
	table_1_out << nx << " & ";
	table_1_out.precision(4);
	table_1_out << m_err*100 << "\\% & ";
	table_1_out << ed_err*100 << "\\% & ";
	table_1_out.precision(3);
	table_1_out << scf << " \\\\ \n";

	std::ofstream file_stream;
	file_stream.open("table_1.tex",std::ios::app|std::ios::out);
	file_stream << table_1_out.str();
	file_stream.close();

	/*
	 * write raw data
	 */
	file_stream.open("ut_bondVolumeConvergenceStudy.dat",std::ios::app|std::ios::out);
	file_stream << nx << " ";
	file_stream << std::scientific;
	file_stream.precision(12);
	file_stream << 2.0*horizon/nx << " ";
	file_stream << m_code << " ";
	file_stream << ed2 << "\n";
	file_stream.close();

}


TEUCHOS_UNIT_TEST(BondVolumeConvergenceStudy, n3Test) {

	std::string json_filename = "./input_files/ut_bondVolumeConvergenceStudy_n=3.json";

        set_static_data(json_filename);

         /*
	 * Unit test looks exclusively at the cell at the center of cube;
	 * This cell ID depends upon nx, ny, nz
	 *
	 * MESH INPUT MUST HAVE EVEN NUMBER OF CELLS
	 */
	TEST_ASSERT(0==(nx+1)%2);
	/*
	 * mesh must have equal number of cells along each axis
	 */
	TEST_ASSERT(nx==ny);
	TEST_ASSERT(nx==nz);

        shared_ptr<BOND_VOLUME::QUICKGRID::Bond_Volume_Calculator> c;
        double horizon;
        QUICKGRID::QuickGridData gridData;
        double cell_diagonal;

	/*
	 * Create neighborhood with an enlarged horizon
	 */

        
        scf_probe_one( c, horizon, gridData, cell_diagonal, json_filename);
  
	shared_ptr<Epetra_Comm> comm(new Epetra_MpiComm(MPI_COMM_WORLD));
	PDNEIGH::NeighborhoodList list(comm,gridData.zoltanPtr.get(),gridData.numPoints,gridData.myGlobalIDs,gridData.myX,horizon+cell_diagonal);

	// coordinate indices of center cell
	size_t ic = (nx -1)/2;
	size_t jc = ic;
	size_t kc = ic;
	size_t gId = nx * ny * kc + nx * jc + ic;


	/**
	 * WARNING: note following ASSUMPTION -- gId == local id
	 * CAUTION: this test only makes sense in 'serial' -- local id
	 * and gId are not the same in parallel.
	 */
	size_t num_neigh = list.get_num_neigh(gId);

	Array<int> neighborhoodPtr(1+num_neigh);

        const int *neighborhood = list.get_neighborhood(gId);
        TEST_ASSERT((int)num_neigh == *neighborhood);
	
	Array<double> xPtr(list.get_num_owned_points()*3,list.get_owned_x());

	/*
	 * expectation is that cell center is at origin
	 */


	TEST_COMPARE(xPtr[3*gId+0], <=, 1.0e-15);
	TEST_COMPARE(xPtr[3*gId+1], <=, 1.0e-15);
	TEST_COMPARE(xPtr[3*gId+2], <=, 1.0e-15);
	/*
	 * X is the center of the sphere
	 */
	Array<double> X(3); 
	/*
	 * Y = X since we are fixing the center of the sphere
	 */
	Array<double> Y(3); 

        Array<double> yPtr(3*list.get_num_owned_points());
	/*
	 * only computing bondVolume on center point 'X'
	 */

	Array<double> bondVolume(num_neigh);
        double theta, m_code;
	
	double gamma = 1.0e-6;
        scf_probe_two( neighborhoodPtr, xPtr, X,Y, bondVolume, c, horizon, yPtr, list , gId, num_neigh, theta, m_code, gamma);

	double tolerance=1.0e-12;
       
	TEST_COMPARE(theta, <=, tolerance);


	/*
	 * PROBE XY
	 */
	probe_shear(MATERIAL_EVALUATION::XY,neighborhoodPtr,X,xPtr,Y,yPtr,bondVolume,horizon,gamma,m_code);
	
}

TEUCHOS_UNIT_TEST(BondVolumeConvergenceStudy, n5Test) {

	std::string json_filename = "./input_files/ut_bondVolumeConvergenceStudy_n=5.json";

        set_static_data(json_filename);

         /*
	 * Unit test looks exclusively at the cell at the center of cube;
	 * This cell ID depends upon nx, ny, nz
	 *
	 * MESH INPUT MUST HAVE EVEN NUMBER OF CELLS
	 */
	TEST_ASSERT(0==(nx+1)%2);
	/*
	 * mesh must have equal number of cells along each axis
	 */
	TEST_ASSERT(nx==ny);
	TEST_ASSERT(nx==nz);

        shared_ptr<BOND_VOLUME::QUICKGRID::Bond_Volume_Calculator> c;
        double horizon;
        QUICKGRID::QuickGridData gridData;
        double cell_diagonal;

	/*
	 * Create neighborhood with an enlarged horizon
	 */

        
        scf_probe_one( c, horizon, gridData, cell_diagonal, json_filename);
  
	shared_ptr<Epetra_Comm> comm(new Epetra_MpiComm(MPI_COMM_WORLD));
	PDNEIGH::NeighborhoodList list(comm,gridData.zoltanPtr.get(),gridData.numPoints,gridData.myGlobalIDs,gridData.myX,horizon+cell_diagonal);

	// coordinate indices of center cell
	size_t ic = (nx -1)/2;
	size_t jc = ic;
	size_t kc = ic;
	size_t gId = nx * ny * kc + nx * jc + ic;


	/**
	 * WARNING: note following ASSUMPTION -- gId == local id
	 * CAUTION: this test only makes sense in 'serial' -- local id
	 * and gId are not the same in parallel.
	 */
	size_t num_neigh = list.get_num_neigh(gId);

	Array<int> neighborhoodPtr(1+num_neigh);

        const int *neighborhood = list.get_neighborhood(gId);
        TEST_ASSERT((int)num_neigh == *neighborhood);
	
	Array<double> xPtr(list.get_num_owned_points()*3,list.get_owned_x());

	/*
	 * expectation is that cell center is at origin
	 */


	TEST_COMPARE(xPtr[3*gId+0], <=, 1.0e-15);
	TEST_COMPARE(xPtr[3*gId+1], <=, 1.0e-15);
	TEST_COMPARE(xPtr[3*gId+2], <=, 1.0e-15);
	/*
	 * X is the center of the sphere
	 */
	Array<double> X(3); 
	/*
	 * Y = X since we are fixing the center of the sphere
	 */
	Array<double> Y(3); 

        Array<double> yPtr(3*list.get_num_owned_points());
	/*
	 * only computing bondVolume on center point 'X'
	 */

	Array<double> bondVolume(num_neigh);
        double theta, m_code;
	
	double gamma = 1.0e-6;
        scf_probe_two( neighborhoodPtr, xPtr, X,Y, bondVolume, c, horizon, yPtr, list , gId, num_neigh, theta, m_code, gamma);

	double tolerance=1.0e-12;
       
	TEST_COMPARE(theta, <=, tolerance);


	/*
	 * PROBE XY
	 */
	probe_shear(MATERIAL_EVALUATION::XY,neighborhoodPtr,X,xPtr,Y,yPtr,bondVolume,horizon,gamma,m_code);
	
	
}

TEUCHOS_UNIT_TEST(BondVolumeConvergenceStudy, n7Test) {

	std::string json_filename = "./input_files/ut_bondVolumeConvergenceStudy_n=7.json";
        set_static_data(json_filename);

         /*
	 * Unit test looks exclusively at the cell at the center of cube;
	 * This cell ID depends upon nx, ny, nz
	 *
	 * MESH INPUT MUST HAVE EVEN NUMBER OF CELLS
	 */
	TEST_ASSERT(0==(nx+1)%2);
	/*
	 * mesh must have equal number of cells along each axis
	 */
	TEST_ASSERT(nx==ny);
	TEST_ASSERT(nx==nz);

        shared_ptr<BOND_VOLUME::QUICKGRID::Bond_Volume_Calculator> c;
        double horizon;
        QUICKGRID::QuickGridData gridData;
        double cell_diagonal;

	/*
	 * Create neighborhood with an enlarged horizon
	 */

        
        scf_probe_one( c, horizon, gridData, cell_diagonal, json_filename);
  
	shared_ptr<Epetra_Comm> comm(new Epetra_MpiComm(MPI_COMM_WORLD));
	PDNEIGH::NeighborhoodList list(comm,gridData.zoltanPtr.get(),gridData.numPoints,gridData.myGlobalIDs,gridData.myX,horizon+cell_diagonal);

	// coordinate indices of center cell
	size_t ic = (nx -1)/2;
	size_t jc = ic;
	size_t kc = ic;
	size_t gId = nx * ny * kc + nx * jc + ic;


	/**
	 * WARNING: note following ASSUMPTION -- gId == local id
	 * CAUTION: this test only makes sense in 'serial' -- local id
	 * and gId are not the same in parallel.
	 */
	size_t num_neigh = list.get_num_neigh(gId);

	Array<int> neighborhoodPtr(1+num_neigh);

        const int *neighborhood = list.get_neighborhood(gId);
        TEST_ASSERT((int)num_neigh == *neighborhood);
	
	Array<double> xPtr(list.get_num_owned_points()*3,list.get_owned_x());

	/*
	 * expectation is that cell center is at origin
	 */


	TEST_COMPARE(xPtr[3*gId+0], <=, 1.0e-15);
	TEST_COMPARE(xPtr[3*gId+1], <=, 1.0e-15);
	TEST_COMPARE(xPtr[3*gId+2], <=, 1.0e-15);
	/*
	 * X is the center of the sphere
	 */
	Array<double> X(3); 
	/*
	 * Y = X since we are fixing the center of the sphere
	 */
	Array<double> Y(3); 

        Array<double> yPtr(3*list.get_num_owned_points());
	/*
	 * only computing bondVolume on center point 'X'
	 */

	Array<double> bondVolume(num_neigh);
        double theta, m_code;
	
	double gamma = 1.0e-6;
        scf_probe_two( neighborhoodPtr, xPtr, X,Y, bondVolume, c, horizon, yPtr, list , gId, num_neigh, theta, m_code, gamma);

	double tolerance=1.0e-12;
       
	TEST_COMPARE(theta, <=, tolerance);


	/*
	 * PROBE XY
	 */
	probe_shear(MATERIAL_EVALUATION::XY,neighborhoodPtr,X,xPtr,Y,yPtr,bondVolume,horizon,gamma,m_code);
	
}


TEUCHOS_UNIT_TEST(BondVolumeConvergenceStudy, n9Test) {

	std::string json_filename = "./input_files/ut_bondVolumeConvergenceStudy_n=9.json";
        set_static_data(json_filename);

         /*
	 * Unit test looks exclusively at the cell at the center of cube;
	 * This cell ID depends upon nx, ny, nz
	 *
	 * MESH INPUT MUST HAVE EVEN NUMBER OF CELLS
	 */
	TEST_ASSERT(0==(nx+1)%2);
	/*
	 * mesh must have equal number of cells along each axis
	 */
	TEST_ASSERT(nx==ny);
	TEST_ASSERT(nx==nz);

        shared_ptr<BOND_VOLUME::QUICKGRID::Bond_Volume_Calculator> c;
        double horizon;
        QUICKGRID::QuickGridData gridData;
        double cell_diagonal;

	/*
	 * Create neighborhood with an enlarged horizon
	 */

        
        scf_probe_one( c, horizon, gridData, cell_diagonal, json_filename);
  
	shared_ptr<Epetra_Comm> comm(new Epetra_MpiComm(MPI_COMM_WORLD));
	PDNEIGH::NeighborhoodList list(comm,gridData.zoltanPtr.get(),gridData.numPoints,gridData.myGlobalIDs,gridData.myX,horizon+cell_diagonal);

	// coordinate indices of center cell
	size_t ic = (nx -1)/2;
	size_t jc = ic;
	size_t kc = ic;
	size_t gId = nx * ny * kc + nx * jc + ic;


	/**
	 * WARNING: note following ASSUMPTION -- gId == local id
	 * CAUTION: this test only makes sense in 'serial' -- local id
	 * and gId are not the same in parallel.
	 */
	size_t num_neigh = list.get_num_neigh(gId);

	Array<int> neighborhoodPtr(1+num_neigh);

        const int *neighborhood = list.get_neighborhood(gId);
        TEST_ASSERT((int)num_neigh == *neighborhood);
	
	Array<double> xPtr(list.get_num_owned_points()*3,list.get_owned_x());

	/*
	 * expectation is that cell center is at origin
	 */


	TEST_COMPARE(xPtr[3*gId+0], <=, 1.0e-15);
	TEST_COMPARE(xPtr[3*gId+1], <=, 1.0e-15);
	TEST_COMPARE(xPtr[3*gId+2], <=, 1.0e-15);
	/*
	 * X is the center of the sphere
	 */
	Array<double> X(3); 
	/*
	 * Y = X since we are fixing the center of the sphere
	 */
	Array<double> Y(3); 

        Array<double> yPtr(3*list.get_num_owned_points());
	/*
	 * only computing bondVolume on center point 'X'
	 */

	Array<double> bondVolume(num_neigh);
        double theta, m_code;
	
	double gamma = 1.0e-6;
        scf_probe_two( neighborhoodPtr, xPtr, X,Y, bondVolume, c, horizon, yPtr, list , gId, num_neigh, theta, m_code, gamma);

	double tolerance=1.0e-12;
       
	TEST_COMPARE(theta, <=, tolerance);


	/*
	 * PROBE XY
	 */
	probe_shear(MATERIAL_EVALUATION::XY,neighborhoodPtr,X,xPtr,Y,yPtr,bondVolume,horizon,gamma,m_code);
	
}

TEUCHOS_UNIT_TEST(BondVolumeConvergenceStudy, n11Test) {

	std::string json_filename = "./input_files/ut_bondVolumeConvergenceStudy_n=11.json";

        set_static_data(json_filename);

         /*
	 * Unit test looks exclusively at the cell at the center of cube;
	 * This cell ID depends upon nx, ny, nz
	 *
	 * MESH INPUT MUST HAVE EVEN NUMBER OF CELLS
	 */
	TEST_ASSERT(0==(nx+1)%2);
	/*
	 * mesh must have equal number of cells along each axis
	 */
	TEST_ASSERT(nx==ny);
	TEST_ASSERT(nx==nz);

        shared_ptr<BOND_VOLUME::QUICKGRID::Bond_Volume_Calculator> c;
        double horizon;
        QUICKGRID::QuickGridData gridData;
        double cell_diagonal;

	/*
	 * Create neighborhood with an enlarged horizon
	 */

        
        scf_probe_one( c, horizon, gridData, cell_diagonal, json_filename);
  
	shared_ptr<Epetra_Comm> comm(new Epetra_MpiComm(MPI_COMM_WORLD));
	PDNEIGH::NeighborhoodList list(comm,gridData.zoltanPtr.get(),gridData.numPoints,gridData.myGlobalIDs,gridData.myX,horizon+cell_diagonal);

	// coordinate indices of center cell
	size_t ic = (nx -1)/2;
	size_t jc = ic;
	size_t kc = ic;
	size_t gId = nx * ny * kc + nx * jc + ic;


	/**
	 * WARNING: note following ASSUMPTION -- gId == local id
	 * CAUTION: this test only makes sense in 'serial' -- local id
	 * and gId are not the same in parallel.
	 */
	size_t num_neigh = list.get_num_neigh(gId);

	Array<int> neighborhoodPtr(1+num_neigh);

        const int *neighborhood = list.get_neighborhood(gId);
        TEST_ASSERT((int)num_neigh == *neighborhood);
	
	Array<double> xPtr(list.get_num_owned_points()*3,list.get_owned_x());

	/*
	 * expectation is that cell center is at origin
	 */


	TEST_COMPARE(xPtr[3*gId+0], <=, 1.0e-15);
	TEST_COMPARE(xPtr[3*gId+1], <=, 1.0e-15);
	TEST_COMPARE(xPtr[3*gId+2], <=, 1.0e-15);
	/*
	 * X is the center of the sphere
	 */
	Array<double> X(3); 
	/*
	 * Y = X since we are fixing the center of the sphere
	 */
	Array<double> Y(3); 

        Array<double> yPtr(3*list.get_num_owned_points());
	/*
	 * only computing bondVolume on center point 'X'
	 */

	Array<double> bondVolume(num_neigh);
        double theta, m_code;
	
	double gamma = 1.0e-6;
        scf_probe_two( neighborhoodPtr, xPtr, X,Y, bondVolume, c, horizon, yPtr, list , gId, num_neigh, theta, m_code, gamma);

	double tolerance=1.0e-12;
       
	TEST_COMPARE(theta, <=, tolerance);


	/*
	 * PROBE XY
	 */
	probe_shear(MATERIAL_EVALUATION::XY,neighborhoodPtr,X,xPtr,Y,yPtr,bondVolume,horizon,gamma,m_code);
	
}

TEUCHOS_UNIT_TEST(BondVolumeConvergenceStudy, n13Test) {


	std::string json_filename = "./input_files/ut_bondVolumeConvergenceStudy_n=13.json";
        set_static_data(json_filename);

         /*
	 * Unit test looks exclusively at the cell at the center of cube;
	 * This cell ID depends upon nx, ny, nz
	 *
	 * MESH INPUT MUST HAVE EVEN NUMBER OF CELLS
	 */
	TEST_ASSERT(0==(nx+1)%2);
	/*
	 * mesh must have equal number of cells along each axis
	 */
	TEST_ASSERT(nx==ny);
	TEST_ASSERT(nx==nz);

        shared_ptr<BOND_VOLUME::QUICKGRID::Bond_Volume_Calculator> c;
        double horizon;
        QUICKGRID::QuickGridData gridData;
        double cell_diagonal;

	/*
	 * Create neighborhood with an enlarged horizon
	 */

        
        scf_probe_one( c, horizon, gridData, cell_diagonal, json_filename);
  
	shared_ptr<Epetra_Comm> comm(new Epetra_MpiComm(MPI_COMM_WORLD));
	PDNEIGH::NeighborhoodList list(comm,gridData.zoltanPtr.get(),gridData.numPoints,gridData.myGlobalIDs,gridData.myX,horizon+cell_diagonal);

	// coordinate indices of center cell
	size_t ic = (nx -1)/2;
	size_t jc = ic;
	size_t kc = ic;
	size_t gId = nx * ny * kc + nx * jc + ic;


	/**
	 * WARNING: note following ASSUMPTION -- gId == local id
	 * CAUTION: this test only makes sense in 'serial' -- local id
	 * and gId are not the same in parallel.
	 */
	size_t num_neigh = list.get_num_neigh(gId);

	Array<int> neighborhoodPtr(1+num_neigh);

        const int *neighborhood = list.get_neighborhood(gId);
        TEST_ASSERT((int)num_neigh == *neighborhood);
	
	Array<double> xPtr(list.get_num_owned_points()*3,list.get_owned_x());

	/*
	 * expectation is that cell center is at origin
	 */


	TEST_COMPARE(xPtr[3*gId+0], <=, 1.0e-15);
	TEST_COMPARE(xPtr[3*gId+1], <=, 1.0e-15);
	TEST_COMPARE(xPtr[3*gId+2], <=, 1.0e-15);
	/*
	 * X is the center of the sphere
	 */
	Array<double> X(3); 
	/*
	 * Y = X since we are fixing the center of the sphere
	 */
	Array<double> Y(3); 

        Array<double> yPtr(3*list.get_num_owned_points());
	/*
	 * only computing bondVolume on center point 'X'
	 */

	Array<double> bondVolume(num_neigh);
        double theta, m_code;
	
	double gamma = 1.0e-6;
        scf_probe_two( neighborhoodPtr, xPtr, X,Y, bondVolume, c, horizon, yPtr, list , gId, num_neigh, theta, m_code, gamma);

	double tolerance=1.0e-12;
       
	TEST_COMPARE(theta, <=, tolerance);


	/*
	 * PROBE XY
	 */
	probe_shear(MATERIAL_EVALUATION::XY,neighborhoodPtr,X,xPtr,Y,yPtr,bondVolume,horizon,gamma,m_code);
	
}


TEUCHOS_UNIT_TEST(BondVolumeConvergenceStudy, n17Test) {

	std::string json_filename = "./input_files/ut_bondVolumeConvergenceStudy_n=17.json";
        set_static_data(json_filename);

         /*
	 * Unit test looks exclusively at the cell at the center of cube;
	 * This cell ID depends upon nx, ny, nz
	 *
	 * MESH INPUT MUST HAVE EVEN NUMBER OF CELLS
	 */
	TEST_ASSERT(0==(nx+1)%2);
	/*
	 * mesh must have equal number of cells along each axis
	 */
	TEST_ASSERT(nx==ny);
	TEST_ASSERT(nx==nz);

        shared_ptr<BOND_VOLUME::QUICKGRID::Bond_Volume_Calculator> c;
        double horizon;
        QUICKGRID::QuickGridData gridData;
        double cell_diagonal;

	/*
	 * Create neighborhood with an enlarged horizon
	 */

        
        scf_probe_one( c, horizon, gridData, cell_diagonal, json_filename);
  
	shared_ptr<Epetra_Comm> comm(new Epetra_MpiComm(MPI_COMM_WORLD));
	PDNEIGH::NeighborhoodList list(comm,gridData.zoltanPtr.get(),gridData.numPoints,gridData.myGlobalIDs,gridData.myX,horizon+cell_diagonal);

	// coordinate indices of center cell
	size_t ic = (nx -1)/2;
	size_t jc = ic;
	size_t kc = ic;
	size_t gId = nx * ny * kc + nx * jc + ic;


	/**
	 * WARNING: note following ASSUMPTION -- gId == local id
	 * CAUTION: this test only makes sense in 'serial' -- local id
	 * and gId are not the same in parallel.
	 */
	size_t num_neigh = list.get_num_neigh(gId);

	Array<int> neighborhoodPtr(1+num_neigh);

        const int *neighborhood = list.get_neighborhood(gId);
        TEST_ASSERT((int)num_neigh == *neighborhood);
	
	Array<double> xPtr(list.get_num_owned_points()*3,list.get_owned_x());

	/*
	 * expectation is that cell center is at origin
	 */


	TEST_COMPARE(xPtr[3*gId+0], <=, 1.0e-15);
	TEST_COMPARE(xPtr[3*gId+1], <=, 1.0e-15);
	TEST_COMPARE(xPtr[3*gId+2], <=, 1.0e-15);
	/*
	 * X is the center of the sphere
	 */
	Array<double> X(3); 
	/*
	 * Y = X since we are fixing the center of the sphere
	 */
	Array<double> Y(3); 

        Array<double> yPtr(3*list.get_num_owned_points());
	/*
	 * only computing bondVolume on center point 'X'
	 */

	Array<double> bondVolume(num_neigh);
        double theta, m_code;
	
	double gamma = 1.0e-6;
        scf_probe_two( neighborhoodPtr, xPtr, X,Y, bondVolume, c, horizon, yPtr, list , gId, num_neigh, theta, m_code, gamma);

	double tolerance=1.0e-12;
       
	TEST_COMPARE(theta, <=, tolerance);


	/*
	 * PROBE XY
	 */
	probe_shear(MATERIAL_EVALUATION::XY,neighborhoodPtr,X,xPtr,Y,yPtr,bondVolume,horizon,gamma,m_code);
	
}



int main
(
		int argc,
		char* argv[]
)
{


	write_table_1_header("table_1.tex");
//	write_table_2_header("table_2.tex");

	// Initialize UTF
	//int flag = unit_test_main( init_unit_test, argc, argv );

        int flag = Teuchos::UnitTestRepository::runUnitTestsFromMain(argc, argv);

	close_table_1("table_1.tex");
	return flag;
}
