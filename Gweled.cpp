//#include <Magick++.h> 
//g++ -O2 -o Magick Magick.cpp `Magick++-config --cppflags --cxxflags --ldflags --libs` -I/usr/include/GraphicsMagick/ PHAIL
//g++ -O2 -o Gweled Gweled.cpp `GraphicsMagick++-config --cppflags --cxxflags --ldflags --libs` -I/usr/include/GraphicsMagick/  :)
#include "/usr/include/GraphicsMagick/Magick++.h"
#include <iostream> 

using namespace std; 
using namespace Magick; 

#define BUFFERLENGTH 12
#define WINDOWSIZE_SMALL "256x323" 
#define SMALL_CELL_SIZE 32
#define WINDOWSIZE_MEDIUM "384x451"
#define MEDIUM_CELL_SIZE 48
#define WINDOWSIZE_LARGE "512x579"
#define LARGE_CELL_SIZE 64
#define CELL_WIDTH 8
#define DEBUGPRINT_0 0
#define DEBUGPRINT_1 1 //prints board color codes


void AnalyzeBoard( char [CELL_WIDTH][CELL_WIDTH] );
string GetWindowPID(int );
int GetGameBoardSize();

int main(int argc,char **argv) 
{ 
  InitializeMagick(*argv);

  size_t width,height;
  int gameboardsize;
  string PID;
  int halfcellpx,wholecellpx;

  std::cout<<"using Xwindowpid:"<<PID<<std::endl;
  gameboardsize=GetGameBoardSize();
  PID=GetWindowPID(gameboardsize);//get PID of window for Gweled
  switch (gameboardsize)
    {
    case 0:
      wholecellpx=SMALL_CELL_SIZE;
      break;
    case 1:
      wholecellpx=MEDIUM_CELL_SIZE;
      break;
    case 2:
      wholecellpx=LARGE_CELL_SIZE;
      break;
    default:
      wholecellpx=SMALL_CELL_SIZE;
      break;
    }
      halfcellpx=wholecellpx/2;
  // Construct the image object. Separating image construction from the 
  // the read operation ensures that a failure to read the image file 
  // doesn't render the image object useless. 

  Image image;
  while (true)
    {
      try { 
	image.read(PID);
	image.crop( Geometry(wholecellpx*CELL_WIDTH,wholecellpx*CELL_WIDTH, 0, 27) ); //the menu bar is a constant 27 pixels tall no matter the size of the game board.

	width=image.columns();
	height=image.rows();
	//    std::cout<<"orig image is "<<width<<" by "<<height<<std::endl;

	char Board[CELL_WIDTH][CELL_WIDTH];
	char Board2[64];
	int boardcounterX=0,boardcounterY=0;
	char temp='.';
	image.modifyImage();
	image.type(TrueColorType);
	for (ssize_t counterY=halfcellpx-1;counterY<CELL_WIDTH*wholecellpx-1;)
	  {
	    for (ssize_t counterX=halfcellpx-1;counterX<CELL_WIDTH*wholecellpx-1;)
	      {
		PixelPacket *imagePixels = image.getPixels(counterX,counterY,1,1);
		Color color;
		color = imagePixels[0];
		//	    std::cout<<int(color.redQuantum())<<","<<int(color.greenQuantum())<<","<<int(color.blueQuantum())<<std::endl;
	
	    switch (int(color.greenQuantum())) //key off the green value to determine jewel
	      {
	      case 185:
	      case 184:
	      case 121:
#if DEBUGPRINT_0
		std::cout<<"B";
#endif
		temp='B';
		break;
	      case 63:
#if DEBUGPRINT_0
		std::cout<<"O";
#endif
		temp='O';
		break;
	      case 153:
	      case 152:
	      case 150:
#if DEBUGPRINT_0
		std::cout<<"G";
#endif
		temp='G';
		break;
	      case 145:
#if DEBUGPRINT_0
		std::cout<<"Y";
#endif

		temp='Y';
		break;
	      case 224:
	      case 182:
#if DEBUGPRINT_0
		std::cout<<"W";
#endif

		temp='W';
		break;
	      case 0:
	      case 2:
#if DEBUGPRINT_0
		std::cout<<"P";
#endif

		temp='P';
		break;
	      case 91:
	      case 89:
	      case 86:
	      case 26:
#if DEBUGPRINT_0
		std::cout<<"R";
#endif
	
		temp='R';
		break;
	      default:
		//std::cout<<"*";
		std::cout<<int(color.redQuantum())<<","<<int(color.greenQuantum())<<","<<int(color.blueQuantum())<<std::endl;
	      }
		//    std::cout<<temp;
		Board[boardcounterX][boardcounterY]=temp;
		Board2[boardcounterX+boardcounterY*CELL_WIDTH]=temp;

	    /* small board
	      0,185,245 blue diamond
	      0,121,193 blue diamond
	      175, 63,  5 orange hexagon 
	      5  ,153,  1 green rounded square
	      145,145,  3 yellow square
	      224,224,224 white round bauble
	      182,182,182 blue diamond
	      175,  0,174 purple triangle
	      176,  2,176 purple triangle
	      213, 91,110 red rounded square
	      193, 26 ,25 red rounded square
	    */
	    //the med and large boards have some slightly diff values for some colors
	    /* med board
	       214, 89,107 red rounded square
	       5,  152,  1 green rounded square */

	    /*large board
	      0,  184,245 blue diamond
	      213, 86,104 red rounded square
	      4,  152,  1 green rounded square
	     */
		counterX+=wholecellpx;
		boardcounterX++;
	      }
	    boardcounterX=0;
	    boardcounterY++;
	    counterY+=wholecellpx;
#if DEBUGPRINT_O
	    std::cout<<std::endl;
#endif
	  }
	AnalyzeBoard( Board );
#if DEBUGPRINT_1
	  std::cout<<"maxrgb is "<<MaxRGB<<std::endl;
	  for (int y=0;y<8;y++)
	  {
	  for (int x=0;x<8;x++)
	  {
	  std::cout<<Board[x][y];
	  }
	  std::cout<<std::endl;
	  }
#endif

	  /*   
	  std::cout<<std::endl;
	  for (int xx=0;xx<64;xx++)
	  {
	  std::cout<<Board2[xx];
	  if ((xx+1)%8==0)
	  std::cout<<std::endl;
      
	  }
	*/

      } 
      catch( Exception &error_ ) 
	{ 
	  cout << "Caught exception: " << error_.what() << endl; 
	  return 1; 
	} 
    }//end inf while loop
  return 0; 
}

void AnalyzeBoard( char A[CELL_WIDTH][CELL_WIDTH] )
{
  int MovesBoard[CELL_WIDTH][CELL_WIDTH];
  int x,y;
  //init  board
  for (y=0;y<8;y++) {for ( x=0;x<8;x++) {
      MovesBoard[x][y]=0;  }     }


  //Pattern 1 horizontal
  /*1101*/
  for (x=0;x<5;x++) { for ( y=0;y<8;y++) {
      if ( A[x][y]==A[x+1][y] && A[x+3][y]==A[x][y] )
	{MovesBoard[x+2][y]++; MovesBoard[x+3][y]++;std::cout<<"1h";}
    }    }
  //pattern 7 horizontal
  /*110
    001*/
  for (x=0;x<6;x++) { for ( y=0;y<7;y++) {
      if ( A[x][y]==A[x+1][y] && A[x+2][y+1]==A[x][y] )
	{MovesBoard[x+2][y]++; MovesBoard[x+2][y+1]++;std::cout<<"7h";}
    }    }
  //Pattern 5 horizontal
  /*011
    100*/
  for (x=1;x<7;x++) { for ( y=0;y<7;y++) {
      if ( A[x][y]==A[x+1][y] && A[x-1][y+1]==A[x][y] )
	{MovesBoard[x-1][y]++; MovesBoard[x-1][y+1]++;std::cout<<"5h";}
    }    }
  //pattern 6 horizontal
  /*100
    011*/
  for (x=1;x<7;x++) { for ( y=1;y<8;y++) {
      if ( A[x][y]==A[x+1][y] && A[x-1][y-1]==A[x][y] )
	{MovesBoard[x-1][y]++; MovesBoard[x-1][y-1]++;std::cout<<"6h";}
    }    }
  //pattern 8 horizontal
  /*001
    110*/    
  for (x=0;x<6;x++) { for ( y=1;y<8;y++) {
      if ( A[x][y]==A[x+1][y] && A[x+2][y-1]==A[x][y] )
	{MovesBoard[x+2][y]++; MovesBoard[x+2][y-1]++;std::cout<<"8h";}
    }    }
  //pattern 4 horizontal
  /*101
    010*/
  for (x=0;x<6;x++) { for ( y=0;y<7;y++) {
      if ( A[x][y]==A[x+2][y] && A[x+1][y+1]==A[x][y] )
	{MovesBoard[x+1][y+1]++; MovesBoard[x+1][y]++;std::cout<<"4h";}
    }    }
  //pattern 2 horizontal
  /*1011*/
  for (x=0;x<5;x++) { for ( y=0;y<8;y++) {
      if ( A[x][y]==A[x+2][y] && A[x+3][y]==A[x][y] )
	{MovesBoard[x][y]++; MovesBoard[x+1][y]++;std::cout<<"2h";}
    }    }
  //pattern 3 horizontal
  /*010
    101*/
  for (x=0;x<6;x++) { for ( y=1;y<8;y++) {
      if ( A[x][y]==A[x+2][y] && A[x+1][y-1]==A[x][y] )
	{MovesBoard[x+1][y]++; MovesBoard[x+1][y-1]++;std::cout<<"3h";}
    }    }

  /////////----------------

  //Pattern 1 vertical
  /*1
    1
    0
    1*/
  for (x=0;x<8;x++) { for ( y=0;y<5;y++) {
      if ( A[x][y]==A[x][y+1] && A[x][y+3]==A[x][y] )
	{MovesBoard[x][y+2]++; MovesBoard[x][y+3]++;std::cout<<"1v";}
    }    }
  //pattern 7 vertical
  /* 10
     10
     01   */
  for (x=0;x<7;x++) { for ( y=0;y<6;y++) {
      if ( A[x][y]==A[x][y+1] && A[x+1][y+2]==A[x][y] )
	{MovesBoard[x][y+2]++; MovesBoard[x+1][y+2]++;std::cout<<"7v";}
    }    }
  //Pattern 5 vertical
  /*10
    01
    01*/
  for (x=0;x<7;x++) { for ( y=1;y<7;y++) {
      if ( A[x][y]==A[x][y+1] && A[x-1][y-1]==A[x][y] )
	{MovesBoard[x][y-1]++; MovesBoard[x-1][y-1]++;std::cout<<"5v";}
    }    }
  //pattern 6 vertical
  /*01
    10
    10*/
  for (x=0;x<7;x++) { for ( y=1;y<7;y++) {
      if ( A[x][y]==A[x][y+1] && A[x+1][y-1]==A[x][y] )
	{MovesBoard[x][y-1]++; MovesBoard[x+1][y-1]++;std::cout<<"6v";}
    }    }
  //pattern 8 vertical
  /*01
    01
    10*/
  for (x=1;x<8;x++) { for ( y=0;y<6;y++) {
      if ( A[x][y]==A[x][y+1] && A[x-1][y+2]==A[x][y] )
	{MovesBoard[x][y+2]++; MovesBoard[x-1][y+2]++;std::cout<<"8v";}
    }    }
  //pattern 4 vertical
  /*10
    01
    10*/
  for (x=0;x<7;x++) { for ( y=0;y<6;y++) {
      if ( A[x][y]==A[x][y+2] && A[x+1][y+1]==A[x][y] )
	{MovesBoard[x+1][y+1]++; MovesBoard[x][y+1]++;std::cout<<"4v";}
    }    }
  //pattern 2 vertical
  /*1
    0
    1
    1*/
  for (x=0;x<8;x++) { for ( y=0;y<5;y++) {
      if ( A[x][y]==A[x][y+2] && A[x][y+3]==A[x][y] )
	{MovesBoard[x][y]++; MovesBoard[x][y+1]++;std::cout<<"2v";}
    }    }
  //pattern 3 vertical
  /*01
    10
    01*/
  for (x=1;x<8;x++) { for ( y=0;y<6;y++) {
      if ( A[x][y]==A[x][y+2] && A[x-1][y+1]==A[x][y] )
	{MovesBoard[x][y+1]++; MovesBoard[x-1][y+1]++;std::cout<<"3v";}
    }    }

  std::cout<<std::endl;
  //  PrintMovesBoard();
  for (y=0;y<8;y++)
    {
      for ( x=0;x<8;x++)
	{
	  std::cout<<MovesBoard[x][y]<<" ";
	}
      std::cout<<std::endl;
    }

}

string GetWindowPID(int boardsize)
{ 
  string PID="x:";
  char temp[BUFFERLENGTH];
  FILE *in;
  string command="xwininfo -root -tree | grep gweled | grep ";
  switch (boardsize)
  {
  case 0:
    command.append(WINDOWSIZE_SMALL);
    break;
  case 1:
    command.append(WINDOWSIZE_MEDIUM);
    break;
  case 2:
    command.append(WINDOWSIZE_LARGE);
    break;
  default:
    command.append(WINDOWSIZE_SMALL);
  }
  command.append(" | awk '{ print $1 }'");
  //  if ((in = popen("xwininfo -root -tree | grep gweled | grep 256x323 | awk '{ print $1 }'","r")))
  if ((in = popen(command.c_str(),"r")))
    { 

      while (fgets(temp, sizeof(temp), in)!=NULL)
	{ 
	  PID.append(temp); //http://www.linuxquestions.org/questions/programming-9/c-c-popen-launch-process-in-specific-directory-620305/#post3053479
	}
      pclose(in);

      return PID;
    }
  else
    return "PHAIL";
}

int GetGameBoardSize()
{ 
  string info="";
  char buffer[BUFFERLENGTH];
  FILE *in;
  if ((in = popen("xwininfo -root -tree | grep gweled","r")))
    { 

      while (fgets(buffer, sizeof(buffer), in)!=NULL)
	{ 
	  info.append(buffer); //http://www.linuxquestions.org/questions/programming-9/c-c-popen-launch-process-in-specific-directory-620305/#post3053479
	}
      pclose(in);
      if (info.rfind("256x323")!=string::npos)
	{
	  std::cout<<"Small Board"<<std::endl;
	  return 0;
	}
      else if (info.rfind("384x451")!=string::npos)
	{
	  std::cout<<"Medium Board"<<std::endl;
	  return 1;
	}
      else if (info.rfind("512x579")!=string::npos)
	{
	  std::cout<<"Large Board"<<std::endl;
	  return 2;
	}
    }
  else
    return -1;
}

