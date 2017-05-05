#include "module_main.h"
#include <cmath>
#include "../FFTW/fftw3.h"

void CalcEnergy(int StartIdx, int EndIdx, int Time, double& OutEnergy, double& OutElecEnergy)
{
	OutEnergy = 0.0;
	OutElecEnergy = 0.0;
	for (int j = StartIdx; j < EndIdx; j++)
	{
		OutEnergy += Energy(j, Time);
		OutElecEnergy += ElecEnergy(j, Time);
	}
}

void MainModule::Init()
{
	StructureLeftEdge = config->GetReal("Data", "Left", 0.0);
	DumpFrameStep = (int)config->GetReal("Data", "FrameStep", 1e10); 
	CheckFrameStep = config->GetInteger("Data", "CheckFrameStep", 100);

	CalcEnergy(0, info->nz, 0, info->TotalEnergy, info->TotalElecEnergy);

	printf("Total Elec Energy: %e\n", info->TotalElecEnergy);
}

void MainModule::Tick(int time)
{
	if (time <= 0) return;

	// Check
	if (time % CheckFrameStep == 0)
	{
		Check(time);
	}

	//*
	// Считаем поле
	info->e->data[0] = info->e->data[1];
	for (int j = 0; j < info->nz - 1; j++)
	{
		info->e->data[j + 1] = (info->e->data[j + 1] * DielCond(j + 1, time - 1) - (info->h->data[j + 1] - info->h->data[j])*info->ts / info->hs) / DielCond(j + 1, time);
	}

	info->h->data[info->nz - 1] = info->h->data[info->nz - 2];
	for (int j = 0; j<info->nz - 1; j++)
	{
		info->h->data[j] = info->h->data[j] - (info->e->data[j + 1] - info->e->data[j])*info->ts / info->hs;
	}

	// Dump frames
	if (time % DumpFrameStep == 1)
	{
		char* fn = new char[256];
		sprintf(fn, "e_frame%06d", time);
		FILE *f = GetFile(fn);
		sprintf(fn, "e-spec_frame%06d", time);
		FILE *fs = GetFile(fn);
		info->e->Fourier();
		info->e->Dump(f, fs);
		fclose(f);
		fclose(fs);
		delete fn;
	}
}

void MainModule::PostCalc(int time, StopType stoptype)
{
	// Dump last frame
	char* fn = new char[256];
	sprintf(fn, "e_frame%06d", time);
	FILE *f = GetFile(fn);
	sprintf(fn, "e-spec_frame%06d", time);
	FILE *fs = GetFile(fn);
	info->e->Fourier();
	info->e->Dump(f, fs);
	fclose(f);
	fclose(fs);
	delete fn;

	// Impulse bounds
	int l, r;
	for (int i = 0; i < info->e->GetLen(); ++i)
	{
		if (abs(info->e->data[i]) > 1e-8)
		{
			l = i;
			break;
		}

	}
	for (int i = info->e->GetLen() - 1; i >= 0 ; --i)
	{
		if (abs(info->e->data[i]) > 1e-8)
		{
			r = i;
			break;
		}

	}

	char* msg = new char[256];
	sprintf(msg, "%lf - %lf", realxe(l), realxe(r));
	Log(msg);
	delete msg;
}

void MainModule::Check(int time)
{
	double ENRG, NRG;
	CalcEnergy(obs1, obs2, time, NRG, ENRG);

	ENRG /= info->TotalElecEnergy;
	NRG  /= info->TotalEnergy;

	char msg1[256];
	sprintf(msg1, "%06d:  Energy Inside (e/t): %.4lf%%/%.4lf%%\n", time, ENRG * 100, NRG * 100);
	Log(msg1, false);

//	printf("Stop condition: %e\n", info->StopEnergyCondition);

	if (ENRG < info->StopEnergyCondition && NRG < info->StopEnergyCondition)
	{
		throw EX_CalcComplete();
	}
}