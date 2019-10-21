 #include<stdio.h>
#include<unistd.h>
#include<string.h>

//#!/bin/bash

#define SIZE 102
int main()
{
  int i =0;
  char  bar[SIZE];
  memset(bar, 0 , sizeof(bar));
  const char* lable = "|/-\\";
  while(i <= 100)
  {
    // printf("[%100s][%d%%][%c]\r",bar,i,lable[i%4]);
     printf("\e[31m\033[40m[%-100s]\e[32m\033[47m[%d%%]\e[30m\033[%c]\r",bar,i,lable[i%4]);
	 fflush(stdout);
	 bar[i++]= '#';
	 usleep(100000);

  }
  printf("\n");
  return 0;

}

