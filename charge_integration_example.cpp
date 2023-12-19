#include "Def.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <thread>
#include <algorithm>
#include <chrono>
#include <fstream>

#include "RegisterFile.h"

extern "C" {
#include "chrgint_new_lib.h"
}

//#include "rootInterface.h"

#include "H5Interface.h"
#include <highfive\H5File.hpp>

#define BOARD_IP_ADDRESS "172.16.50.185"

using namespace std;

const int numChannels = 2;
const int oscSize = 16384;

class DigitizerInterface {
public:
	DigitizerInterface(uint32_t timeout, uint32_t pretrigger, uint32_t decimation, uint32_t trigger)
	{
		this->timeout = timeout;
		this->pretrigger = pretrigger;
		this->decimation = decimation;
		this->triggerLevel = trigger;
	}

	int setInputs(NI_HANDLE* handle, uint32_t signalThreshold, uint32_t intTime, uint32_t gain, uint32_t decimation)
	{
		//Sets signal threshold and integration time, as well as any other necessary inputs
		if (__abstracted_reg_write(signalThreshold, SCI_REG_Threshold, handle) != 0)
		{
			cout << "Error setting signal threshold" << endl;
			return -1;
		}

		if (__abstracted_reg_write(intTime, SCI_REG_int_time, handle) != 0)
		{
			cout << "Error setting integration time" << endl;
			return -1;
		}

		if (__abstracted_reg_write(intTime, SCI_REG_int_signal, handle) != 0)
		{
			cout << "Error setting integration signal" << endl;
			return -1;
		}

		if (__abstracted_reg_write(gain, SCI_REG_gain, handle) != 0)
		{
			cout << "Error setting gain" << endl;
			return -1;
		}

		if (__abstracted_reg_write(decimation, SCI_REG_Oscilloscope_0_CONFIG_DECIMATOR, handle) != 0)
		{
			cout << "Error setting decimation" << endl;
			return -1;
		}

		if (__abstracted_reg_write(decimation, SCI_REG_Oscilloscope_1_CONFIG_DECIMATOR, handle) != 0)
		{
			cout << "Error setting decimation" << endl;
			return -1;
		}

		return 0;
	}

	uint32_t getTrigger(NI_HANDLE* handle)
	{
		uint32_t data;

		if (__abstracted_reg_read(&data, SCI_REG_t_0_trigger, handle) != 0)
		{
			cout << "Error getting trigger" << endl;
			return -1;
		}

		return data;
	}

	int startOsc0(NI_HANDLE* handle)
	{
		if (OSCILLOSCOPE_Oscilloscope_0_SET_PARAMETERS(this->decimation, this->pretrigger, 0, 0, 0, 0, 0, 0, 0, 1, 1, handle) != 0)
		{
			cout << "Parameters Error" << endl;
			return -1;
		}

		if (OSCILLOSCOPE_Oscilloscope_0_START(handle) != 0)
		{
			cout << "Start Error" << endl;
			return -1;
		}
	}

	int getNewData(NI_HANDLE* handle, vector<vector<uint32_t>>* waveforms, time_t* timestamp, int entriesPerEvent, bool osc)
	{
		uint32_t dataArr[(oscSize * numChannels) + 16];
		uint32_t read;
		uint32_t valid;
		int32_t position;

		uint32_t readAnalog[(oscSize * numChannels) + 16];
		uint32_t readDigit0[(oscSize * numChannels) + 16];

		uint32_t status = 0;
		if (osc)
		{
			/*while (status % 2 != 1)
			{
				if (OSCILLOSCOPE_Oscilloscope_0_STATUS(&status, handle) != 0)
				{
					cout << "Status error" << endl;
				}
				//cout << status << endl;
			}*/

			*timestamp = time(nullptr);

			if (OSCILLOSCOPE_Oscilloscope_1_SET_PARAMETERS(this->decimation, this->pretrigger, 0, 0, 0, 0, 0, 0, 0, 1, 1, handle) != 0)
			{
				cout << "Parameters Error" << endl;
				return -1;
			}

			if (OSCILLOSCOPE_Oscilloscope_1_START(handle) != 0)
			{
				cout << "Start Error" << endl;
				return -1;
			}

			if (OSCILLOSCOPE_Oscilloscope_0_POSITION(&position, handle) != 0)
			{
				cout << "Position Error" << endl;
				return -1;
			}

			if (OSCILLOSCOPE_Oscilloscope_0_DOWNLOAD(dataArr, oscSize * numChannels, this->timeout, handle, &read, &valid) != 0)
			{
				cout << "Download Error" << endl;
				return -1;
			}

			if (OSCILLOSCOPE_Oscilloscope_0_RECONSTRUCT(dataArr, position, this->pretrigger, readAnalog, readDigit0, readDigit0, readDigit0, readDigit0) != 0)
			{
				cout << "Reconstruct Error" << endl;
				return -1;
			}
		}
		else {
			/*while (status % 2 != 1)
			{
				if (OSCILLOSCOPE_Oscilloscope_1_STATUS(&status, handle) != 0)
				{
					cout << "Status error" << endl;
				}
				//cout << status << endl;
			}*/

			*timestamp = time(nullptr);

			if (OSCILLOSCOPE_Oscilloscope_0_SET_PARAMETERS(this->decimation, this->pretrigger, 0, 0, 0, 0, 0, 0, 0, 1, 1, handle) != 0)
			{
				cout << "Parameters Error" << endl;
				return -1;
			}

			if (OSCILLOSCOPE_Oscilloscope_0_START(handle) != 0)
			{
				cout << "Start Error" << endl;
				return -1;
			}

			if (OSCILLOSCOPE_Oscilloscope_1_POSITION(&position, handle) != 0)
			{
				cout << "Position Error" << endl;
				return -1;
			}

			if (OSCILLOSCOPE_Oscilloscope_1_DOWNLOAD(dataArr, oscSize * numChannels, this->timeout, handle, &read, &valid) != 0)
			{
				cout << "Download Error" << endl;
				return -1;
			}

			if (OSCILLOSCOPE_Oscilloscope_1_RECONSTRUCT(dataArr, position, this->pretrigger, readAnalog, readDigit0, readDigit0, readDigit0, readDigit0) != 0)
			{
				cout << "Reconstruct Error" << endl;
				return -1;
			}
		}

		for (int i = 0; i < numChannels; i++)
		{
			waveforms->at(i) = vector<uint32_t>(readAnalog + (oscSize * i), readAnalog + (oscSize * i) + entriesPerEvent);
		}

		return 0;
	}

	int connectToDigitizer(string boardIP, NI_HANDLE* handle)
	{
		//Connects to digitizer
		R_Init();

		if (R_ConnectDevice(&boardIP[0], 8888, handle) != 0)
		{
			cout << "Unable to connect to the board." << endl;
			return -1;
		}

		return 0;
	}

private:
	uint32_t timeout;
	uint32_t pretrigger;
	uint32_t decimation;
	uint32_t triggerLevel;
};

string* splitEq(string orig)
{
	int ind = -1;
	for (int i = 0; i < orig.length(); i++)
	{
		if (orig[i] == '=')
		{
			ind = i;
			break;
		}
	}

	string* result = new string[2];
	result[0] = orig.substr(0, ind);
	result[1] = orig.substr(ind + 1);

	return result;
}

int parseConfig(string configFileName, string* outputFileName, uint32_t* signalThreshold, uint32_t* intTime, uint32_t* gain, int* numEvents, int* numFileEvents, uint32_t* entriesPerEvent, uint32_t* pretrigger, uint32_t* decimation)
{
	fstream config;

	config.open(configFileName, ios::in);

	if (!config.is_open())
	{
		cout << "Error opening config file" << endl;
		return 1;
	}

	string currLine;
	while (getline(config, currLine))
	{
		if (currLine[0] == '#' || currLine[0] == '\n')
			continue;

		string* values = splitEq(currLine);
		if (values[0] == "OutputFileName")
		{
			*outputFileName = values[1];
		}
		else if (values[0] == "SignalThreshold")
		{
			*signalThreshold = stoi(values[1]);
		}
		else if (values[0] == "IntegrationTime")
		{
			*intTime = stoi(values[1]);
		}
		else if (values[0] == "Gain")
		{
			*gain = stoi(values[1]);
		}
		else if (values[0] == "NumberOfEvents")
		{
			*numEvents = stoi(values[1]);
		}
		else if (values[0] == "EventsInFile")
		{
			*numFileEvents = stoi(values[1]);
		}
		else if (values[0] == "EntriesPerEvent")
		{
			*entriesPerEvent = stoi(values[1]);
		}
		else if (values[0] == "Pretrigger")
		{
			*pretrigger = stoi(values[1]);
		}
		else if (values[0] == "Decimation")
		{
			*decimation = stoi(values[1]);
		}
		else {
			cout << "Config file error" << endl;
			return 1;
		}
	}

	return 0;
}

//Command line args: output file name, output object name, signal threshold, integration time, gain, number of total events, number of events per file, max number of entries per event, pretrigger samples, decimation
//Command line args: config file name
int main(int argc, char* argv[])
{
	/*int numArgs = 10;
	if (argc != numArgs + 1)
	{
		cout << "Error: number of arguments was " << argc - 1 << " when it should have been " << numArgs << endl;
		system("pause");
		return 1;
	}

	bool readFile = false;

	cout << "Starting..." << endl;

	string outputFile = argv[1];
	string outputObj = argv[2];
	uint32_t signalThreshold = stoi(argv[3]);
	uint32_t integrationTime = stoi(argv[4]);
	uint32_t gain = stoi(argv[5]);
	int numEvents = stoi(argv[6]);
	int numFileEvents = stoi(argv[7]);
	uint32_t entriesPerEvent = stoi(argv[8]);
	uint32_t pretrigger = stoi(argv[9]);
	uint32_t decimation = stoi(argv[10]);*/

	if (argc != 2)
	{
		cout << "Please only give the name of the config file" << endl;
		system("pause");
		return 1;
	}

	string outputFile;
	uint32_t signalThreshold;
	uint32_t integrationTime;
	uint32_t gain;
	int numEvents;
	int numFileEvents;
	uint32_t entriesPerEvent;
	uint32_t pretrigger;
	uint32_t decimation;

	string configFileName = argv[1];

	if (parseConfig(configFileName, &outputFile, &signalThreshold, &integrationTime, &gain, &numEvents, &numFileEvents, &entriesPerEvent, &pretrigger, &decimation) != 0) return 1;

	bool readFile = false;
	cout << "Starting..." << endl;

	NI_HANDLE handle;

	DigitizerInterface digit = DigitizerInterface(1000, pretrigger, decimation, signalThreshold);

	if (digit.connectToDigitizer(BOARD_IP_ADDRESS, &handle) != 0)
	{
		cerr << "Error: Could not connect to digitizer" << endl;
		return 1;
	}

	cout << "Connected to digitizer" << endl;

	digit.setInputs(&handle, signalThreshold, integrationTime, gain, decimation);

	cout << "Successfully set digitizer inputs" << endl;

	vector<vector<uint32_t>> currData;

	for (int i = 0; i < numChannels; i++)
	{
		vector<uint32_t> wave;
		currData.push_back(wave);
	}

	time_t timestamp = time(nullptr);

	int numRecords = 0;

	//TTree tree = TTree();
	//RootInterface root = RootInterface(&tree, &timestamp, &currData, numChannels);

	HighFive::File file(outputFile + "0.hdf5", HighFive::File::OpenOrCreate);
	H5Interface fileInter(&file, outputFile + "0.hdf5");

	int totalEvents = 0;
	int eventsInFile = 0;
	int numFiles = 0;
	cout << "Reading waveform data" << endl;

	digit.startOsc0(&handle);

	bool osc = true;

	while (totalEvents < numEvents)
	{
		digit.getNewData(&handle, &currData, &timestamp, entriesPerEvent, osc);
		//root.WriteData(&tree);
		fileInter.WriteData(currData, timestamp, totalEvents);
		totalEvents++;
		eventsInFile++;
		osc = !osc;

		if (totalEvents % 10 == 0)
			cout << totalEvents << endl;

		if (eventsInFile >= numFileEvents)
		{
			numFiles++;
			//root.WriteToFile(&tree, outputFile + to_string(numFiles) + ".root", outputObj);
			//root.ResetTree(&tree);

			//root = RootInterface(&tree, &timestamp, &currData, numChannels);

			if (totalEvents < numEvents)
			{
				file = HighFive::File(outputFile + to_string(numFiles) + ".hdf5", HighFive::File::OpenOrCreate);
				fileInter = H5Interface(&file, outputFile + to_string(numFiles) + ".hdf5");

				eventsInFile = 0;
			}

			cout << "Created file: " << outputFile << numFiles << ".root" << endl;

			cout << numFileEvents << endl;
		}
	}

	/*if (eventsInFile > 0)
	{
		root.WriteToFile(&tree, outputFile + to_string(numFiles) + ".root", outputObj);
	}*/

	cout << "Quitting..." << endl;

	if (readFile)
	{
		//root.ReadFile(outputFile + to_string(i) + ".root", outputObj);
		fileInter.ReadFile(eventsInFile, numChannels);
	}

	system("pause");
	return 0;
}