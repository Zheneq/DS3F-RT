#include "global.h"

struct RecHead
{
	int idx, len;
	field *e,*h;
	vector<field*> Records;

	RecHead(int _idx, int _len)
	{
		idx = _idx;
		len = _len;
		e   = NULL;
		h   = NULL;
	}
	void Init()
	{
		e = new field();
		e->Init(len);
		h = new field();
		h->Init(len);
	}
	~RecHead()
	{
		if (e) delete e;
		if (h) delete h;
		for (auto RS : Records)
		{
			if(RS) delete RS;
		}
	}
	RecHead(const RecHead&) = delete;
	int get_len() { return len; }

};

class ObsModule : public Module
{
	vector<RecHead*> RecHeads;
	vector<string> RecHeadNames;
public:
	~ObsModule();
	virtual void Init() override;
	virtual void Tick(int time) override;
	virtual void PostCalc(int time, StopType stoptype) override;

	void AddObserver(int x, const char* name);
};