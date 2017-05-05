#include "module_observer.h"
#include <string>

using namespace std;

void ObsModule::Init()
{
	for (auto rc : RecHeads)
		rc->Init();
	Tick(0);
}

void ObsModule::AddObserver(int x, const char* name)
{
	RecHeads.push_back(new RecHead(x, info->nt));
	RecHeadNames.push_back(string(name));
	//RecHeads.back()->Init();

	char *msg = new char[256];
	sprintf_s(msg, 256, "Observer %02d (%s) at %f", RecHeads.size() - 1, RecHeadNames.back().c_str(), realxe(x));
	Log(msg);
	delete[] msg;
}

ObsModule::~ObsModule()
{
	for (auto RH : RecHeads)
	{
		if (RH) delete RH;
	}
}

void ObsModule::Tick(int time)
{
	for (auto& RC : RecHeads)
	{
		RC->e->data[time] = info->e->data[RC->idx];
		RC->h->data[time] = info->h->data[RC->idx];
	}
}

void ObsModule::PostCalc(int time, StopType stoptype)
{
	if (stoptype == ST_Time) return;

	// Dump all records into single file and reset recheads
	for (unsigned int i = 0; i < RecHeads.size(); ++i)
	{
		// FILE *f = GetFile(RecHeadNames[i].c_str());
		double TotalNRG = 0.0, ElecNRG = 0.0;
		for (int j = 0; j < RecHeads[i]->get_len(); ++j)
		{
			TotalNRG += .5 * (RecHeads[i]->e->data[j] * RecHeads[i]->e->data[j] + RecHeads[i]->h->data[j] * RecHeads[i]->h->data[j]);
			ElecNRG += .5 *  RecHeads[i]->e->data[j] * RecHeads[i]->e->data[j];
		}

		double ElecCoef = ElecNRG / info->TotalElecEnergy;
		double TotalCoef = TotalNRG / info->TotalEnergy;
		// fprintf(ElecCoef)
		char msg[256];
		sprintf_s(msg, 256, "Coef (e/t): %s: %.4lf/%.4lf", RecHeadNames[i].c_str(), ElecCoef, TotalCoef);
		Log(msg);
		// fclose(f);

		char name[256];
		sprintf_s(name, "RESULT_ELEC_%s", RecHeadNames[i].c_str());

		FILE *f = GetFile(name);
		fprintf(f, "%.4lf %.4lf", info->cf, ElecCoef);
		fclose(f);

		sprintf_s(name, "RESULT_TOTAL_%s", RecHeadNames[i].c_str());
		f = GetFile(name);
		fprintf(f, "%.4lf %.4lf", info->cf, TotalCoef);
		fclose(f);
	}

}