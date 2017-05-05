#include "module_refl_trans.h"

RTModule::RTModule()
{
	refl = NULL;
	tran = NULL;
}

RTModule::~RTModule()
{
	if (refl) delete refl;
	if (tran) delete tran;
}

void RTModule::Init()
{
	if (refl) delete refl;
	if (tran) delete tran;
	refl = new field();
	refl->Init(info->nz);
	tran = new field();
	tran->Init(info->nz);
}

void RTModule::PostCalc(int time, StopType stoptype)
{
	// �������������
	memcpy(refl->data, info->e->data, sizeof(info->e->data[0]) * info->nz);
	memcpy(tran->data, info->e->data, sizeof(info->e->data[0]) * info->nz);

	int Left = idxxe(info->Layers[0].left) - 1,
		Right = idxxe(info->Layers[info->LayerCount].right) + 1;
	ZeroMemory(&refl->data[Left], info->nz - Left);
	// TODO: ?? ��������
	ZeroMemory(&tran->data[0], Right);

	// ������
	refl->Fourier();
	tran->Fourier();

	// ������������ ����������� � ���������


	Dump();
}

void RTModule::Dump()
{
	refl->Dump("reflected");
	tran->Dump("transmitted");
}