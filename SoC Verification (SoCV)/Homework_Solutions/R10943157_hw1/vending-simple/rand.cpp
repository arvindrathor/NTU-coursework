#include<iostream>
#include<fstream>
#include<stdlib.h>
#include<time.h>    
#include <cstdlib>

#define min 1
#define max 3

using namespace std;

void genPat(ofstream &testPat)
{
    for(int k=0;k<5;k++)
    {   int randNum = rand()%(max-min + 1) + min;
        if(randNum==1)
        {
         testPat <<"01";
        }
         if(randNum==2)
        {
         testPat <<"10";
        }
         if(randNum==3)
        {
         testPat <<"11";
        }
    }
    testPat<<"1"<<endl;
}

int main(int argc,char* argv[])
{   ofstream testPat;
    testPat.open("Test3.pattern");
    srand(time(NULL));
    int max_iter=atoi(argv[1]);
    testPat <<max_iter<<" 11"<<endl;
    testPat <<"XXXXXXXXXX0"<<endl;
    int count=1;
    int it=((max_iter-1)/11);
    for(int i=0;i<it;i++)
    {   cout <<((max_iter-1)/11)<<endl;
        genPat(testPat);
        count++;
        for(int j=0;j<10;j++)
            {testPat <<"00XXXXXXXX1"<<endl;}
        count+=10;
    }
    genPat(testPat);
    int a= max_iter-1-count;
    for(int j=0;j<a;j++)
            { 
                testPat <<"00XXXXXXXX1"<<endl;}

    cout<<count;
    return 0;
}
