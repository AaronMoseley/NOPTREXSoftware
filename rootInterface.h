#ifndef ROOTINTERFACE_H
#define ROOTINTERFACE_H

#include <string>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <ctime>

#include "TKey.h"
#include "TObject.h"
#include "TROOT.h"
#include "TTree.h"
#include "TFile.h"

using namespace std;

class RootInterface
{
	public:
		RootInterface(TTree *tree, time_t *timestampAddr, int *energyAddr, vector<int> *waveformAddr);

		RootInterface(TTree *tree, time_t *timestampAddr, int *energyAddr, vector<int> *waveformAddr, string timestampName, string energyName, string waveformName);

		void WriteData(TTree *tree);

		void WriteToFile(TTree *tree, string fileName, string objName);

		void ReadFile(string fileName, string objName);
};

#endif
