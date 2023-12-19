#include "H5Interface.h"

using namespace std;

H5Interface::H5Interface(HighFive::File* filePtr, string fileName, uint32_t signalThreshold, uint32_t integrationTime, uint32_t gain, uint32_t pretrigger, uint32_t decimation)
{
	this->file = filePtr;
	this->fileName = fileName;

	HighFive::Group header = this->file->createGroup("Header");
	header.createAttribute("SignalThreshold", signalThreshold);
	header.createAttribute("IntTime", integrationTime);
	header.createAttribute("Gain", gain);
	header.createAttribute("Pretrigger", pretrigger);
	header.createAttribute("Decimation", decimation);
}

void H5Interface::WriteData(vector<vector<uint32_t>> waveform, time_t timestamp, int eventNum)
{
	//HighFive::File file(fileName, HighFive::File::OpenOrCreate);

	HighFive::Group currGroup = this->file->createGroup("Event" + to_string(eventNum));
	currGroup.createAttribute("Timestamp", timestamp);
	for (int i = 0; i < waveform.size(); i++)
	{
		currGroup.createDataSet("Channel" + to_string(i), waveform.at(i));
	}
}

void H5Interface::ReadFile(int numEvents, int channels)
{
	HighFive::File file(fileName, HighFive::File::ReadOnly);

	for (int i = 0; i < numEvents; i++)
	{
		HighFive::Group currGroup = file.getGroup("Event" + to_string(i));

		time_t timestamp;
		currGroup.getAttribute("Timestamp").read(timestamp);

		cout << "Event " << i << "(" << timestamp << ") : " << endl;
		for (int j = 0; j < channels; j++)
		{
			vector<uint32_t> waveform;
			currGroup.getDataSet("Channel" + to_string(j)).read(waveform);
			cout << "Channel " << j << " Waveform: " << endl;
			for (int k = 0; k < waveform.size(); k++)
			{
				cout << waveform.at(k) << " ";
			}

			cout << endl;
		}
	}
}
