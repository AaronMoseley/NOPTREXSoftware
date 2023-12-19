#include "rootInterface.h"

using namespace std;

RootInterface::RootInterface(TTree* tree, time_t* timestampAddr, vector<vector<uint32_t>>* waveformAddr, int channels)
{
	tree->Branch("TimeStamp", timestampAddr);

	for (int i = 0; i < waveformAddr->size(); i++)
	{
		tree->Branch(("Waveform" + to_string(i)).c_str(), &(waveformAddr->at(i)));
	}

	this->numChannels = channels;
}

RootInterface::RootInterface(TTree* tree, time_t* timestampAddr, vector<vector<uint32_t>>* waveformAddr, string timestampName, string waveformName, int channels)
{

	tree->Branch(timestampName.c_str(), timestampAddr);


	for (int i = 0; i < waveformAddr->size(); i++)
	{
		tree->Branch((waveformName + to_string(i)).c_str(), &(waveformAddr->at(i)));
	}

	this->numChannels = channels;
}

void RootInterface::WriteData(TTree* tree)
{
	tree->Fill();
}

void RootInterface::WriteToFile(TTree* tree, string fileName, string objName)
{
	unique_ptr<TFile> outputFile(TFile::Open(fileName.c_str(), "RECREATE"));
	outputFile->WriteObject(tree, objName.c_str());
}

void RootInterface::ReadFile(string fileName, string objName)
{
	unique_ptr<TFile> file(TFile::Open(fileName.c_str(), "READ"));
	if (!file || file->IsZombie())
	{
		cerr << "Error opening file";
		exit(-1);
	}

	cout << "File Contents: " << endl;
	file->ls();

	cout << endl;

	unique_ptr<TTree> tree(file->Get<TTree>(objName.c_str()));
	tree->Print();

	cout << endl << endl;

	int numEntries = tree->GetEntries();

	for (int i = 0; i < numEntries; i++)
	{
		tree->Show(i);
		cout << endl;
	}
}

void RootInterface::ResetTree(TTree* tree)
{
	tree->GetListOfBranches()->Clear();
}
