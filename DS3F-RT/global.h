#define _CRT_SECURE_NO_WARNINGS
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <windows.h>
#include <direct.h>
#include <string>
#include <vector>
#include <random>
#include "env.h"
#include "module.h"
#include "field.h"
#include "../Inih/cpp/INIREADER.h"

const double SPEC_LEVEL = .001;

extern INIReader *config;
extern int ExperimentNum;
extern default_random_engine *gen;
extern char BaseFolder[512];

struct Layer
{
	double left, right, dc;
};

// Forward decl
class EnvInfo;

class EX_CalcComplete {};
enum StopType
{
	ST_None,
	ST_Time,
	ST_Complete
};

extern EnvInfo *info;
extern int obs1, obs2;

double realxe(int i);
double realxh(int i);
double realte(int i);
double realth(int i);
double realspec(int i);
double realspect(int i);
int idxxe(double x);

inline double DielCond(int x, int t = 0);

double Energy(int x, int t);

double ElecEnergy(int x, int t);

FILE *GetFile(const char *name);

void Log(const char *msg, bool bToConsole = true);