//g++ -O2 -o MWT MouseWarppointerTest.cpp  -lX11  :)

#include <time.h> //for sleep 
#include <cstring> //for memset
#include <unistd.h> //for usleep

//#include <iostream> //stderr exit fprint EXIT_FAILURE 
#include <stdio.h> //stderr,fprintf 
#include <cstdlib> //exit EXIT_FAILURE


#include <X11/Xlib.h>
#include <X11/Xutil.h>  //add on -lX11 to compile command

//#include <X11/keysym.h> //for fake keypress

using namespace std; 

struct XYPair
{
  int x,y;
};

void MoveToCoordinatesAndClick(XYPair Move);
void mouseClick(int button);


int main(int argc,char **argv) 
{ 

  XYPair move;
  move.x=80;
  move.y=0;

  while(true)
    {
      MoveToCoordinatesAndClick( move);
      move.y+=100;
      if (move.y>1200)
	move.y=0;

      sleep(1);
    }

  return 0; 
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
  
  //  XFlush(display);
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





