#pragma once
#include "global.h"

// Forward declaration
struct Layer;
class field;

class EnvInfo
{
private:

public:
	double lz;  // ����� �����
	double lt;  // ����� ��������� 
	double lz0; // ����� ������������ �����
	int    nz;  // ����� ����� ����� �� �����
	int    nt;  // ����� ����� ����� �� �������
	double hs,  // ���� ����� �� �����
		ts;  // ���� ����� �� �������

	double cf;  // ������� ������� ��������� ������ (carrier frequency)
	double a;   // ������������ ��������

	// ��������� ������� ��������������� �������������
	double DP[2];
	double DPMaxDivergenceRel;

	Layer *Layers; // ������� �������
	int LayerCount; // ����� �������
	double LayerWidth[2]; // ������ ������� (����������)
	double LayerWidthMaxDivergenceRel;

	double eps; // ��������
	char DumpPattern[64];

	int TimeStamp; // ���, �� ������� ����� ������������� ������������� �������

	double StopEnergyCondition;
	double TotalEnergy, TotalElecEnergy;

	field *e, *h, *e_cache, *h_cache;

	EnvInfo(double _StructureLeftEdge = 10000.0);
};