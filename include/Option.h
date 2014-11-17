// Copyright (C) 2014-2014 tfliao <tingfuliao@gmail.com>
#ifndef __OPTION_H__
#define __OPTION_H__

#include "Invoker.h"
#include <string>
using std::string;

namespace program_option {

class ProgramOption ;

/*!
	@brief  Option for ProgramOption, support chaining to set the porperties
	@author tfliao
*/
class Option
{
	friend class ProgramOption;
private:
	string m_long;
	char m_short;
	string m_name;
	string m_desc;
	mutable BaseInvoker* m_invoker;
	string m_group;

	enum Flag {
		FLAG_DEFAULT	= 1, ///< the option is default, using postion to identify it
		FLAG_OPTIONAL   = 2, ///< default option which is optional
		FLAG_ARG_LIST   = 4, ///< default option which accepts all following arguments
		FLAG_NO_ARG	    = 8, ///< the option need no argument
	};
	unsigned int m_flag;

	int m_help_level;
	static const int InvisibleLevel = 999;

	string m_warning;
public:
	inline Option(BaseInvoker* p = NULL)
		: m_long("")
		, m_short(0)
		, m_name("arg")
		, m_desc("")
		, m_invoker(p)
		, m_flag(0)
		, m_help_level(0)
		, m_group("")
	{}
	template<class _T, class _Loader>
	inline Option(_T& ref, _Loader loader)
		: m_long("")
		, m_short(0)
		, m_name("arg")
		, m_desc("")
		, m_invoker(new Invoker<_T, _Loader>(ref, loader))
		, m_flag(0)
		, m_help_level(0)
		, m_group("")
	{}
	inline Option(const Option& rhs)
		: m_long(rhs.m_long)
		, m_short(rhs.m_short)
		, m_name(rhs.m_name)
		, m_desc(rhs.m_desc)
		, m_invoker(rhs.m_invoker)
		, m_flag(rhs.m_flag)
		, m_help_level(rhs.m_help_level)
		, m_group(rhs.m_group)
	{
		rhs.m_invoker = NULL;
	}

	inline ~Option() {
		if (m_invoker) delete m_invoker;
	}

	inline Option& operator= ( const Option& rhs )
	{
		m_long = rhs.m_long;
		m_short = rhs.m_short;
		m_name = rhs.m_name;
		m_desc = rhs.m_desc;
		m_flag = rhs.m_flag;
		m_help_level = rhs.m_help_level;
		m_group = rhs.m_group;

		m_invoker = rhs.m_invoker;
		rhs.m_invoker = NULL;
		return *this;
	}

	inline Option& keys(const string& long_key, char short_key) {
		return (*this).long_key(long_key).short_key(short_key);
	}

	inline Option& long_key(const string& key) {
		m_long = key;
		return  *this;
	}
	inline Option& short_key(char key) {
		m_short = key;
		return  *this;
	}
	inline Option& name(const string& name) {
		m_name = name;
		return *this;
	}
	inline Option& description(const string& desc) {
		m_desc = desc ;
		return *this;
	}
	inline Option& group(const string& group) {
		m_group = group;
		return *this;
	}
	inline Option& invoker(BaseInvoker* p) {
		if (m_invoker) {
			warning("remove previous invoker");
			delete m_invoker;
		}
		m_invoker = p;
		return *this;
	}
	template<class _T, class _Loader>
	inline Option& invoker(_T& ref, _Loader loader) {
		return invoker(new Invoker<_T, _Loader>(ref, loader));
	}

	inline Option& is_default_arg(const string& name) {
		m_flag |= FLAG_DEFAULT;
		m_name = name;
		return *this;
	}
	inline Option& is_optional() {
		m_flag |= FLAG_OPTIONAL;
		return *this;
	}
	inline Option& is_arg_list() {
		m_flag |= FLAG_ARG_LIST ;
		return *this;
	}
	inline Option& invisible() {
		m_help_level = InvisibleLevel;
		return *this;
	}
	inline Option& help_level(int level) {
		m_help_level = level;
		return *this;
	}
	inline Option& no_arg() {
		m_flag |= FLAG_NO_ARG;
		return *this;
	}

	inline const string& warning_msg() { return m_warning; }
private:
	inline void warning(const string& msg) { m_warning += msg + "\n"; }

	bool validate(string& msg) const {
		bool ret = true;
		// 1. short is number or alpha or 0 (not set)
		if (has_short() && !isalpha(m_short) && !isdigit(m_short)) {
			msg += "short key must be number or alpha\n";
			ret = false;
		}
		// 2. long is string match /[0-9A-Za-z][0-9A-Za-z_\-]*/
		for (unsigned int i=0;i<m_long.length();++i) {
			char c = m_long[i];
			if (!(isdigit(c) || isalpha(c) || (i!=0 && (c=='_' || c=='-')))) {
				msg += "long key must be /[\\d\\w][\\d\\w_-]*/";
				ret = false;
				break;
			}
		}
		// 3. long is string match /[0-9A-Za-z][0-9A-Za-z_\-]*/
		for (unsigned int i=0;i<m_group.length();++i) {
			char c = m_group[i];
			if (!(isdigit(c) || isalpha(c) || (i!=0 && (c=='_' || c=='-')))) {
				msg += "long key must be /[\\d\\w][\\d\\w_-]*/";
				ret = false;
				break;
			}
		}
		return ret;
	}

	inline bool check_is_default()   const { return (m_flag & FLAG_DEFAULT) != 0; }
	inline bool check_is_arg_list()   const { return (m_flag & FLAG_ARG_LIST) != 0; }
	inline bool check_is_optional()  const { return (m_flag & FLAG_OPTIONAL) != 0; }
	inline bool check_is_no_arg()	const { return (m_flag & FLAG_NO_ARG) != 0; }
	
	inline bool check_is_invisible() const { return (m_help_level == InvisibleLevel); }
	inline bool check_visible_for(int level) const { return m_help_level <= level; } 

	inline int help_level() const { return m_help_level; }
	inline bool has_long() const { return !m_long.empty(); }
	inline bool has_short() const { return m_short != 0; }
	inline bool has_group() const { return !m_group.empty(); }

	static bool group_cmp(const Option& o1, const Option& o2) {
		return o1.m_group < o2.m_group;
	}

};

}

#endif
