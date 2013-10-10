#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "distribution.h"

int _k;
double _delta;
double _R;
double _beta;

int next_int(int min, int max) 
{
   return rand() % (max - min + 1) + min;
}

double next_double()
{
  int random_repeat = rand()%10;
  int i;
  for (i=0; i < random_repeat; ++i) rand();
  double random = (double)rand()/RAND_MAX;
  return random;
}

double rho(int i)
{
  if(i<1 || i>_k)
  {
    printf("Invalid algorithm parameter\n");
    return;
  }

  if (i==1)
    return 1.0/_k;
  else
    return 1.0/(i*(i-1.0));
}

double tau(int i)
{
  if(i<1 || i>_k)
  {
    printf("Invalid algorthm parameter\n");
    return;
  }

  int kR = (int) round(_k/_R);
  if (i < kR)
    return _R/(i*_k);
  else
    if (i > kR)
      return 0;
    else
      return _R*log(_R/_delta)/_k;
}

double mu(int i)
{
  return (rho(i)+tau(i))/_beta;
}

// Inizializza i parametri per la distribuzione rsd
void initialize_rsd(double c, double delta, int k)
{
  int i;

  if(k>0 && c>0 && delta>=0 && delta<=1)
  {
    _k = k;
    _delta = delta;
  }
  else
  {
    printf("Invalid arguments");
    return;
  }
  
  _R = c * log(k/delta) * sqrt(k);
  
  _beta=0;
  for(i=1; i<=k; i++)
  {
    _beta += rho(i) + tau(i);
  }
}

//Ritorna un intero random preso dalla distribuzione rsd
int rsd()
{
  double r = next_double();
  double sum = 0;
  int d=0;
  while(sum<r)
    sum+=mu(++d);

  return d;
}

//Ritorna un intero random preso da una distribuzione esponenziale
int expd(int max)
{
  return (int)(log(1-next_double())/(-0.1)) % (max) + 1;
}