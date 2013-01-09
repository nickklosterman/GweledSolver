//#include <Magick++.h> 
//g++ -O2 -o Magick Magick.cpp `Magick++-config --cppflags --cxxflags --ldflags --libs` -I/usr/include/GraphicsMagick/ PHAIL
//g++ -O2 -o Gweled Gweled.cpp `GraphicsMagick++-config --cppflags --cxxflags --ldflags --libs` -I/usr/include/GraphicsMagick/ -lX11  :)

//TODO-redo pattern matching with some similar starting point DONE 2012-07-04 its much cleaner this way and gets rid of those crazy corner cases. Now only two corner cases for each set (hor vert)
// use open cv library to create gameboard
// convert code into a class
// look into usleep and the mouse click and see if can lower time  limits
// check on if the mouse moves are being done in correct order or if things are 
// keep counter of number of moves and keep track of score.
// use enum to make board memory size smaller
// if Gweled isn't running then start it up. (check for process or window)
// -xwininfo chokes if there isn't a window named Gweled runing. I need to check for this.

//need an array copy constructor since will need separate copy of game board for subsequent calls
//-->it looks like it might be easiest to just drop using arrays and use vectors. Then need to convert all [x][y] --> [x+y*CELLWIDTH]

#include "/usr/include/GraphicsMagick/Magick++.h"
#include <iostream> 
#include <cstring> //for memset
#include <unistd.h> //for usleep

#include <X11/Xlib.h>
#include <X11/Xutil.h>  //add on -lX11 to compile command

#include <X11/keysym.h> //for fake keypress


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
#define CELL_HEIGHT 8
#define DEBUGPRINT_0 1
#define DEBUGPRINT_1 0 //prints board as color codes
#define DEBUGPRINT_2 0
#define SENTINEL 'X'
#define WINDOWCOORDSENTINEL -99

#define MENUBAROFFSET 27  // 25 on icewm on arch, 27px on icewm for ubuntu. hmmm that sux... This is the height in pixels of the menu in the OS you are running. If you run ./GweledMarkCells and open the image in Gimp, find the number of pixels from the top to the first cell (don't count the border). Using GweledMarkCells should write to GweledMediumMarked.png. In Gimp the menu won't be there. It'll be transparent pixels. Count those and there might be a 1 px border color that needs to be counted as well. 
//TODO ::: for all these different configurations, have a command line switch or json/xml file that can be ingested to set these variables. that way it doesn't require a recompile for each system. 

struct CoordPair
{  // could also achieve same results of a coord and direction with an x,y pair and a N,E,S,W direction
  int x1,x2,y1,y2;
};
struct XYPair
{
  int x,y;
};
void PerformMove(CoordPair Move);
void MoveToCoordinatesAndClick(XYPair Move);
void mouseClick(int button);
void PrintXYPair(XYPair xy);
XYPair GetGameBoardOriginCoordinates();
CoordPair TopMostMove(const int A[CELL_WIDTH][CELL_HEIGHT] );
CoordPair AnalyzeBoard( char [CELL_WIDTH][CELL_HEIGHT] );
void AnalyzeBoardv2( char [CELL_WIDTH][CELL_HEIGHT] );
template < class T > void PrintBoard( T*,int, int );// [CELL_WIDTH][CELL_HEIGHT] );
CoordPair AnalyzeBoardSingle( char [CELL_WIDTH][CELL_HEIGHT] );
CoordPair AnalyzeBoardSingleMatch( char [CELL_WIDTH][CELL_HEIGHT] );
void PrintMove(CoordPair);
string GetWindowPID( /*int*/ );
CoordPair GetWindowCoords(int );
bool GameOver();

int GetGameBoardSize();
XYPair ConvertMoveToScreenCoordinates(XYPair,CoordPair,int,int);
XYPair ConvertGameBoardToScreenCoordinates(XYPair,int,XYPair);
XYPair GetCoords(CoordPair,int);

XYPair GetGameOverWindowOriginCoordinates();
XYPair GetGweledScoresWindowOriginCoordinates();
XYPair GetWindowOriginCoordinates(string ); 

void ClickToRemoveHighScoreNotificationAndStartNewGame();
void ClickForNewGame();

int SetGameBoardCellSize();

int FakeKeyPress();
int TabFakeKeyPress();
int SpaceFakeKeyPress();
XKeyEvent createKeyEvent(Display , Window, Window , bool ,int , int);

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
		   5,  152,  1 green rounded square 
		   195, 34, 35 red rounded square 
		   0,  126,197 blue diamond*/

		/*large board
		  0,  184,245 blue diamond
		  0,  131,201 blue diamond
		  213, 86,104 red rounded square
		  198, 42, 46 red rounded square
		  4,  152,  1 green rounded square
		  185,185,185 white round bauble
		*/

int main(int argc,char **argv) 
{ 
  InitializeMagick(*argv);

  size_t width,height;
  int gameboardsize;
  string PID;
  int halfcellpx,wholecellpx;

  PID=GetWindowPID(/*gameboardsize*/);//get PID of window for Gweled
#if DEBUGPRINT_0 

  std::cout<<"-"<<PID<<"-"<<std::endl;
#endif
  /*
    XYPair origin;
    origin=GetGameBoardOriginCoordinates();
  */  gameboardsize=GetGameBoardSize();


#if DEBUGPRINT_0
  std::cout<<"using Xwindowpid:"<<PID<<std::endl;
#endif
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
      bool GameOverFlag=false;
      try { 
	image.read(PID);
	image.crop( Geometry(wholecellpx*CELL_WIDTH,wholecellpx*CELL_WIDTH, 0, MENUBAROFFSET) ); //the menu bar is a constant 27 pixels tall no matter the size of the game board. it is 25px tall in icewm on arch on the cameron laptop.

	width=image.columns();
	height=image.rows();
	//    std::cout<<"orig image is "<<width<<" by "<<height<<std::endl;

	char Board[CELL_WIDTH][CELL_HEIGHT];
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
		  case 132:
		  case 131:
		  case 126:
		  case 121:
#if DEBUGPRINT_0
		    std::cout<<"B";
#endif
		    if (int(color.redQuantum()==185))
		      temp='W';
		    else
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
		    if ( int(color.redQuantum())==int(color.greenQuantum()) && int(color.greenQuantum())==int(color.blueQuantum()) && int(color.blueQuantum())==0 )
		      {
			GameOverFlag=true; //now need to check if its the game over or high score window that is covering up the gameboard and click appropriately or just see if its neither and just being obscured
		      }
		    //if covered by the game over window then the pixels of the actual game board are interpreted as 0,0,0
		    //
#endif

		    temp='P';
		    break;
		  case 91:
		  case 89:
		  case 86:
		  case 42:
		  case 34:
		  case 26:
#if DEBUGPRINT_0
		    std::cout<<"R";
#endif
	
		    temp='R';
		    break;
		  case 218:
		    GameOverFlag=true;
		    std::cout<<"G.O.";
		    break;
		  default:
		    //std::cout<<"*";
		    std::cout<<"Unkown:"<<int(color.redQuantum())<<","<<int(color.greenQuantum())<<","<<int(color.blueQuantum())<<std::endl;
		  }

		if (!GameOverFlag)
		  {
		//    std::cout<<temp;
		Board[boardcounterX][boardcounterY]=temp;
		  }
		Board2[boardcounterX+boardcounterY*CELL_WIDTH]=temp;

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

	//	AnalyzeBoard( Board );

	CoordPair Move,PreviousMove;
	//	Move=AnalyzeBoardSingle( Board );
	//	Move=AnalyzeBoard( Board );
	//PrintMove(Move);

	AnalyzeBoardv2(Board);

	// Check to see if the game has ended 
	if (!GameOver())
	  {


	//	if (Move==PreviousMove)

	Move=AnalyzeBoardSingleMatch( Board );
	PrintMove(Move);
	//	PrintXYPair(screen_coords);
	PerformMove(Move);
	  }
	else
	  {
	    std::cout<<"G.O. need to restart"<<std::endl;
	    int code;
	    code=TabFakeKeyPress();
	    code=TabFakeKeyPress();
	    code=TabFakeKeyPress();
	    code=SpaceFakeKeyPress();
	    
	  }

#if DEBUGPRINT_0
	std::cout<<"maxrgb is "<<MaxRGB<<std::endl;
#endif
#if DEBUGPRINT_1
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
#if DEBUGPRINT_3
      std::cout<<"---------------------"<<std::endl;
#endif 
   }//end inf while loop
  return 0; 
}

template <class T>
void PrintBoard(const T* Board,int W,int H)//[CELL_WIDTH][CELL_HEIGHT])  
{
  for (int counter=0;counter<W*H;counter++)
    {
      std::cout<<Board++;
      if ( counter % W ==0 )
	std::cout<<std::endl;
    }
  /*
	for (int y=0;y<H;y++)
	  {
	    for (int x=0;x<W;x++)
	      {
		std::cout<<Board[x][y];
	      }
	    std::cout<<std::endl;
	  }
  */
}

void ClickForNewGame()
{
  /*
xwininfo: Window id: 0x2027964 (has no name)

  Absolute upper-left X:  2
  Absolute upper-left Y:  787
  Relative upper-left X:  0
  Relative upper-left Y:  0
  Width: 310
  Height: 131
  Depth: 24
  Visual: 0x21
  Visual Class: TrueColor
  Border width: 0
  Class: InputOutput
  Colormap: 0x20 (installed)
  Bit Gravity State: NorthWestGravity
  Window Gravity State: NorthWestGravity
  Backing Store State: NotUseful
  Save Under State: no
  Map State: IsViewable
  Override Redirect State: no
  Corners:  +2+787  -968+787  -968-106  +2-106
  -geometry 310x131+0+765
  * /



  206,87 to 298,118 so center is approx +46,+15 so 252,102
or press 'N' for new game.
  */

  XYPair coords;
  coords=GetGameOverWindowOriginCoordinates();


  coords.x+=3; //46;
  coords.y+=15;

  std::cout<<"New Game. Score not high enough."<<std::endl;
  // or do this
  // http://www.doctort.org/adam/nerd-notes/x11-fake-keypress-event.html


}



void ClickToRemoveHighScoreNotificationAndStartNewGame()
{
  /*
xwininfo: Window id: 0x2055ae0 "Gweled Scores"

  Absolute upper-left X:  149
  Absolute upper-left Y:  559
  Relative upper-left X:  0
  Relative upper-left Y:  0
  Width: 274
  Height: 433
  Depth: 24
  Visual: 0x21
  Visual Class: TrueColor
  Border width: 0
  Class: InputOutput
  Colormap: 0x20 (installed)
  Bit Gravity State: NorthWestGravity
  Window Gravity State: NorthWestGravity
  Backing Store State: NotUseful
  Save Under State: no
  Map State: IsViewable
  Override Redirect State: no
  Corners:  +149+559  -857+559  -857-32  +149-32
  -geometry 274x433+143-26

170,389 to 262,420

*/
  XYPair coords;
  coords=GetGweledScoresWindowOriginCoordinates();

  std::cout<<"Restarting Game. High Score Achieved"<<std::endl;

} 

bool GameOver()
{
  //Function determines if the game is over, 

  XYPair coords;
  coords=GetGameOverWindowOriginCoordinates();
  std::cout<<"G.O coords:";
  PrintXYPair(coords);
  if (coords.x!=WINDOWCOORDSENTINEL && coords.y!=WINDOWCOORDSENTINEL) //this is the
    {
      std::cout<<"duh";
      return true;
      std::cout<<"after return statment for G.O.";
    }


    coords=GetGweledScoresWindowOriginCoordinates();
    std::cout<<"H.S. coords:";
    PrintXYPair(coords);
    if (coords.x!=WINDOWCOORDSENTINEL && coords.y!=WINDOWCOORDSENTINEL)
      {
	return true;
      std::cout<<"after return statment for H.S.";
      }

  return false;

}

void PrintBoardInt(int Board[CELL_WIDTH][CELL_HEIGHT],int W,int H)
{

	for (int y=0;y<H;y++)
	  {
	    for (int x=0;x<W;x++)
	      {
		std::cout<<Board[x][y];
	      }
	    std::cout<<std::endl;
	  }

}


void PrintBoardChar(char Board[CELL_WIDTH][CELL_HEIGHT],int W,int H)
{

	for (int y=0;y<H;y++)
	  {
	    for (int x=0;x<W;x++)
	      {
		std::cout<<Board[x][y];
	      }
	    std::cout<<std::endl;
	  }

}

CoordPair TopMostMove(const int A[CELL_WIDTH][CELL_HEIGHT])  
{
  int x,y;
  bool flag=false;
  for (y=0;y<8;y++)
    { 
      for (x=0;x<8;x++) 
	{
	  if (A[x][y]>0)
	    {
	      flag=true;
	      break;//break out of x loop
	    }
	}
      if (flag)
	break;//break out of y loop
    }
  CoordPair Move;
  Move.x1=x;Move.y1=y;
  //due to way we loop (from L->R, U->D we only need to check for a move to the right or down
  if (A[x+1][y]>0)//&&B[x][y]=='H') //aux board B states if move is 'H'orizontal or 'V'ertical; I think H should override V since H is less disruptive
    {Move.x2=x+1;Move.y2=y;}
  else
    {Move.x2=x;Move.y2=y+1;}
  return Move;
}

CoordPair AnalyzeBoard( char A[CELL_WIDTH][CELL_HEIGHT] )
{//this function creates a secondary board which holds the number of moves that complete a sequence from the current position
  //It returns the topmost move. I believe that due to MovesBoard marking both the cells involved in the move that it *might* not always pick the topmost. The first encountered match might be a vertical match where a horizontal match might be a better choice.
  int MovesBoard[CELL_WIDTH][CELL_HEIGHT];
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
  CoordPair Move;
  Move=TopMostMove(MovesBoard);
  return Move;
}

CoordPair ReorderCoordPair( CoordPair a)
{
  //not needed since made sure that "Move" variable in AnalyzeBoardSingleMatch has the coordinates of the location that completes a sequence in the Move.x1 Move.y1 variables
  CoordPair b;
  if (a.x1!=a.x2 && a.x1>a.x2)
    {
      b.x1=a.x2; b.x2=a.x1;
      b.y1=a.y1; b.y2=a.y1;
    }
  if (a.y1!=a.y2 && a.y1>a.y2)
    {
      b.y1=a.y2; b.y2=a.y1;
      b.x1=a.x1; b.x2=a.x1;
    }
  return b;
}

void AnalyzeBoardv2( char A[CELL_WIDTH][CELL_HEIGHT] )
{//this function modifies a second matrix which holds the number of moves that can be made using the current coordinate.
  //the first version marked both cells that were involved in the move. This version only marks the cell that would copmlete the sequence when the appropriate jewel was moved *into* it.
  int MovesBoard[CELL_WIDTH][CELL_HEIGHT];
  for (int i=0;i<CELL_WIDTH;i++)
    {
      for (int j=0;j<CELL_HEIGHT;j++)
	{
      MovesBoard[i][j]=0;
	}
    }

  int x,y;
  //loop over entire board returning when first match found 
  for (y=0;y<8;y++) {
    for ( x=0;x<8;x++) {
      if (x<5)
	{
	  //Pattern 1 horizontal
	  /*1101*/      
	  if ( A[x][y]==A[x+1][y] && A[x+3][y]==A[x][y] )
	    {MovesBoard[x+2][y]++; }//Move.x2=x+3; Move.y2=y;std::cout<<"1h"; return Move;}//	    {MovesBoard[x+2][y]++; MovesBoard[x+3][y]++;std::cout<<"1h";}

	  //pattern 2 horizontal
	  /*1011*/
	  if ( A[x][y]==A[x+2][y] && A[x+3][y]==A[x][y] )
	    {MovesBoard[x][y]++;}// Move.x2=x+1;Move.y2=y;std::cout<<"2h";return Move;}//	    {MovesBoard[x][y]++; MovesBoard[x+1][y]++;std::cout<<"2h";}
    	}    
      if (x<6&&y<7)
	{
	  //pattern 4 horizontal
	  /*101
	    010*/
	  if ( A[x][y]==A[x+2][y] && A[x+1][y+1]==A[x][y] )
	    {MovesBoard[x+1][y]++;}// Move.x2=x+1;Move.y2=y+1;std::cout<<"4h";return Move;}// need to reorder coords so x1,y1 is coord that completes 3some

	  //pattern 7 horizontal
	  /*110
	    001*/
	  if ( A[x][y]==A[x+1][y] && A[x+2][y+1]==A[x][y] )
	    {MovesBoard[x+2][y]++;}//Move.x2=x+2;Move.y2=y+1;std::cout<<"7h";return Move;}//	    {MovesBoard[x+2][y]++; MovesBoard[x+2][y+1]++;std::cout<<"7h";}
	}

      if (x<6 && y<7 )
	{
	  //pattern 3 horizontal
	  /*010
	    101*/
	  if ( A[x+1][y]==A[x][y+1] && A[x+2][y+1]==A[x+1][y] )
	    {MovesBoard[x+1][y+1]++;}// Move.x2=x+1;Move.y2=y;std::cout<<"3h";return Move;}//		{MovesBoard[x+1][y]++; MovesBoard[x+1][y-1]++;std::cout<<"3h";}

	  //pattern 8 horizontal
	  /*001
	    110*/    
	  if ( A[x+2][y]==A[x][y+1] && A[x+2][y]==A[x+1][y+1] )
	    {MovesBoard[x+2][y+1]++;}// Move.x2=x+2;Move.y2=y;std::cout<<"8h";return Move;}//		{MovesBoard[x+2][y]++; MovesBoard[x+2][y-1]++;std::cout<<"8h";}

	  //Pattern 5 horizontal
	  /*011
	    100*/
	  if ( A[x][y+1]==A[x+1][y] && A[x][y+1]==A[x+2][y] )
	    {MovesBoard[x][y]++;}//Move.x2=x;Move.y2=y+1;std::cout<<"5h";return Move;}//		{MovesBoard[x-1][y]++; MovesBoard[x-1][y+1]++;std::cout<<"5h";}

	  //pattern 6 horizontal
	  /*100
	    011*/

	      if ( A[x][y]==A[x+1][y+1] && A[x+2][y+1]==A[x][y] )
		{MovesBoard[x][y+1]++;}//Move.x2=x;Move.y2=y;std::cout<<"6h";return Move;}//		{MovesBoard[x-1][y]++; MovesBoard[x-1][y-1]++;std::cout<<"6h";}
	}
      //End Horizontal cases
      //Start Vertical cases
      if (y<5)
	{
	  //Pattern 1 vertical
	  /*1
	    1
	    0
	    1*/
	  if ( A[x][y]==A[x][y+1] && A[x][y+3]==A[x][y] )
	    {MovesBoard[x][y+2]++;}//Move.x2=x;Move.y2=y+3;std::cout<<"1v";return Move;}//}	{MovesBoard[x][y+2]++; MovesBoard[x][y+3]++;std::cout<<"1v";}
	  //pattern 2 vertical
	  /*1
	    0
	    1
	    1*/
	  if ( A[x][y]==A[x][y+2] && A[x][y+3]==A[x][y] )
	    {MovesBoard[x][y+1]++;}// Move.x2=x;Move.y2=y;std::cout<<"2v";return Move;}//}	{MovesBoard[x][y]++; MovesBoard[x][y+1]++;std::cout<<"2v";}

	}
      if(x<7&&y<6)
	{
	  //pattern 4 vertical
	  /*10
	    01
	    10*/
	  if ( A[x][y]==A[x][y+2] && A[x+1][y+1]==A[x][y] )
	    {MovesBoard[x][y+1]++;}// Move.x2=x+1;Move.y2=y+1;std::cout<<"4v";return Move;}//	    {MovesBoard[x+1][y+1]++; MovesBoard[x][y+1]++;}

	  //pattern 7 vertical
	  /* 10
	     10
	     01   */
	  if ( A[x][y]==A[x][y+1] && A[x+1][y+2]==A[x][y] )
	    {MovesBoard[x][y+2]++;}// Move.x2=x+1;Move.y2=y+2;std::cout<<"7v";return Move;}//}	    {MovesBoard[x][y+2]++; MovesBoard[x+1][y+2]++;std::cout<<"7v";}
	}
      if (x<7 && y<6)
	{
	  if ( A[x][y]==A[x+1][y+1] && A[x+1][y+2]==A[x][y] )
	    {MovesBoard[x+1][y]++;}// Move.x2=x;Move.y2=y;std::cout<<"5v";return Move;}//; }		{MovesBoard[x][y-1]++; MovesBoard[x-1][y-1]++;std::cout<<"5v";}

	  if ( A[x+1][y]==A[x][y+1] && A[x+1][y]==A[x][y+2] )
	    {MovesBoard[x][y]++;}// Move.x2=x+1;Move.y2=y;std::cout<<"6v";return Move;}//}	    {MovesBoard[x][y-1]++; MovesBoard[x+1][y-1]++;std::cout<<"6v";}
	  if ( A[x+1][y]==A[x+1][y+1] && A[x][y+2]==A[x+1][y] )
	    {MovesBoard[x+1][y+2]++;}// Move.x2=x;Move.y2=y+2;std::cout<<"8v";return Move;}//}	{MovesBoard[x][y+2]++; MovesBoard[x-1][y+2]++;std::cout<<"8v";}

	  if ( A[x+1][y]==A[x][y+1] && A[x+1][y]==A[x+1][y+2] )
	    {MovesBoard[x+1][y+1]++;}// Move.x2=x;Move.y2=y+1;std::cout<<"3v";return Move;}//}	{MovesBoard[x][y+1]++; MovesBoard[x-1][y+1]++;std::cout<<"3v";}
	} 
    }
  }

  PrintBoardInt(MovesBoard,CELL_WIDTH,CELL_HEIGHT);

}

CoordPair AnalyzeBoardSingleMatch( char A[CELL_WIDTH][CELL_HEIGHT] )
{//function takes a gameboard and returns a coordpair of first move found 

  CoordPair Move;
  Move.x1=-1;Move.y1=-1;  Move.x2=-1;Move.y2=-1;
  int x,y;
  //would it be better to set a flag and use breaks and have only one return statement?

  //loop over entire board returning when first match found 
  for (y=0;y<8;y++) {
    for ( x=0;x<8;x++) {
      if (x<5)
	{
	  //Pattern 1 horizontal
	  /*1101*/      
	  if ( A[x][y]==A[x+1][y] && A[x+3][y]==A[x][y] )
	    {Move.x1=x+2; Move.y1=y; Move.x2=x+3; Move.y2=y;std::cout<<"1h"; return Move;}//	    {MovesBoard[x+2][y]++; MovesBoard[x+3][y]++;std::cout<<"1h";}

	  //pattern 2 horizontal
	  /*1011*/
	  if ( A[x][y]==A[x+2][y] && A[x+3][y]==A[x][y] )
	    {Move.x1=x;Move.y1=y; Move.x2=x+1;Move.y2=y;std::cout<<"2h";return Move;}//	    {MovesBoard[x][y]++; MovesBoard[x+1][y]++;std::cout<<"2h";}
    	}    
      if (x<6&&y<7)
	{
	  //pattern 4 horizontal
	  /*101
	    010*/
	  if ( A[x][y]==A[x+2][y] && A[x+1][y+1]==A[x][y] )
	    {Move.x1=x+1;Move.y1=y; Move.x2=x+1;Move.y2=y+1;std::cout<<"4h";return Move;}// need to reorder coords so x1,y1 is coord that completes 3some
	  //{Move.x1=x+1;Move.y1=y+1; Move.x2=x+1;Move.y2=y;std::cout<<"4h";return Move;}//	    {MovesBoard[x+1][y+1]++; MovesBoard[x+1][y]++;std::cout<<"4h";}

	  //pattern 7 horizontal
	  /*110
	    001*/
	  if ( A[x][y]==A[x+1][y] && A[x+2][y+1]==A[x][y] )
	    {Move.x1=x+2;Move.y1=y;Move.x2=x+2;Move.y2=y+1;std::cout<<"7h";return Move;}//	    {MovesBoard[x+2][y]++; MovesBoard[x+2][y+1]++;std::cout<<"7h";}
	}

      //old limits      if (x<6 && y>0 )
      if (x<6 && y<7 )
	{
	  //pattern 3 horizontal
	  /*010
	    101*/

	  //	  if ( A[x][y]==A[x+2][y] && A[x+1][y-1]==A[x][y] ) 	    {Move.x1=x+1;Move.y1=y; Move.x2=x+1;Move.y2=y-1;std::cout<<"3h";return Move;}//		{MovesBoard[x+1][y]++; MovesBoard[x+1][y-1]++;std::cout<<"3h";}
	  //new 3h start at 0,0 of pattern instead of 1,0
	  if ( A[x+1][y]==A[x][y+1] && A[x+2][y+1]==A[x+1][y] )
	    {Move.x1=x+1;Move.y1=y+1; Move.x2=x+1;Move.y2=y;std::cout<<"3h";return Move;}//		{MovesBoard[x+1][y]++; MovesBoard[x+1][y-1]++;std::cout<<"3h";}
	  //	    {Move.x1=x+1;Move.y1=y; Move.x2=x+1;Move.y2=y+1;std::cout<<"3h";return Move;}//		{MovesBoard[x+1][y]++; MovesBoard[x+1][y-1]++;std::cout<<"3h";}

	  //pattern 8 horizontal
	  /*001
	    110*/    
	  //  if ( A[x][y]==A[x+1][y] && A[x+2][y-1]==A[x][y] )	    {Move.x1=x+2;Move.y1=y; Move.x2=x+2;Move.y2=y-1;std::cout<<"8h";return Move;}//		{MovesBoard[x+2][y]++; MovesBoard[x+2][y-1]++;std::cout<<"8h";}
	  //new 8h start at 0,0 of pattern instead of 1,0
	  if ( A[x+2][y]==A[x][y+1] && A[x+2][y]==A[x+1][y+1] )
	    {Move.x1=x+2;Move.y1=y+1; Move.x2=x+2;Move.y2=y;std::cout<<"8h";return Move;}//		{MovesBoard[x+2][y]++; MovesBoard[x+2][y-1]++;std::cout<<"8h";}
	  //	    {Move.x1=x+2;Move.y1=y; Move.x2=x+2;Move.y2=y+1;std::cout<<"8h";return Move;}//		{MovesBoard[x+2][y]++; MovesBoard[x+2][y-1]++;std::cout<<"8h";}

  //new 5h
  //Pattern 5 horizontal
  /*011
    100*/
	      if ( A[x][y+1]==A[x+1][y] && A[x][y+1]==A[x+2][y] )
		{Move.x1=x;Move.y1=y;Move.x2=x;Move.y2=y+1;std::cout<<"5h";return Move;}//		{MovesBoard[x-1][y]++; MovesBoard[x-1][y+1]++;std::cout<<"5h";}
	      //		{Move.x1=x;Move.y1=y;Move.x2=x;Move.y2=y+1;std::cout<<"5h";return Move;}//		{MovesBoard[x-1][y]++; MovesBoard[x-1][y+1]++;std::cout<<"5h";}

	      //new 6h
  //pattern 6 horizontal
  /*100
    011*/

	      if ( A[x][y]==A[x+1][y+1] && A[x+2][y+1]==A[x][y] )
		{Move.x1=x;Move.y1=y+1;Move.x2=x;Move.y2=y;std::cout<<"6h";return Move;}//		{MovesBoard[x-1][y]++; MovesBoard[x-1][y-1]++;std::cout<<"6h";}
	      //		{Move.x1=x;Move.y1=y;Move.x2=x;Move.y2=y+1;std::cout<<"6h";return Move;}//		{MovesBoard[x-1][y]++; MovesBoard[x-1][y-1]++;std::cout<<"6h";}
	}
	
      /*
      if (x>0 && x<7)
	{
	  if (y<7)
	    {
	      //Pattern 5 horizontal
	      / *011
		100 * /
	      //	      if ( A[x][y]==A[x+1][y] && A[x-1][y+1]==A[x][y] )		{Move.x1=x-1;Move.y1=y;Move.x2=x-1;Move.y2=y+1;std::cout<<"5h";return Move;}//		{MovesBoard[x-1][y]++; MovesBoard[x-1][y+1]++;std::cout<<"5h";}
	    
	    }
	  if (y>0)
	    {
	      //pattern 6 horizontal
	      / * 100
		011* /
	      //	      if ( A[x][y]==A[x+1][y] && A[x-1][y-1]==A[x][y] )		{Move.x1=x-1;Move.y1=y;Move.x2=x-1;Move.y2=y-1;std::cout<<"6h";return Move;}//		{MovesBoard[x-1][y]++; MovesBoard[x-1][y-1]++;std::cout<<"6h";}

	    }
	}
*/
      //End Horizontal cases
      //Start Vertical cases
      if (y<5)
	{
	  //Pattern 1 vertical
	  /*1
	    1
	    0
	    1*/
	  if ( A[x][y]==A[x][y+1] && A[x][y+3]==A[x][y] )
	    {Move.x1=x;Move.y1=y+2;Move.x2=x;Move.y2=y+3;std::cout<<"1v";return Move;}//}	{MovesBoard[x][y+2]++; MovesBoard[x][y+3]++;std::cout<<"1v";}
	  //pattern 2 vertical
	  /*1
	    0
	    1
	    1*/
	  if ( A[x][y]==A[x][y+2] && A[x][y+3]==A[x][y] )
	    {Move.x1=x;Move.y1=y+1; Move.x2=x;Move.y2=y;std::cout<<"2v";return Move;}//}	{MovesBoard[x][y]++; MovesBoard[x][y+1]++;std::cout<<"2v";}
	  //	    {Move.x1=x;Move.y1=y; Move.x2=x;Move.y2=y+1;std::cout<<"2v";return Move;}//}	{MovesBoard[x][y]++; MovesBoard[x][y+1]++;std::cout<<"2v";}
	}
      if(x<7&&y<6)
	{
	  //pattern 4 vertical
	  /*10
	    01
	    10*/
	  if ( A[x][y]==A[x][y+2] && A[x+1][y+1]==A[x][y] )
	    {Move.x1=x;Move.y1=y+1; Move.x2=x+1;Move.y2=y+1;std::cout<<"4v";return Move;}//	    {MovesBoard[x+1][y+1]++; MovesBoard[x][y+1]++;}
	  //	    {Move.x1=x+1;Move.y1=y+1; Move.x2=x;Move.y2=y+1;std::cout<<"4v";return Move;}//	    {MovesBoard[x+1][y+1]++; MovesBoard[x][y+1]++;}
	  //pattern 7 vertical
	  /* 10
	     10
	     01   */
	  if ( A[x][y]==A[x][y+1] && A[x+1][y+2]==A[x][y] )
	    {Move.x1=x;Move.y1=y+2; Move.x2=x+1;Move.y2=y+2;std::cout<<"7v";return Move;}//}	    {MovesBoard[x][y+2]++; MovesBoard[x+1][y+2]++;std::cout<<"7v";}
	}
      if (x<7 && y<6)
	{
	  if ( A[x][y]==A[x+1][y+1] && A[x+1][y+2]==A[x][y] )
	    {Move.x1=x+1;Move.y1=y; Move.x2=x;Move.y2=y;std::cout<<"5v";return Move;}//; }		{MovesBoard[x][y-1]++; MovesBoard[x-1][y-1]++;std::cout<<"5v";}
	  //	    {Move.x1=x;Move.y1=y; Move.x2=x+1;Move.y2=y;std::cout<<"5v";return Move;}//; }		{MovesBoard[x][y-1]++; MovesBoard[x-1][y-1]++;std::cout<<"5v";}
	  if ( A[x+1][y]==A[x][y+1] && A[x+1][y]==A[x][y+2] )
	    {Move.x1=x;Move.y1=y; Move.x2=x+1;Move.y2=y;std::cout<<"6v";return Move;}//}	    {MovesBoard[x][y-1]++; MovesBoard[x+1][y-1]++;std::cout<<"6v";}
	  if ( A[x+1][y]==A[x+1][y+1] && A[x][y+2]==A[x+1][y] )
	    {Move.x1=x+1;Move.y1=y+2; Move.x2=x;Move.y2=y+2;std::cout<<"8v";return Move;}//}	{MovesBoard[x][y+2]++; MovesBoard[x-1][y+2]++;std::cout<<"8v";}
	  //	    {Move.x1=x;Move.y1=y+2; Move.x2=x+1;Move.y2=y+2;std::cout<<"8v";return Move;}//}	{MovesBoard[x][y+2]++; MovesBoard[x-1][y+2]++;std::cout<<"8v";}
	  if ( A[x+1][y]==A[x][y+1] && A[x+1][y]==A[x+1][y+2] )
	    {Move.x1=x+1;Move.y1=y+1; Move.x2=x;Move.y2=y+1;std::cout<<"3v";return Move;}//}	{MovesBoard[x][y+1]++; MovesBoard[x-1][y+1]++;std::cout<<"3v";}
	  //	    {Move.x1=x;Move.y1=y+1; Move.x2=x+1;Move.y2=y+1;std::cout<<"3v";return Move;}//}	{MovesBoard[x][y+1]++; MovesBoard[x-1][y+1]++;std::cout<<"3v";}


	} 
      /*
      if (y>0 && x>0)//(x<7 && y>0 )//&& y<7)
	    {
	      //Pattern 5 vertical
	      / *10
		01
		01* /
	      //redo	      if ( A[x][y]==A[x][y+1] && A[x-1][y-1]==A[x][y] )		{Move.x1=x;Move.y1=y-1; Move.x2=x-1;Move.y2=y-1;std::cout<<"5v";return Move;}//; }		{MovesBoard[x][y-1]++; MovesBoard[x-1][y-1]++;std::cout<<"5v";}


	    }

	
      if (x<7 && y<7 && y>0)
	{
	  //pattern 6 vertical
	  / *01
	    10
	    10* /
	  //redo	  if ( A[x][y]==A[x][y+1] && A[x+1][y-1]==A[x][y] )	    {Move.x1=x;Move.y1=y-1; Move.x2=x+1;Move.y2=y-1;std::cout<<"6v";return Move;}//}	    {MovesBoard[x][y-1]++; MovesBoard[x+1][y-1]++;std::cout<<"6v";}


	}

      if (x>0&&y<6)
	{
	  //pattern 8 vertical
	  / *01
	    01
	    10* /
	  //redo	  if ( A[x][y]==A[x][y+1] && A[x-1][y+2]==A[x][y] )	    {Move.x1=x;Move.y1=y+2; Move.x2=x-1;Move.y2=y+2;std::cout<<"8v";return Move;}//}	{MovesBoard[x][y+2]++; MovesBoard[x-1][y+2]++;std::cout<<"8v";}

	  //pattern 3 vertical
	  / *01
	    10
	    01* /
	  //redo	  if ( A[x][y]==A[x][y+2] && A[x-1][y+1]==A[x][y] )	    {Move.x1=x;Move.y1=y+1; Move.x2=x-1;Move.y2=y+1;std::cout<<"3v";return Move;}//}	{MovesBoard[x][y+1]++; MovesBoard[x-1][y+1]++;std::cout<<"3v";}


	}*/
    }
  }
  return Move;
}

CoordPair AnalyzeBoardSingle( char A[CELL_WIDTH][CELL_HEIGHT] )
{
  CoordPair Move;
  Move.x1=-1;Move.y1=-1;  Move.x2=-1;Move.y2=-1;
  int x,y;
  /*
    Aggg The problem with this algorithm is that it stops after it finds a match. That match
    might not be the one closest to the top of the screen.
  */

  //Pattern 1 horizontal
  /*1101*/
  for (x=0;x<5;x++) { for ( y=0;y<8;y++) {
      if ( A[x][y]==A[x+1][y] && A[x+3][y]==A[x][y] )
	{Move.x1=x+2; Move.y1=y; Move.x2=x+3; Move.y2=y; return Move;
	}
    }    }
  //pattern 7 horizontal
  /*110
    001*/
  for (x=0;x<6;x++) { for ( y=0;y<7;y++) {
      if ( A[x][y]==A[x+1][y] && A[x+2][y+1]==A[x][y] )
	{Move.x1=x+2;Move.y1=y; Move.x2=x+2;Move.y2=y+1; return Move;}
    }    }
  //Pattern 5 horizontal
  /*011
    100*/
  for (x=1;x<7;x++) { for ( y=0;y<7;y++) {
      if ( A[x][y]==A[x+1][y] && A[x-1][y+1]==A[x][y] )
	{Move.x1=x-1;Move.y1=y;Move.x2=x-1;Move.y2=y+1;return Move;}
    }    }
  //pattern 6 horizontal
  /*100
    011*/
  for (x=1;x<7;x++) { for ( y=1;y<8;y++) {
      if ( A[x][y]==A[x+1][y] && A[x-1][y-1]==A[x][y] )
	{Move.x1=x-1;Move.y1=y;Move.x2=x-1;Move.y2=y-1;return Move;}//std::cout<<"6h";}
    }    }
  //pattern 8 horizontal
  /*001
    110*/    
  for (x=0;x<6;x++) { for ( y=1;y<8;y++) {
      if ( A[x][y]==A[x+1][y] && A[x+2][y-1]==A[x][y] )
	{Move.x1=x+2;Move.y1=y; Move.x2=x+2;Move.y2=y-1;return Move;}//std::cout<<"8h";}
    }    }
  //pattern 4 horizontal
  /*101
    010*/
  for (x=0;x<6;x++) { for ( y=0;y<7;y++) {
      if ( A[x][y]==A[x+2][y] && A[x+1][y+1]==A[x][y] )
	{Move.x1=x+1;Move.y1=y+1; Move.x2=x+1;Move.y2=y;return Move;}//std::cout<<"4h";}
    }    }
  //pattern 2 horizontal
  /*1011*/
  for (x=0;x<5;x++) { for ( y=0;y<8;y++) {
      if ( A[x][y]==A[x+2][y] && A[x+3][y]==A[x][y] )
	{Move.x1=x;Move.y1=y; Move.x2=x+1;Move.y2=y;return Move;}//std::cout<<"2h";}
    }    }
  //pattern 3 horizontal
  /*010
    101*/
  for (x=0;x<6;x++) { for ( y=1;y<8;y++) {
      if ( A[x][y]==A[x+2][y] && A[x+1][y-1]==A[x][y] )
	{Move.x1=x+1;Move.y1=y; Move.x2=x+1;Move.y2=y-1;return Move;}//;std::cout<<"3h";}
    }    }

  /////////----------------

  //Pattern 1 vertical
  /*1
    1
    0
    1*/
  for (x=0;x<8;x++) { for ( y=0;y<5;y++) {
      if ( A[x][y]==A[x][y+1] && A[x][y+3]==A[x][y] )
	{Move.x1=x;Move.y1=y+2;Move.x2=x;Move.y2=y+3;return Move;}//std::cout<<"1v";}
    }    }
  //pattern 7 vertical
  /* 10
     10
     01   */
  for (x=0;x<7;x++) { for ( y=0;y<6;y++) {
      if ( A[x][y]==A[x][y+1] && A[x+1][y+2]==A[x][y] )
	{Move.x1=x;Move.y1=y+2; Move.x2=x+1;Move.y2=y+2;return Move;}//std::cout<<"7v";}
    }    }
  //Pattern 5 vertical
  /*10
    01
    01*/
  for (x=0;x<7;x++) { for ( y=1;y<7;y++) {
      if ( A[x][y]==A[x][y+1] && A[x-1][y-1]==A[x][y] )
	{Move.x1=x;Move.y1=y-1; Move.x2=x-1;Move.y2=y-1;return Move;}//;std::cout<<"5v";}
    }    }
  //pattern 6 vertical
  /*01
    10
    10*/
  for (x=0;x<7;x++) { for ( y=1;y<7;y++) {
      if ( A[x][y]==A[x][y+1] && A[x+1][y-1]==A[x][y] )
	{Move.x1=x;Move.y1=y-1; Move.x2=x+1;Move.y2=y-1;return Move;}//std::cout<<"6v";}
    }    }
  //pattern 8 vertical
  /*01
    01
    10*/
  for (x=1;x<8;x++) { for ( y=0;y<6;y++) {
      if ( A[x][y]==A[x][y+1] && A[x-1][y+2]==A[x][y] )
	{Move.x1=x;Move.y1=y+2; Move.x2=x-1;Move.y2=y+2;return Move;}//std::cout<<"8v";}
    }    }
  //pattern 4 vertical
  /*10
    01
    10*/
  for (x=0;x<7;x++) { for ( y=0;y<6;y++) {
      if ( A[x][y]==A[x][y+2] && A[x+1][y+1]==A[x][y] )
	{Move.x1=x+1;Move.y1=y+1; Move.x2=x;Move.y2=y+1;return Move;}//std::cout<<"4v";}
    }    }
  //pattern 2 vertical
  /*1
    0
    1
    1*/
  for (x=0;x<8;x++) { for ( y=0;y<5;y++) {
      if ( A[x][y]==A[x][y+2] && A[x][y+3]==A[x][y] )
	{Move.x1=x;Move.y1=y; Move.x2=x;Move.y2=y+1;return Move;}//std::cout<<"2v";}
    }    }
  //pattern 3 vertical
  /*01
    10
    01*/
  for (x=1;x<8;x++) { for ( y=0;y<6;y++) {
      if ( A[x][y]==A[x][y+2] && A[x-1][y+1]==A[x][y] )
	{Move.x1=x;Move.y1=y+1; Move.x2=x-1;Move.y2=y+1;return Move;}//std::cout<<"3v";}
    }    }


  return Move;
}

void PrintMove( CoordPair Move)
{
  std::cout<<"Move from ["<<Move.x1+1<<"]["<<Move.y1+1<<"] to ["<<Move.x2+1<<"]["<<Move.y2+1<<"]."<<std::endl;
}

string GetWindowPID(/*int boardsize*/)
{ 
  string PID="x:";
  char temp[BUFFERLENGTH];
  FILE *in;
  //  string command="xwininfo -name Gweled |  grep xwininfo | awk '{print $4}'";//"xwininfo -root -tree | grep gweled | grep ";
  string command="xwininfo -name Gweled |  grep Gweled | awk '{print $4}'";
  /*  switch (boardsize)
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
      command.append(" | awk '{ print $1 }'");*/
  //  if ((in = popen("xwininfo -root -tree | grep gweled | grep 256x323 | awk '{ print $1 }'","r")))


  if ((in = popen(command.c_str(),"r")))
    { 
      while (fgets(temp, sizeof(temp), in)!=NULL)
	{ 
	  //printf("%c",temp);//I think printing it out screws up the buffer so you can't print out and capture together
	  PID.append(temp); //http://www.linuxquestions.org/questions/programming-9/c-c-popen-launch-process-in-specific-directory-620305/#post3053479
	}
      pclose(in);
#if DEBUGPRINT_0 
      std::cout<<PID<<std::endl;
#endif
      return PID.substr(0,PID.length()-1);
    }
  else
    return "PHAIL";
}

XYPair ConvertMoveToScreenCoordinates(XYPair origin, CoordPair move,int whole_cell_size,int set)
{
  XYPair screen_coords;
  XYPair singlemovecoords;
#if DEBUGPRINT_2
  PrintMove(move);
#endif

  singlemovecoords=GetCoords(move,set);

#if DEBUGPRINT_2
  PrintXYPair(singlemovecoords);
#endif

  screen_coords=ConvertGameBoardToScreenCoordinates(origin,whole_cell_size,singlemovecoords);
  return screen_coords;
}



void PerformMove(char A[CELL_WIDTH][CELL_HEIGHT] ,CoordPair move)
{//this function moves pieces on the game board showing a game board with the pieces moved
  char temp=A[move.x1][move.y1];
  A[move.x1][move.y1]=A[move.x2][move.y2];
  A[move.x2][move.y2]=temp;
}
CoordPair FindStartStopOfSequence(char A[CELL_WIDTH][CELL_HEIGHT] ,CoordPair move)
{//given a gameboard with a move performed on it, this function finds the start position and end position of a match
  //the x1,y1 always holds the coord of the position that completes the match.
  CoordPair Seq;
      Seq.y1=move.y1;//initialize
      Seq.y2=move.y1;
      Seq.x1=move.x1;
      Seq.x2=move.x1;


  if (move.x1==move.x2) //vertical match
    {
      int x=move.x1;
      bool flag=true; 
      int counter=1;

      while (flag==true)
	{//find bottom most extent of sequence
	  if ( move.y1+counter<CELL_HEIGHT && A[x][move.y1+counter]==A[x][move.y1])
		{
		  Seq.y1=move.y1+counter;
		  counter++;
		}
	  else
	    {flag=false;}
	}
      flag=true;counter=1;
      x=move.x1;
      while (flag==true)
	{//find topmost extent of sequence
	  if ( move.y1+counter>=0 && A[x][move.y1+counter]==A[x][move.y1])
		{
		  Seq.y1=move.y1-counter;
		  counter++;
		}
	  else
	    {flag=false;}
	}
    }
  else //horizontal match
    {
      int y=move.y1;
      bool flag=true; 
      int counter=1;

      while (flag==true)
	{ //find right most extent of sequence
	  if ( move.x1+counter<CELL_WIDTH && A[move.x1+counter][y]==A[move.x1][y])
		{
		  Seq.x1=move.x1+counter;
		  counter++;
		}
	  else
	    {flag=false;}
	}
      flag=true;counter=1;
      y=move.y1;
      while (flag==true)
	{ // find left most extent of sequence
	  if ( move.x1+counter>=0 && A[move.x1+counter][y]==A[move.x1][y])
		{
		  Seq.x1=move.x1-counter;
		  counter++;
		}
	  else
	    {flag=false;}
	}
    }
  return Seq;

}

void MovePiecesOnBoard(char A[CELL_WIDTH][CELL_HEIGHT] ,CoordPair move)
{ //This function removes the matched sequence, moves the game pieces down, and places sentinels in the positions where new game pieces (which are unknown at the time) will fall
  // move holds the starting and ending position of the sequence to be removed
  int length=0;
  int x = 0;
  if (move.x1==move.x2) //vertical match
    {
      length=move.y1-move.y2+1;
      x=move.x1; //could be move.x2 as they are both the same;
      for (int y=move.y1-1;y==0;y--)
	{
	  A[x][y+length]=A[x][y]; //move pieces down
	}
      while (length>0)
	{
	  A[x][length]=SENTINEL;
	  length--;
	}
    }
  else //horizontal match
    {
      length=1; //pieces only move down 1 slot
      for (x=move.x1-1;x==0;x--)
	{
	  for (int y=move.y1-1;y==0;y--)
	    {
	      A[x][y+length]=A[x][y]; //move pieces down
	      A[x][0]=SENTINEL;
	    }
	}
      while (length>0)
	{
	  A[x][length]=SENTINEL;
	  length--;
	}
    }
}

XYPair GetGameBoardOriginCoordinates()
{
  XYPair coords; coords.x=-1;coords.y=-1;
  char buffer[BUFFERLENGTH];
  string info;
  FILE *in;
  if ((in = popen("xwininfo -name Gweled | grep Corners | awk '{print $2}' ","r")))
    { 
      while (fgets(buffer, sizeof(buffer), in)!=NULL)
	{ 
	  info.append(buffer); //http://www.linuxquestions.org/questions/programming-9/c-c-popen-launch-process-in-specific-directory-620305/#post3053479
	}
      pclose(in);
      // Corners:  +2+787  -968+787  -968-106  +2-106
      // info should hold : +2+787
      int plus_pos=info.rfind("+");
      if (plus_pos!=int(string::npos)) //reverse find , wo the int() casting we are comparing unsigned and signed ints
	{
	  //	  std::cout<<info.substr(1,plus_pos-1)<<std::endl<<info.substr(plus_pos+1,info.length())<<std::endl;
	  string temp=info.substr(1,plus_pos-1);
	  coords.x=atoi(temp.c_str());//one off from beginning to (info.rfind("+");
	  temp=info.substr(plus_pos+1,info.length());
	  coords.y=atoi(temp.c_str());//info.substr(plus_pos+1,info.length()));
#if DEBUGPRINT_2
	  printf("Board Coordinates %d,%d",coords.x,coords.y); //need to add in offset
#endif
	  int xoffset=0;
	  int yoffset=27;
	  coords.x+=xoffset;
	  coords.y+=yoffset;
	}
    }
  return coords;
 }

XYPair GetGameOverWindowOriginCoordinates()
{
  string command="xwininfo -root -tree | grep Gweled | grep 'has no name' | awk '{print $8}' ";
  XYPair coords=GetWindowOriginCoordinates(command);
  coords.x=-99;  coords.y=-99;
  return coords;
}


XYPair GetGweledScoresWindowOriginCoordinates()
{
  string command="xwininfo -name 'Gweled Scores'  | grep Corners | awk '{print $2}' ";
  XYPair coords=GetWindowOriginCoordinates(command);
  return coords;
}

XYPair GetWindowOriginCoordinates(string command)
{
  XYPair coords; coords.x=-1;coords.y=-1;
  char buffer[BUFFERLENGTH];
  string info;
  FILE *in;
  
  if ((in = popen(command.c_str(),"r")))
    { 
      while (fgets(buffer, sizeof(buffer), in)!=NULL)
	{ 
	  info.append(buffer); //http://www.linuxquestions.org/questions/programming-9/c-c-popen-launch-process-in-specific-directory-620305/#post3053479
	}
      pclose(in);

      int plus_pos=info.rfind("+");
      if (plus_pos!=int(string::npos)) //reverse find , wo the int() casting we are comparing unsigned and signed ints
	{
	  //	  std::cout<<info.substr(1,plus_pos-1)<<std::endl<<info.substr(plus_pos+1,info.length())<<std::endl;
	  string temp=info.substr(1,plus_pos-1);
	  coords.x=atoi(temp.c_str());//one off from beginning to (info.rfind("+");
	  temp=info.substr(plus_pos+1,info.length());
	  coords.y=atoi(temp.c_str());//info.substr(plus_pos+1,info.length()));
#if DEBUGPRINT_2
	  printf("Board Coordinates %d,%d",coords.x,coords.y); //need to add in offset
#endif
	  int xoffset=0;
	  int yoffset=27;
	  coords.x+=xoffset;
	  coords.y+=yoffset;
	}
      else 
	{

	  coords.x=WINDOWCOORDSENTINEL;
	  coords.y=WINDOWCOORDSENTINEL;
	}
    }
  return coords;
 }

XYPair GetCoords(CoordPair CP,int set)
{
  XYPair xy;
  if (set==0)
    {
      xy.x=CP.x1;xy.y=CP.y1;
    }
  else 
    {
      xy.x=CP.x2;xy.y=CP.y2;
    }
  if (set>1)
    printf("defaulted to copying second set.");
  return xy; //always helps to return something!
}

void PrintXYPair(XYPair xy)
{
  printf(" %d, %d\n",xy.x,xy.y);
}

int GetGameBoardSize()
{ 
  string info="";
  char buffer[BUFFERLENGTH];
  FILE *in;
  int  returnval=-1;
  //  if ((in = popen("xwininfo -root -tree | grep gweled","r"))) //method 1 
  if ((in = popen("xwininfo -name Gweled | grep geometry","r"))) //method 2 
    { 

      while (fgets(buffer, sizeof(buffer), in)!=NULL)
	{ 
	  info.append(buffer); //http://www.linuxquestions.org/questions/programming-9/c-c-popen-launch-process-in-specific-directory-620305/#post3053479
	}
      pclose(in);
      if (info.rfind("269x319")!=string::npos||info.rfind("256x323")!=string::npos) //was 256x323 (ubuntu icewm) 269x319 (arch icewm)
	{
#if DEBUGPRINT_0
	  std::cout<<"Small Board"<<std::endl;
#endif
	  returnval= 0;
	}
      else if (info.rfind("384x447")!=string::npos||info.rfind("384x451")!=string::npos) //was 384x451(ubuntu icewm) 383x447(arch icewm)
	{
#if DEBUGPRINT_0
	  std::cout<<"Medium Board"<<std::endl;
#endif
	  returnval= 1;
	}
      else if (info.rfind("512x575")!=string::npos||info.rfind("512x579")!=string::npos) //was 512x579(ubuntu icewm) 512x575 (arch icewm)
	{
#if DEBUGPRINT_0
	  std::cout<<"Large Board"<<std::endl;
#endif
	  returnval= 2;
	}
    }
  
  return returnval;
}

XYPair ConvertGameBoardToScreenCoordinates(XYPair gameboardcoords,int whole_cell_size, XYPair movecoords)//not sure if need to use const,
{

  XYPair screen_coords;

  screen_coords.x=gameboardcoords.x+whole_cell_size/2+whole_cell_size*movecoords.x;

#if DEBUGPRINT_2
  printf("gameboard.x %d, cell size %d, move.x %d\n",gameboardcoords.x,whole_cell_size,movecoords.x);
#endif

  screen_coords.y=gameboardcoords.y+whole_cell_size/2+whole_cell_size*movecoords.y;

#if DEBUGPRINT_2
  printf("gameboard.y %d, cell size %d, move.y %d\n\n",gameboardcoords.y,whole_cell_size,movecoords.y);
#endif
  return screen_coords;
}
int SetGameBoardCellSize()
{
  int gameboardsize=GetGameBoardSize();
  int wholecellpx=0; 
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
  return wholecellpx;
}

void mouseClick(int button)
{
  Display *display = XOpenDisplay(NULL);

  XEvent event;
  
  if(display == NULL)
    {
      fprintf(stderr, "Errore nell'apertura del Display !!!\n");
      exit(EXIT_FAILURE);
    }
  
  memset(&event, 0x00, sizeof(event));
  
  event.type = ButtonPress;
  event.xbutton.button = button;
  event.xbutton.same_screen = True;
  
  XQueryPointer(display, RootWindow(display, DefaultScreen(display)), &event.xbutton.root, &event.xbutton.window, &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
  
  event.xbutton.subwindow = event.xbutton.window;
  
  while(event.xbutton.subwindow)
    {
      event.xbutton.window = event.xbutton.subwindow;
      
      XQueryPointer(display, event.xbutton.window, &event.xbutton.root, &event.xbutton.subwindow, &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
    }
  
  if(XSendEvent(display, PointerWindow, True, 0xfff, &event) == 0) fprintf(stderr, "Error\n");
  
  XFlush(display);
  
  usleep(100000);//so far reducing this breaks the program
  
  event.type = ButtonRelease;
  event.xbutton.state = 0x100;
  
  if(XSendEvent(display, PointerWindow, True, 0xfff, &event) == 0) fprintf(stderr, "Error\n");
  
  XFlush(display);
  
  XCloseDisplay(display);
}


void MoveToCoordinatesAndClick(XYPair Move)
{
  Display *display = XOpenDisplay(0);
  Window root = DefaultRootWindow(display);


  XWarpPointer(display, None, root, 0, 0, 0, 0, Move.x, Move.y);
  mouseClick(Button1);
  XFlush(display); //need to flush after EACH move
  XCloseDisplay(display);
	
}


void PerformMove(CoordPair Move)
{
  Display *display = XOpenDisplay(0);
  Window root = DefaultRootWindow(display);
  int wholecellpx=SetGameBoardCellSize();
  XYPair screen_coords;
  XYPair origin=GetGameBoardOriginCoordinates();//need to update this every loop in case board was moved.       
#if DEBUGPRINT_2
  PrintMove(Move);
#endif

  screen_coords= ConvertMoveToScreenCoordinates(origin,Move,wholecellpx,/*set*/ 0);

#if DEBUGPRINT_2
  PrintXYPair(screen_coords);
#endif

  XWarpPointer(display, None, root, 0, 0, 0, 0, screen_coords.x, screen_coords.y);
  mouseClick(Button1);
  XFlush(display); //need to flush after EACH move
  
  screen_coords= ConvertMoveToScreenCoordinates(origin,Move,wholecellpx,/*set*/ 1);
  //  usleep(100000);//pause for 200ms

#if DEBUGPRINT_2
  PrintXYPair(screen_coords);
#endif

  XWarpPointer(display, None, root, 0, 0, 0, 0, screen_coords.x, screen_coords.y);
  mouseClick(Button1);
  XFlush(display);
  XCloseDisplay(display);
        
}


//-------------------------
// The key code to be sent.
// A full list of available codes can be found in /usr/include/X11/keysymdef.h
#define KEYCODE_TAB 0xff89
#define KEYCODE_Space 0xff80


// Function to create a keyboard event
XKeyEvent createKeyEvent(Display *display, Window &win,
			 Window &winRoot, bool press,
			 int keycode, int modifiers)
{
  XKeyEvent event;

  event.display     = display;
  event.window      = win;
  event.root        = winRoot;
  event.subwindow   = None;
  event.time        = CurrentTime;
  event.x           = 1;
  event.y           = 1;
  event.x_root      = 1;
  event.y_root      = 1;
  event.same_screen = True;
  event.keycode     = XKeysymToKeycode(display, keycode);
  event.state       = modifiers;

  if(press)
    event.type = KeyPress;
  else
    event.type = KeyRelease;

  return event;
}

int FakeKeyPress(int KeyCode)
{
  // Obtain the X11 display.
  Display *display = XOpenDisplay(0);
  if(display == NULL)
    return -1;

  // Get the root window for the current display.
  Window winRoot = XDefaultRootWindow(display);

  // Find the window which has the current keyboard focus.
  Window winFocus;
  int    revert;
  XGetInputFocus(display, &winFocus, &revert);
  // Send a fake key press event to the window.
  XKeyEvent event = createKeyEvent(display, winFocus, winRoot, true, KeyCode, 0  );
  XSendEvent(event.display, event.window, True, KeyPressMask, (XEvent *)&event);

  // Send a fake key release event to the window.
  event = createKeyEvent(display, winFocus, winRoot, false, KeyCode, 0);
  XSendEvent(event.display, event.window, True, KeyPressMask, (XEvent *)&event);

  // Done.
  XCloseDisplay(display);

  return 0;
}

//To compile it, you need to type this:
//g++ -o XFakeKey XFakeKey.cpp -L/usr/X11R6/lib -lX11

int TabFakeKeyPress()
{
  int errorcode= FakeKeyPress(XK_Tab); //XK_KP_Tab);
  printf("TabFakeKeyPress errorcode:%d\n",errorcode);
  return errorcode;
}

int SpaceFakeKeyPress()
{
  int errorcode= FakeKeyPress(XK_space) ; // XK_KP_Space) ;        
  printf("SpaceFakeKeyPress errorcode:%d\n",errorcode);            
  return errorcode;
}

/* 
Errors & Solutions:
 expected unqualified-id before numeric constant: had a function named KeyPress() yet KeyPress is an event type in the Xlib library. Changed KeyPress to FakeKeyPress() All uppercase names are often used for preprocessor macros, which
                                                             doesn't respect namespace scopes. Therefore such names should
                                                             generally be avoided for everything else.

*/
