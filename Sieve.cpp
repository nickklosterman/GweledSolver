#ifndef AVOID_STANDARD_BITSET

#include <bitset>

#else
template <int N >
class bitset
{
public:
  bitset() : bits(new char[(N-1) / 8+1]) {}

  bool test(intn n)
  {
    return (bits[n>>3]&(1 << (n&7))) !=0;
  }
  void set(int n)
  {
    bits[n>>3] |= 1 <<(n&7);
  }
  void reset(int n)
  { bits[n>>3] &= ~(1 << (n & 7));
  }
private:
  char* bits;
};
#endif

#include <iostream>
#include <ctime>
using namespace std;

int main()
{
  const int N = 10000000;
  clock_t cstart=clock();
  bitset<N+1> b;
  int count = 0 ;
  int i;
  for (i =2; i<=N; i++)
    b.set(i);
  i=2;
  while(i*i<=N)
    {
      if (b.test(i))
	{
	  //	  int k = 2*i;
	  int k=i*i;
	  while (k<=N)
	    { b.reset(k);
	      k+=i;
	    }
	}
      i++;
    }
  for (i=2;i<=N;i++)
    {
      if (b.test(i))\
	{
#ifdef PRINT
	    cout << i << "\n";
#endif
	  count++;
	}
    }
  clock_t cend=clock();
  double millis = 1000.0 * (cend -cstart) / CLOCKS_PER_SEC;
  cout << count << " primes \n" << millis <<  " milliseconds\n";
  return 0;
}
