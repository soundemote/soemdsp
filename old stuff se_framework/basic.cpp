#include "basic.h"

using juce::String;
using std::vector;

String convertToAcceptableXMLVariable(String s)
{
  s = s.replaceCharacters("&<\' \"/>", "_______");

  for (const auto & c :{ '0','1','2','3','4','5','6','7','8','9' })
  {
    if (s[0] == (unsigned)c)
    {
      s = "_"+s;
      break;
    }
  }

  return s;
}

const vector<String> TempoOptionsStr
{
	"0/0", "1/4","1/3","1/2","x1",
	"x1.333","1.5x","x2",
	"x2.666","x3","x4",
	"x5.333", "x6", "x8",
	"x10.666","x12","x16",
	"x21.333","x24","x32",
	"x42.666","x48","x64",
	"x85.333","x96","x128",
};

const vector<signature> timeSignatures
{
{ "4/1" , 4.0 },
{ "2/1." , 2.0 * 2.0 / 3.0 },
{ "4/1T" , 2.0 * 3.0 / 4.0 },
{ "2/1" , 2.0 },
{ "1/1." , 1.0 * 2.0 / 3.0 },
{ "2/1T" , 1.0 * 3.0 / 4.0 },
{ "1/1" , 1.0 },
{ "1/2." , 1.0 / 2.0 * 2.0 / 3.0 },
{ "1/1T" , 1.0 / 2.0 * 3.0 / 4.0 },
{ "1/2" , 1.0 / 2.0 },
{ "1/4." , 1.0 / 4.0 * 2.0 / 3.0 },
{ "1/2T" , 1.0 / 4.0 * 3.0 / 4.0 },
{ "1/4" , 1.0 / 4.0 },
{ "1/8." , 1.0 / 8.0 * 2.0 / 3.0 },
{ "1/4T" , 1.0 / 8.0 * 3.0 / 4.0 },
{ "1/8" , 1.0 / 8.0 },
{ "1/16." , 1.0 / 16.0 * 2.0 / 3.0 },
{ "1/8T" , 1.0 / 16.0 * 3.0 / 4.0 },
{ "1/16" , 1.0 / 16.0 },
{ "1/32." , 1.0 / 32.0 * 2.0 / 3.0 },
{ "1/16T" , 1.0 / 32.0 * 3.0 / 4.0 },
{ "1/32" , 1.0 / 32.0 },
{ "1/64." , 1.0 / 64.0 * 2.0 / 3.0 },
{ "1/32T" , 1.0 / 64.0 * 3.0 / 4.0 },
{ "1/64" , 1.0 / 64.0 },
{ "1/128." , 1.0 / 128.0 * 2.0 / 3.0 },
{ "1/64T" , 1.0 / 128.0 * 3.0 / 4.0 },
{ "1/128" , 1.0 / 128.0 },
//{ "4", 4 },
//{ "2 + 1", 1.0 * 2.0/3.0 },
//{ "2 + 2/3", 2.0 * 3.0/4.0 },
//{ "2", 2.0 },
//{ "1 + 1/2", 1.0 * 2.0/3.0 },
//{ "1 + 1/3", 1.0 * 3.0/4.0 },
//{ "1", 1.0 },
//{ "1/2 + 1/4", 1.0/2.0 * 2.0/3.0 },
//{ "1/2 + 1/6", 1.0/2.0 * 3.0/4.0 },
//{ "1/2", 1.0/2.0 },
//{ "1/4 + 1/8", 1.0/4.0 * 2.0/3.0 },
//{ "1/4 + 1/12", 1.0/4.0 * 3.0/4.0 },
//{ "1/4", 1.0/4.0 },
//{ "1/8 + 1/16", 1.0/8.0 * 2.0/3.0 },
//{ "1/8 + 1/24", 1.0/8.0 * 3.0/4.0 },
//{ "1/8", 1.0/8.0 },
//{ "1/16 + 1/32", 1.0/16.0 * 2.0/3.0 },
//{ "1/16 + 1/48", 1.0/16.0 * 3.0/4.0 },
//{ "1/16", 1.0/16.0 },
//{ "1/32 + 1/64", 1.0/32.0 * 2.0/3.0 },
//{ "1/32 + 1/96", 1.0/32.0 * 3.0/4.0 },
//{ "1/32", 1.0/32.0 },
//{ "1/64 + 1/128", 1.0/64.0 * 2.0/3.0 },
//{ "1/64 + 1/192", 1.0/64.0 * 3.0/4.0 },
//{ "1/64", 1.0/64.0 },
//{ "1/128 + 1/256", 1.0/128.0 * 2.0/3.0 },
//{ "1/128 + 1/384", 1.0/128.0 * 3.0/4.0 },
//{ "1/128", 1.0/128.0 }
};

namespace elan {
String valueToStringWithPlusMinusSign(double v, int decimalplaces)
{
	return v > 0 ? "+" + String(v, decimalplaces) : String(v, decimalplaces);
}
String StringFunc0WithX(double v)
{
	return "x"+String((int)v);
}
String StringFunc0WithCapitolX(double v)
{
	return "X"+String(v, 0);
}
String StringFunc3WithX(double v)
{
	return "x"+String(v, 3);
}

String addPlusCharIfPos(const String & s, double v)
{
	return v > 0 ? "+"+s : s;
}

String StringFunc0(double v)
{
	return String((int)v);
}

String StringFunc1(double v)
{
	if (v == floor(v))
		return String(v, 1);

	return String(v, 1);
}

String StringFunc2(double v)
{
	if (v == 0.0)
		return "0";

	if (v == floor(v))
		return String(v, 0);

	return String(v, 2);
}

String StringFunc3(double v)
{
	if (v == floor(v))
		return String(v, 0);

	return String(v, 3);
}

String StringFunc4(double v)
{
	if (v == floor(v))
		return String(v, 1);

	return String(v, 4);
}

String StringFunc5(double v)
{
	if (v == floor(v))
		return String(v, 1);

	return String(v, 5);
}

String percentToStringWith2Decimals(double v)
{
	if (v == floor(v))
		return String(int(v*100))+"%";

	return String(v*100, 2)+"%";
}

String percentToStringWith3Decimals(double v)
{
	if (v == floor(v))
		return String(int(v*100))+"%";

	return String(v*100, 3)+"%";
}

String hertzToStringWithUnitTotal5(double v)
{
	if (v == 0)
		return "0.0 Hz";

	double absVal = std::abs(v);

	if (absVal >= 10000.0)
		return String((int)round(v)) + String(" Hz");
	if (absVal >= 1000.0)
		return String(v, 1) + String(" Hz");
	if (absVal >= 100.0)
		return String(v, 2) + String(" Hz");
	if (absVal >= 10.0)
		return String(v, 3) + String(" Hz");
	if (absVal >= 1.0)
		return String(v, 4) + String(" Hz");

	return String(v, 5) + String(" Hz");
}
String hertzToStringWithUnitTotal5_bipolar(double v)
{
	return addPlusCharIfPos(hertzToStringWithUnitTotal5(v), v);
}
String hertzToStringWithUnitTotal4(double v)
{
	if (v == 0)
		return "0.0 Hz";

	double absVal = std::abs(v);

	if (absVal >= 10000.0)
		return String((int)round(v)) + String(" Hz");
	if (absVal >= 1000.0)
		return String(v, 1) + String(" Hz");
	if (absVal >= 100.0)
		return String(v, 2) + String(" Hz");
	if (absVal >= 10.0)
		return String(v, 3) + String(" Hz");

	return String(v, 4) + String(" Hz");
}
String hertzToStringWithUnitTotal4_bipolar(double v)
{
	return addPlusCharIfPos(hertzToStringWithUnitTotal4(v), v);
}
String hertzToStringWithUnitTotal3(double v)
{
	if (v == 0)
		return "0 Hz";

	double absVal = std::abs(v);

	if (absVal >= 10000.0)
		return String((int)round(v)) + " Hz";
	if (absVal >= 1000.0)
		return String(v, 1) + " Hz";
	if (absVal >= 100.0)
		return String(v, 2) + " Hz";

	return String(v, 3) + " Hz";
}
String hertzToStringWithUnitTotal3_bipolar(double v)
{
	return addPlusCharIfPos(hertzToStringWithUnitTotal3(v), v);
}
String hertzToStringLowpassFilter(double v)
{
	if (v >= 24000 - 1.e-6)
		return "BYPASS";

	return hertzToStringWithUnitTotal5(v);
}
String hertzToStringHighpassFilter(double v)
{
	if (v <= 0.01 + 1.e-6)
		return "BYPASS";

	return hertzToStringWithUnitTotal5(v);
}
String secondsToStringWithUnitTotal4(double v)
{
	if (v == 0)
		return "0.000";
	if (v >= 100.0)
		return String(v, 2) + " s";
	if (v >= 1.0)
		return String(v, 3) + " s";
	if (v >= 0.1)
		return String(1000*v, 1) + " ms";
	if (v >= 0.01)
		return String(1000*v, 2) + " ms";
	if (v >= 0.001)
		return String(1000*v, 3) + " ms";
	if (v >= 0.0001)
		return String(1000*v, 3) + " ms";

	return String(1000*v, 3) + " ms";
}
String semitonesToStringWithUnit2(double v)
{
	if (v == 0)
		return "0 st";

	return addPlusCharIfPos(String(v, 2) + " st", v);
}
String valueToPercentStringWithUnit0(double v)
{
	if (v == 0)
		return "0 %";

	return String(round(v*100)) + " %";
}
String decibelsToStringWithUnit1(double v)
{
	return String(v, 1) + " dB";
}
String decibelsToStringWithUnit2(double v)
{
	return String(v, 2) + " dB";
}
String octavesToStringWithUnit0(double v)
{
	return String((int)v) + " oct";
}
String beatMulToString(double v)
{
	if (v < 0)
		return "x1/"+String(powerOfTwo((int)std::abs(v)));
	else
		return "x"+String(powerOfTwo((int)v));
}
String tempoMulToString(double v)
{
	return TempoOptionsStr[(int)v];
}

String bipolarRateToString(double v)
{
	if (v > 1)
		return "x"+String(v, 2) + " ff";
	if (v == 1)
		return "x1.00 play";
	if (v == 0)
		return "frozen (let it go!)";
	if (v < 0)
		return "x"+String(std::abs(v), 2)+" rw";

	return "x"+String(v, 2) + " slo";
}
String numShroomsToString(double v)
{
	if (v > 1)
		return String(v)+" shrooms";
	if (v == 1)
		return "1 shroom";
	if (v == 0)
		return "1 shroom";
	if (v == -1)
		return "-1 anti shroom";

	return String(v)+" anti shrooms";
}
String invertedBeatsPerSecondToString(double v)
{
	return StringFunc3(1/v) + "beats";
}
String dotLimitToString(double v)
{
	if (v == 1)
		return "dots only";

	return String((int)v);
}
String afterGlowToString(double v)
{
	if (v >= 50)
		return "no decay";

	return StringFunc3(v);
}
String indexToTimeSigName(double v)
{
  size_t index = (size_t)round(clamp(v, 0.0, double(timeSignatures.size()-1)));
  return timeSignatures[index].name;
}
double indexToTimeSigValue(double v)
{
	size_t index = (size_t)round(clamp(v, 0.0, double(timeSignatures.size()-1)));
	return timeSignatures[index].value;
}
} // namespace elan
