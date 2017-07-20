#include <sstream>
using namespace std;

//long to string
string ltos(long l)
{
    ostringstream os;
    os<<l;
    string result;
    istringstream is(os.str());
    is>>result;
    return result;
 }

//int to string
string itos(int i)
{
    ostringstream os;
    os<<i;
    string result;
    istringstream is(os.str());
    is>>result;
    return result;

}

//float to string
string ftos(float f)
{
    ostringstream os;
    os<<f;
    string result;
    istringstream is(os.str());
    is>>result;
    return result;

}

//double to string
string dtos(double d)
{
    ostringstream os;
    os<<d;
    string result;
    istringstream is(os.str());
    is>>result;
    return result;

}
