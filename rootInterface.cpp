#include "rootInterface.h"

using namespace std;

RootInterface::RootInterface(TTree *tree, time_t *timestampAddr, int *energyAddr, vector<int> *waveformAddr)
{
	tree->Branch("TimeStamp", timestampAddr);
	tree->Branch("Energy", energyAddr);
	tree->Branch("Waveform", waveformAddr);
}

RootInterface::RootInterface(TTree *tree, time_t *timestampAddr, int *energyAddr, vector<int> *waveformAddr, string timestampName, string energyName, string waveformName)
{

	tree->Branch(timestampName.c_str(), timestampAddr);
	tree->Branch(energyName.c_str(), energyAddr);
	tree->Branch(waveformName.c_str(), waveformAddr);
}

void RootInterface::WriteData(TTree *tree)
{
	tree->Fill();
}

void RootInterface::WriteToFile(TTree *tree, string fileName, string objName)
{
	unique_ptr<TFile> outputFile(TFile::Open(fileName.c_str(), "RECREATE"));
	outputFile->WriteObject(tree, objName.c_str());
}

void RootInterface::ReadFile(string fileName, string objName)
{
	unique_ptr<TFile> file(TFile::Open(fileName.c_str(), "READ"));
	if(!file || file->IsZombie())
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
	
	TBranch *bWaveForm = 0;
	vector<int> *waveForm = 0;
	tree->SetBranchAddress("Waveform", &waveForm, &bWaveForm);
	
	for(int i = 0; i < numEntries; i++)
	{
		tree->Show(i);
	
		bWaveForm->GetEntry(tree->LoadTree(i));
	
		for(int j = 0; j < waveForm->size(); j++)
		{
			cout << "Waveform Entry " << j << ": " << waveForm->at(j) << endl;
		}
	
		cout << endl;
	}
}
