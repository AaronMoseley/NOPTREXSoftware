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
	RootInterface(TTree* tree, time_t* timestampAddr, vector<vector<uint32_t>>* waveformAddr, int channels);

	RootInterface(TTree* tree, time_t* timestampAddr, vector<vector<uint32_t>>* waveformAddr, string timestampName, string waveformName, int channels);

	void ResetTree(TTree* tree);

	void WriteData(TTree* tree);

	void WriteToFile(TTree* tree, string fileName, string objName);

	void ReadFile(string fileName, string objName);

private:
	int numChannels;
};

#endif
