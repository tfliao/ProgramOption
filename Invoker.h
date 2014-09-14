// Copyright (C) 2014-2014 tfliao <tingfuliao@gmail.com>
#ifndef INVOKER_H
#define INVOKER_H

#include <sstream>
#include <string>
#include <limits>
using std::string ;


namespace program_option {

/*!
	@brief  base class of Invoker
	@author T.F. Liao
*/
class BaseInvoker
{
public:
	/*!
		@brief  prototype of functor that load value
		@author T.F. Liao
		@param  key [in] key of variable set
		@param  value [in] value of variable set
		@return load successful or not
		in common case, key is not necessary, but there may be some situation need it
		so we keep this field
	*/
	virtual bool operator()(const string& key, const string& value) = 0;
};

/*!
	@brief  template implementation with storage type and loader
	@author T.F. Liao
*/
template<class _T, class _Loader>
class Invoker : public BaseInvoker
{
private:
	_T& m_ref;
	_Loader m_loader ;
public:
	/*!
		@brief  constructor with reference to storage and loader
		@author T.F. Liao
		@param  value [in] reference to storage
		@param  loader [in] loader to handle the type of variable
	*/
	Invoker ( _T& value, _Loader loader ) : m_ref(value), m_loader(loader) {}

	/*!
		@brief  functor that load value
		@author T.F. Liao
		@param  key [in] key of variable set
		@param  value [in] value of variable set
		@return load successful or not
	*/
	bool operator()(const string& key, const string& value) { return m_loader (m_ref, key, value); }
};


/*!
	@brief  a template implementation of loader
	@author T.F. Liao
	@param  ref [out]   loaded value to store in
	@param  key [in] key of variable set
	@param  value [in] value of variable set
	@return load successful or not
	the implementation is convert string to the type with stringstream
	and check if any chars not used.
*/
template<class _T>
bool load ( _T& ref, const string& key, const string& value )
{
	std::istringstream iss(value);
	if ( std::numeric_limits<_T>::is_integer ) {
		string prefix = value.substr(0,2);
		if ( prefix == "0x" ) { // hex
			iss.ignore(2) ;
			iss >> std::hex ;
		} else if ( value.length() > 1 && prefix[0] == '0' ) { // oct
			iss.ignore(1) ;
			iss >> std::oct ;
		}
	}

	iss >> ref ;

	if ( !iss || iss.rdbuf()->in_avail() != 0 )  
		return false;
	return true;
}
/*!
	@brief  specialization implementation of string type
	@author T.F. Liao
	copy the value to ref
*/
template<>
bool load<string> ( string& ref, const string& key, const string& value ) ;

/*!
	@brief  specialization implementation of boolean type
	@author T.F. Liao
	ref = true if value is neither "true" (case insensitive) or "1"
*/
template<>
bool load<bool> ( bool& ref, const string& key, const string& value ) ;


template<class _Container>
bool load2 (_Container& ref, const string& key, const string& value)
{
	typename _Container::value_type v ;
	if (!load(v, key, value)) return false;
	ref.push_back(v);
	return true;
}

/*!
	@brief  special implementation of loader
	@author T.F. Liao
	construct with a reference ref and a value, once invoked, set the value to ref
*/
template<class _T> 
class ValueSetter : public BaseInvoker
{
	_T& m_ref;
	_T m_value ;
public:
	ValueSetter ( _T& ref, const _T& value ) : m_ref(ref), m_value(value) {}
	bool operator()(const string& key, const string& value) { m_ref = m_value ; return true ; }
};

/*!
	@brief  special Invoker, call a method without parameter
	@author T.F. Liao
	construct with a function pointer, once invoked, call the function
*/
template<class _Callable>
class MethodCaller : public BaseInvoker
{
	_Callable f;
public:
	MethodCaller(_Callable c) : f(c) {}
	bool operator()(const string&, const string&) { (*f)(); return true; }
};

/*!
	@brief  new a MethodCaller without specifiy the type of method 
	@author T.F. Liao
*/
template<class _Callable>
MethodCaller<_Callable>* newMethodCaller(_Callable c)
{
	return new MethodCaller<_Callable>(c);
}

}

#endif
