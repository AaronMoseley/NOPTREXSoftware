#include "Def.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include <iostream>
#include <string>
#include <vector>
#include <ctime>

#include "RegisterFile.h"
#include "charge_integration_lib.h"
#include "rootInterface.h"

extern "C" int __abstracted_reg_read(uint32_t * data, uint32_t address, NI_HANDLE * handle);
extern "C" int __abstracted_reg_write(uint32_t data, uint32_t address, NI_HANDLE * handle);

extern "C" int LISTMODULE_List_0_RESET(NI_HANDLE * handle);
extern "C" int LISTMODULE_List_0_DOWNLOAD(uint32_t * val, uint32_t size, int32_t timeout, NI_HANDLE * handle, uint32_t * read_data, uint32_t * valid_data);
extern "C" int LISTMODULE_List_0_STATUS(uint32_t * status, NI_HANDLE * handle);
extern "C" int LISTMODULE_List_0_START(NI_HANDLE * handle);

#define BOARD_IP_ADDRESS "172.16.50.185"

using namespace std;

class DigitizerInterface {
	public:
		DigitizerInterface(uint32_t timeout)
		{
			timeoutLength = timeout;
			triggerReset = 0;
		}

		int setInputs(NI_HANDLE* handle, uint32_t signalThreshold, uint32_t intTime)
		{
			//Sets signal threshold and integration time, as well as any other necessary inputs
			if (__abstracted_reg_write(signalThreshold, SCI_REG_Threshold, handle) != 0)
			{
				cout << "Error setting signal threshold" << endl;
				return -1;
			}

			if (__abstracted_reg_write(intTime, SCI_REG_Int_Time, handle) != 0)
			{
				cout << "Error setting integration time" << endl;
				return -1;
			}

			if (__abstracted_reg_write(intTime, SCI_REG_int_signal, handle) != 0)
			{
				cout << "Error setting integration signal" << endl;
				return -1;
			}

			return 0;
		}

		int getSignal(NI_HANDLE* handle)
		{
			//Reads signal register and sets it to 0 when finished
			//Requires that when the signal register is set, it stay set, talk to JT about this
			// 
			// 
			//Assumes TEMP is the address of the write register
			//Assumes RAWTRIGGER as the address of the trigger register with raw data
			//Assumes that the signal going to t0_trigger updates constantly and is 1 if the signal is high enough, 0 if not
			//Assumes an AND logic gate between the register labeled TEMP and the signal currently going to t0_trigger

			/*uint32_t result = -1;
			if (__abstracted_reg_read(&result, SCI_REG_t0_trigger, handle) != 0)
			{
				cout << "Unable to read trigger signal" << endl;
				return -1;
			}

			return (int)result;*/

			uint32_t triggerData = -1;
			if (__abstracted_reg_read(&triggerData, SCI_REG_t0_trigger, handle) != 0)
			{
				cout << "Unable to read trigger signal" << endl;
				return -1;
			}

			if (triggerData == 0 && triggerReset == 0)
			{
				triggerReset = 1;
			}else if (triggerData == 1 && triggerReset == 1)
			{
				triggerReset = 0;
				return 1;
			}

			return 0;
		}

		vector<int> getNewData(int lastInd, NI_HANDLE* handle)
		{
			//Returns a vector of all new data since the previous index
			uint32_t read;
			uint32_t valid;

			uint32_t startValid;
			bool first = true;

			uint32_t totalRead = 0;

			vector<int> result;

			uint32_t dataArr[26];

			do {
				if (LISTMODULE_List_0_DOWNLOAD(dataArr, 10, timeoutLength, handle, &read, &valid) != 0)
				{
					cout << "Error reading list" << endl;
					return result;
				}

				if (first)
				{
					startValid = valid;
					first = false;
				}

				for (int i = 0; i < (int)read; i++)
				{
					result.push_back(dataArr[i]);
				}

				totalRead += read;
			} while (totalRead < startValid - lastInd);

			return result;
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
		uint32_t timeoutLength;
		int triggerReset;
};

//Command line args: output file name, output object name, signal threshold, integration time, number of events
int main(int argc, char* argv[])
{
	int numArgs = 5;
	if (argc != numArgs + 1)
	{
		cerr << "Error: number of arguments was " << argc - 1 << " when it should have been " << numArgs << endl;
		return 1;
	}

	uint32_t signalThreshold = stoi(argv[3]);
	uint32_t integrationTime = stoi(argv[4]);
	int numEvents = stoi(argv[5]);

	string outputFile = argv[1];
	string outputObj = argv[2];

	NI_HANDLE handle;

	DigitizerInterface digit = DigitizerInterface(1000);

	if (digit.connectToDigitizer(BOARD_IP_ADDRESS, &handle) != 0)
	{
		cerr << "Error: Could not connect to digitizer" << endl;
		return 1;
	}

	digit.setInputs(&handle, signalThreshold, integrationTime);

	vector<int> currData;
	time_t timestamp = time(nullptr);
	int energy = 0;

	int numRecords = 0;

	uint32_t listStatus = 0;

	if (LISTMODULE_List_0_RESET(&handle) != 0)
	{
		cerr << "List Reset Error" << endl;
	}

	if (LISTMODULE_List_0_START(&handle) != 0)
	{
		cerr << "List Start Error" << endl;
	}

	if (LISTMODULE_List_0_STATUS(&listStatus, &handle) != 0)
	{
		cerr << "List Status Error" << endl;
	}

	//ofstream filePtr;

	TTree tree = TTree();
	RootInterface root = RootInterface(&tree, &timestamp, &energy, &currData);

	int totalEvents = 1;
	if (listStatus == 2)
	{
		while (totalEvents < numEvents)
		{
			//Check for signal to start new "line" on output file
			if (digit.getSignal(&handle))
			{
				timestamp = time(nullptr);

				//Send current array/vector to root file output function
				root.WriteData(&tree);

				//Clear output array
				currData.clear();
				energy = 0;

				totalEvents++;
			}

			//Add any new elements of list to array
			vector<int> newData = digit.getNewData(numRecords, &handle);

			for (int i = 0; i < (int)newData.size(); i++)
			{
				energy += newData[i];

				currData.push_back(newData[i]);
			}

			numRecords += newData.size();
		}
	}

	root.WriteToFile(&tree, outputFile, outputObj);

	cout << "Quitting..." << endl;

	root.ReadFile(outputFile, outputObj);

	return 0;
}