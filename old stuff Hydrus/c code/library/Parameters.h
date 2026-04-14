#pragma once

typedef struct Par
{
	char name[kVstMaxLabelLen];
	float min;
	float max;
	float def;
	float normVal; // current normalized 0 to 1 value
	double val; // current full scale value from min to max
	double storedVal; // current full scale value except used to store conversions

} Par;

typedef struct Preset
{
  float value[NUM_PARAMS];
  char  name[kVstMaxProgNameLen];
} Preset;


Par makePar(char* name, double minVal, double maxVal, double defaultVal)
{
	Par o = {
		.min = minVal,
		.max = maxVal,
		.def = defaultVal,
		.normVal = 0
	};
	strcpy(o.name, name);
	return o;
}

void makeDefaultPatch(Preset* o, Par* parameters, char *presetName)
{
	strcpy((char*) &o->name, presetName);

	for (int i = 0; i < NUM_PARAMS; ++i)
		o->value[i] = map(parameters[i].def, parameters[i].min, parameters[i].max, 0, 1);
}

void makePatch(Preset* o, Par* parameters, char *presetName)
{
	strncpy((char*)&o->name, presetName, kVstMaxProgNameLen);
	for (int i = 0; i < NUM_PARAMS; ++i)
		o->value[i] = parameters[i].normVal;
}
