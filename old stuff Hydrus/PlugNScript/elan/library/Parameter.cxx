class Parameter
{
    Parameter(){};

    Parameter(string _name, 
    double _minVal = 0, 
    double _maxVal = 1, 
    double _defaultVal = 0, 
    string _units = "", 
    string _format = ".3", 
    int _steps = 0, 
    string _enums = "")
    {
        // remove underscores
        array<string> x = _name.split("_");
        name = join(x, " ");
        minVal = _minVal;
        maxVal = _maxVal;
        defaultVal = _defaultVal;
        format = _format;
        units = _units;
        steps = _steps;
        enums = _enums;
    };

    string name = "label";
    double minVal = 0;
    double maxVal = 1;
    double defaultVal =.5;
    string units = ".3";
    string format = "";
    int steps = 0;
    string enums = "";
};