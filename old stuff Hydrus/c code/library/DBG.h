#pragma once

bool LogCreated = false;

void LogB(char *message)
{
	//FILE *file; if (!LogCreated) { 
	//	file = fopen(LOGFILE, "w"); 
	//	LogCreated = true; 
	//} else 
	//file = fopen(LOGFILE, "a"); 
	//if (file == NULL) { 
	//	if (LogCreated) 
	//		LogCreated = false; 
	//	return; 
	//} 
	//else { 
	//	fputs(message, file); 
	//	fclose(file); 
	//} 
	//if (file) fclose(file); 
}

void Log (char *message) {LogB(message);}

#define LOGM(v) LogLn(v); LogLine();

void LogLn (char *message) { 
	LogB(message);
	LogB("\n");
} 

void LogInt(int x)
{
	char str[255];
	sprintf_s(str, 255, "%d", x);
	Log(str);
}

void LogFl(float x)
{
	char str[255];
	sprintf_s(str, 255, "%f", x);
	Log(str);
}

void LogLine()
{
	LogB("\n");
}

void LogMsg(char *message, int x)
{
	LogB(message);
	LogInt(x);
}

#define LINE Break(__FILE__, __LINE__);

void Break(char* file, int line)
{
	LogMsg(file, line);
	LogLine();
}

void LogMsgF(char *message, float x)
{
	LogB(message);
	LogFl(x);
}

#define LOGI(v) LogIL(#v": ", v);

void LogIL(char *message, float x)
{
	LogB(message);
	LogInt(x);
	LogLine();
}

#define LOGF(v) LogFL(#v": ", v);

void LogFL(char *message, float x)
{
	LogB(message);
	LogFl(x);
	LogLine();
}

#define FUNC LogFunc(__func__);

void LogFunc(char *funcname)
{
	LogLine();
	LogLine();
	Log("- - - - - - - - -");
	LogLine();
	Log(funcname);
	LogLine();
	Log("- - - - - - - - -");
	LogLine();
}