#include "jsonHelper.h"

int main()
{
    string str("{\"func\":\"dosomething\", \"param\":{\"A\": 1, \"B\": 2}}");
    string func;
    int a,b;
    jsonHelper::getField(func, "func", str);
    jsonHelper::getField(a, "A", str);
    jsonHelper::getField(b, "B", str);
    return 0;
}