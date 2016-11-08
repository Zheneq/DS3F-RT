#include "module_specstats.h"

void SSModule::Init()
{
	WidestSpecTimestamp = -1;
	WidestSpecWidth = 0;
}

void SSModule::Tick(int time)
{
	info->e->Fourier();
	// info->h->Fourier();

	integral[0] = 0;
	// ������� ����
	for (int j = 0; j < info->nz - 1; j++)
	{
		integral[0] += info->e->data[j + 1] * info->e->data[j + 1];
	}

	// ������ ������� � ��������
	integral[1] = 0;
	int w = -1;
	for (int j = 0; j < info->nz / 2 + 1; ++j)
	{
		integral[1] += info->e->spec[j];
		if (info->e->spec[j] >= SPEC_LEVEL) w = j;
	}
	integ = integral[0] - integral[1] + 0.5*info->e->spec[0];
	if (w > WidestSpecWidth)
	{
		WidestSpecWidth = w;
		WidestSpecTimestamp = time;
	}

	// ���� �������� ������
	// info->e->Fourier(true);
}