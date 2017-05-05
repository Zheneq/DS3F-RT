#include "global.h"

#define _USE_MATH_DEFINES
#include <math.h>

INIReader *config = NULL;
EnvInfo *info = NULL;
char BaseFolder[512];

// TODO: inline??
double realxe(int i) { return i*info->hs - info->lz0; }
double realxh(int i) { return realxe(i) + info->hs * .5; }
double realte(int i) { return i*info->ts; }
double realth(int i) { return realte(i) + info->ts * .5; }
double realspec(int i) { return 2 * M_PI * i / info->lz; }
double realspect(int i) { return 2 * M_PI * i / info->lt; }
int idxxe(double x) { return (int)(round((x + info->lz0) / info->hs)); }

inline double DielCond(int x, int t)
{
	register double rx = realxe(x);

	// ���� �� ��� ����, ����� �������
	if ((rx<info->Layers[0].left - info->hs) || (rx>info->Layers[info->LayerCount - 1].right + info->hs))
		return 1;
	for (int i = 0; i < info->LayerCount; ++i)
	{
		// ���� �� ������ �� �������
		if (abs(rx - info->Layers[i].left) < info->hs*.5 || abs(rx - info->Layers[i].right) < info->hs*.5)
			return .5*(DielCond(x - 1, t) + DielCond(x + 1, t));
		// ���� �� ������ � �������
		if (rx > info->Layers[i].left && rx < info->Layers[i].right)
			return info->Layers[i].dc;
	}
	// ���� �� ������ �� ������
	return 1;
}

double Energy(int x, int t)
{
	return .5 * (DielCond(x, t) * info->e->data[x] * info->e->data[x] + info->h->data[x] * info->h->data[x]);
}

double ElecEnergy(int x, int t)
{
	return .5 * DielCond(x, t) * info->e->data[x] * info->e->data[x];
}

FILE *GetFile(const char *name)
{
	char DumpPath[512];
	sprintf_s(DumpPath, "%s/%s.txt", BaseFolder, name);
	FILE *f = fopen(DumpPath, "wt");

	if (!f)
	{
		char Error[544];
		sprintf_s(Error, "Cannot create file: %s", DumpPath);
		throw(Error);
	}

	return f;
}