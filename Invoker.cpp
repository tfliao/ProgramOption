// Copyright (C) 2014-2014 tfliao <tingfuliao@gmail.com>
#include "Invoker.h"
#include <algorithm>
#include <cctype>
using namespace std ;

template<>
bool load<string> ( string& ref, const string& key, const string& value )
{
	ref = value ;
	return true ; 
}


template<>
bool load<bool> ( bool& ref, const string& key, const string& value )
{
	string buffer ;
	istringstream iss ( value ) ;
	iss >> buffer ;

	transform ( buffer.begin(), buffer.end(), buffer.begin(), ::toupper ) ;

	ref = ( buffer == "TRUE" || buffer == "1" ) ;
	return true;
}
