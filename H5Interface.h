#ifndef H5INTERFACE_H
#define H5INTERFACE_H

#include <string>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <ctime>

#include <highfive\H5File.hpp>

using namespace std;

class H5Interface
{
public:
	H5Interface(HighFive::File* filePtr, string fileName, uint32_t signalThreshold, uint32_t integrationTime, uint32_t gain, uint32_t pretrigger, uint32_t decimation);

	void WriteData(vector<vector<uint32_t>> waveform, time_t timestamp, int eventNum);

	void ReadFile(int numEvents, int channels);

private:
	string fileName;
	HighFive::File* file;
};

#endif
