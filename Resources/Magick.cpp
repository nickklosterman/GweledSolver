//#include <Magick++.h> 
//g++ -O2 -o Magick Magick.cpp `Magick++-config --cppflags --cxxflags --ldflags --libs` -I/usr/include/GraphicsMagick/ PHAIL
//g++ -O2 -o Magick Magick.cpp `GraphicsMagick++-config --cppflags --cxxflags --ldflags --libs` -I/usr/include/GraphicsMagick/  :)
#include "/usr/include/GraphicsMagick/Magick++.h"
#include <iostream> 
//#include <sstream>
using namespace std; 
using namespace Magick; 
int main(int argc,char **argv) 
{ 
  InitializeMagick(*argv);

  size_t width,height;

  // Construct the image object. Separating image construction from the 
  // the read operation ensures that a failure to read the image file 
  // doesn't render the image object useless. 
  Image image;
  try { 
    // Read a file into image object 
    //   image.read( "test.jpg" );
    //	image.crop( Geometry(32*8,32*8, 0, 27) );
    //    image.read( "x.jpg");//white.png" );
    image.read("gweledimg.png");
    image.crop( Geometry(32*8,32*8, 0, 27) );

    width=image.columns();
    height=image.rows();
    std::cout<<"orig image is "<<width<<" by "<<height<<std::endl;

    image.modifyImage();
    image.type(TrueColorType);
    for (ssize_t counterY=15;counterY<255;)
      {
	for (ssize_t counterX=15;counterX<255;)
	  {
	    //std::cout<<counterX<<" "<<counterY<<std::endl;
	    PixelPacket *imagePixels = image.getPixels(counterX,counterY,1,1);
	    PixelPacket *pixels = imagePixels;//+0;

	    Color color;
	    color = imagePixels[0];
	    //	    std::cout<<int(color.redQuantum())<<","<<int(color.greenQuantum())<<","<<int(color.blueQuantum())<<std::endl;
	    switch (int(color.greenQuantum()))
	      {
	      case 185:
		std::cout<<"B";
		break;
	      case 63:
		std::cout<<"O";
		break;
	      case 153:
		std::cout<<"G";
		break;
	      case 145:
		std::cout<<"Y";
		break;
	      case 224:
		std::cout<<"W"; break;
	      case 0:
	      case 2:
		std::cout<<"P"; break;
	      case 91:
		std::cout<<"R";break;
	      default:
		std::cout<<"*";
		std::cout<<int(color.redQuantum())<<","<<int(color.greenQuantum())<<","<<int(color.blueQuantum())<<std::endl;
	      }
	    /*
	        0,185,245 blue diamond
	      175, 63, 5 orange hexagon 
	      5  ,153, 1 green rounded square
	      145,145, 3 yellow square
	      224,224,224 white round bauble
	      175,  0,174 purple triangle
	      176,  2,176 purple triangle
	      213, 91,110 red rounded square
	    */
	    *pixels=Color("red");	    
	    counterX+=32;
	  }
	counterY+=32;
	    std::cout<<std::endl;
      }
    image.syncPixels();
    
    if (1==0)
      {
	PixelPacket *imagePixels = image.getPixels(0,0,width,height);
	Color color;
	//    ColorRGB color;
	//    for (size_t i=32/2*width+32/2; i<width*height;)
	int y=16;
	for (size_t counter=16;counter<256*256;)
	  {
	    //	size_t counter=y*width+x;//32/2*width+32*i; //<<---as a test change the color of the pixels and sync and view image to see if have correct placement, also see if import works for screen cap
	    color = imagePixels[counter];
	    //	std::cout<<*imagePixels[i].red<<","<<*imagePixels[i].green<<","<<*imagePixels[i].blue<<std::endl;

	    std::cout<<int(color.redQuantum())<<","<<int(color.greenQuantum())<<","<<int(color.blueQuantum())<<std::endl;
	    //	std::cout<<color.redQuantum()<<","<<color.greenQuantum()<<","<<color.blueQuantum()<<std::endl; //prints out unsigned char (I think )
	
	    //std::cout<<color.red()<<","<<color.green()<<","<<color.blue()<<std::endl;

	    //	std::cout<<"i";
	    if (counter%240==0)
	      {
		counter+=32*width;
	      }
	    else
	      counter+=32;//i+=32/2;
	  }
      }
    std::cout<<"maxrgb is "<<MaxRGB<<std::endl;

    if (1==1)
      {
	// Crop the image to specified size (width, height, xOffset, yOffset)
	//    image.crop( Geometry(32*8,32*8, 0, 27) );
	std::cout<<"writing image to xmod.jpg"<<std::endl;
	// Write the image to a file 
	image.write( "xmod.png" ); 
      }
  } 
  catch( Exception &error_ ) 
    { 
      cout << "Caught exception: " << error_.what() << endl; 
      return 1; 
    } 
  return 0; 
}
