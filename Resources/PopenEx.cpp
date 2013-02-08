#include <iostream>
#include <stdio.h>
using namespace std;
int main ()
{
  FILE *in;
  char buff[512];
  if (!(in = popen("xwininfo -root -tree | grep gweled | grep 256x323 | awk '{ print $1 }'","r")))
    {
      return 1;
    }
  while (fgets(buff, sizeof(buff), in)!=NULL)
    {
      cout<<buff;
    }
     cout<<buff;
  pclose(in);
  return 0;

}

//http://www.linuxquestions.org/questions/programming-9/c-c-popen-launch-process-in-specific-directory-620305/#post3053479
//this post helped to convert the buff to a string.
