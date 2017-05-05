#define _USE_MATH_DEFINES
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <cmath>
#include <windows.h>
#include <direct.h>
#include <string>
#include <ctime>
#include "global.h"

// Modules
#include "module_main.h"
#include "module_nrg.h"
#include "module_refl_trans.h"
#include "module_specstats.h"
#include "module_observer.h"
#include "module_inv.h"

using namespace std;

FILE *LogFile = NULL;
std::list<Module*> modules;
field *e_cache = NULL, *h_cache = NULL;
ObsModule *obs = NULL;
int obs1 = 0, obs2 = 0;
default_random_engine *gen = NULL;

void Log(const char *msg, bool bToConsole)
{
	if (!LogFile) throw("No log file!");

	fprintf(LogFile, "%s\n", msg);
	printf("\n\tLog:\t%s\n", msg);
}

void Load(int argc, char **argv)
{
	const unsigned len = 512;
	char Path[len];

	// Если не задан файл конфигурации, ищем дефолтный
	if (argc == 1)
	{
		strcpy_s(Path, argv[0]); //strcpy_s(Path, strlen(argv[0]), argv[0]);
		for (int i = strlen(argv[0]); i; --i) if (Path[i] == '\\' || Path[i] == '/') { Path[i] = '\0'; break; }
		strcat_s(Path, "\\default.ini");
	}
	else
	{
		strcpy_s(Path, argv[1]);
	}
	printf("%s\n", Path);

	if (config) delete config;
	config = new INIReader(Path);

	if (config->ParseError() < 0) {
		throw("Can't load ini file");
	}
	
	//std::cout << "Config loaded from 'test.ini': version="
	//	<< reader.GetInteger("protocol", "version", -1) << ", name="
	//	<< reader.Get("user", "name", "UNKNOWN") << ", email="
	//	<< reader.Get("user", "email", "UNKNOWN") << ", pi="
	//	<< reader.GetReal("user", "pi", -1) << ", active="
	//	<< reader.GetBoolean("user", "active", true) << "\n";

	// As we have config now, we can create a temprary info object
	info = new EnvInfo();

	// Output folders
	if (argc >= 4)
	{
		strcpy_s(BaseFolder, argv[3]);
		_mkdir(BaseFolder);
	}
	else
	{
		throw "Missing parameter 3 (Name).\n";
	}

	// Init logger
	char *fn = new char[256];
	sprintf_s(fn, 256, "%s/log.txt", BaseFolder);
	fopen_s(&LogFile, fn, "w");
	delete[] fn;

	// Модули

	// for (auto m : modules)
	//	delete m;
	// modules.clear();

//	obs = new ObsModule;

	// obs->AddObserver(idxxe(-100));
	//obs->AddObserver(idxxe(-250));
	//obs->AddObserver(idxxe(-500));
	//obs->AddObserver(idxxe(130));
	//obs->AddObserver(idxxe(530));
	//obs->AddObserver(idxxe(780));

	modules.push_back(new MainModule);
//	modules.push_back(new NRGModule);
//	modules.push_back(new InvModule);
//	modules.push_back(new RTModule);
//	modules.push_back(new SSModule);
//	modules.push_back(obs);

}

void UnLoad()
{
	Log("Exiting.");
	fclose(LogFile);
}

void Init(int argc, char **argv)
{
	double freq;
	if (argc >= 3)
	{
		int res = sscanf_s(argv[2], "%lf", &freq);
		if (!res)
		{
			throw "Invalid parameter 2. Frequency (real) expected.";
		}
	}
	else
	{
		throw "Missing parameter 2. Frequency (real) expected.";
	}
	int PeriodCount = config->GetInteger("Data", "PeriodCount", 1);
	double duration = M_PI * PeriodCount / freq;
	double left = -4.0 * duration;
	double right = -left;

	// Создаём новую среду
	if (info) delete info;
	info = new EnvInfo(right);
	info->cf = freq;

	// Настраиваем наблюдателей
	obs1 = idxxe(left);
	obs2 = idxxe(info->Layers[info->LayerCount - 1].right + 10.0);
	if (obs2 >= info->nz || obs1 < 0)
	{
		throw "Medium is not long enough.";
	}

	char msg[256];
	sprintf_s(msg, "periods: %d, cf: %.3lf, structure at: %.1lf, observers: %.1lf, %.1lf, medium: %.1lf, %.1lf", PeriodCount, info->cf, right, realxe(obs1), realxe(obs2), realxe(0), realxe(info->nz));
	Log(msg);

	// Удаляем старый модуль наблюдателей и создаём новый
	modules.remove(obs);
	if (obs) delete obs;
	obs = new ObsModule;
	modules.push_back(obs);

	obs->AddObserver(obs1, "Refl");
	obs->AddObserver(obs2, "Trans");

	// Начальные значения
	double xe = realxe(0);
	for (int i = 0; i<info->nz; xe += info->hs, ++i)
	{
		info->h->data[i] = info->e->data[i] = exp(-((xe*xe) / (duration*duration)))*cos(info->cf*xe);
	}

}

int main(int argc, char **argv)
{
	try
	{
		gen = new default_random_engine((unsigned int)time(0));

		Load(argc, argv);

		{
			Init(argc, argv);
			for (auto m : modules)
				m->Init();

			///////////////////////////////

			char *m = new char[128];
			Log("Structure");
			for (int i = 0; i < info->LayerCount; ++i)
			{
				sprintf_s(m, 128, info->DumpPattern, info->Layers[i].right - info->Layers[i].left, info->Layers[i].dc);
				Log(m);
			}
			Log("Structure end");

			field* dp = new field;
			dp->Init(info->nz, FFTW_ESTIMATE);
			for (int i = 0; i < dp->GetLen(); ++i)
			{
				dp->data[i] = DielCond(i);
			}

			FILE *f = GetFile("DielCond");
			dp->DumpFullPrecision(f, NULL);
			fclose(f);

			///////////////////////////////
			StopType ST = ST_Time;

			// Просчёт
			int time;
			try
			{
				for (time = 1; time < info->nt; ++time)
				{
					printf("\r%d/%d          ", time, info->nt - 1);
					for (auto m : modules)
						m->Tick(time);

					if (time % 10000 == 1)
					{
						fflush(LogFile);
					}
				}
				printf("\n");
			}
			catch (EX_CalcComplete exception)
			{
				// One of the modules signals that calculation is complete
				ST = ST_Complete;
			}
			for (auto m : modules)
				m->PostCalc(time, ST);
		}
	}
	catch (char *error)
	{
		cerr << error << endl;
		UnLoad();
		return 1;
	}
	UnLoad();

	return 0;
}