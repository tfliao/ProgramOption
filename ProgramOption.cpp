// Copyright (C) 2014-2014 tfliao <tingfuliao@gmail.com>
#include "ProgramOption.h"
#include <iostream>
#include <sstream>
#include <cstdio>
using namespace std;

#pragma warning( disable : 4996 )

namespace program_option {

bool ProgramOption::addOption( const Option& option )
{
	string msg ;
	if (!option.validate(msg)) {
		return setError(msg);
	}

	if (option.m_short != 0) {
		if (m_short_opts.count(option.m_short) != 0) {
			return setError("duplicate short key");
		}
		m_short_opts.insert(option.m_short);
	}

	if (!option.m_long.empty()) {
		if (m_long_opts.count(option.m_long) != 0) {
			return setError("duplicate long key");
		}
		m_long_opts.insert(option.m_long);
	}
	
	if (option.check_is_default() && m_default_options.size() > 0) {
		Option& last_option = m_default_options.back();
		if (last_option.check_is_arg_list()) {
			return setError("no more default after arg list");
		}
		if (last_option.check_is_optional() && !option.check_is_optional()) {
			return setError("default & optional should be successive");
		}
	}
	
	if (option.check_is_default()) {
		m_default_options.push_back(option);
		int width = (option.m_desc.empty()? 0: option.m_name.length());
		m_max_def_width = max(m_max_def_width, width);
	} else {
		m_options.push_back(option);
		int opt_width = 0;
		if (option.has_long()) opt_width += 2 + option.m_long.length();
		if (option.has_short()) opt_width += 2 + 2;
		if (!option.check_is_no_arg()) opt_width += 1 + option.m_name.length();
		if (option.check_is_invisible()) opt_width = 0;
		m_max_opt_width = max(m_max_opt_width, opt_width);

		if (!option.check_is_invisible()) {
			m_show_options ++ ;
		}

	}

	return true;
}

bool ProgramOption::parse( int argc, char** argv )
{
	m_idx_def = -1;
	for (int i=1;i<argc;++i) {
		const char* p = argv[i];
		int cnt = 0;
		while(*p == '-' && cnt < 2) { ++p; ++cnt; }

		string opt = p;
		char* next = NULL;
		if (i+1 < argc) next = argv[i+1];

		switch(cnt) {
			case 0: // default option
				if (!parseDefault(opt)) return false;
				break;
			case 1: // short option
				m_use_next = false;
				if (!parseShort(opt, next)) return false;
				if (m_use_next) ++i;
				break;
			case 2: // long option
				m_use_next = false;
				if (!parseLong(opt, next)) return false;
				if (m_use_next) ++i;
				break;
		}
	}
	
	if (m_idx_def+1 < m_default_options.size()) {
		const Option& opt = m_default_options[m_idx_def+1];
		if (!opt.check_is_optional()) {
			string losts ;
			for (int i=m_idx_def+1;i<m_default_options.size();++i) {
				const Option& o = m_default_options[i];
				if (o.check_is_optional()) break;
				losts += (losts.empty()?"":", ") + o.m_name;
			}
			return setError("arguments " + losts + " not specified");
		}
	}

	return true;
}

bool ProgramOption::parseDefault( const string& value )
{
	if (m_idx_def + 1 < m_default_options.size()) m_idx_def ++;
	else if (!m_default_options.back().check_is_arg_list())
		return setError("too many arguments");

	const Option& option = m_default_options[m_idx_def];
	if ( !(*option.m_invoker) (option.m_name, value) ) {
		return setError("Bad value \"" + value + "\" for \"" + option.m_name + "\"");
	}
	return true;
}

bool ProgramOption::parseLong( const string& opt, const char* next )
{
	int p = opt.find_first_of('=');
	
	string key;
	if (p != string::npos) {
		key = opt.substr(0, p);
	} else {
		key = opt;
	}
	const Option* option = findOption(key, 0);
	if (NULL == option) {
		return setError("Unknown key \"" + key + "\"");
	}

	string value;
	if (!option->check_is_no_arg()) {
		if (p != string::npos) {
			value = opt.substr(p+1);
		} else if (next != NULL) {
			value = next;
			m_use_next = true;
		} else {
			return setError("lost argument for \"" + key + "\"");
		}
	}
	if ( !(*option->m_invoker) (key, value) ) {
		return setError("Bad value \"" + value + "\" for \"" + key + "\"");
	}
	return true;
}

bool ProgramOption::parseShort( const string& opt, const char* next )
{
	for(int i=0;i<opt.length();++i) {
		string key = opt.substr(i, 1);
		const Option* option = findOption("", key[0]);
		if (NULL == option) {
			return setError("Unknown key \"" + opt.substr(i,1) + "\"");
		}
		string value;
		if (!option->check_is_no_arg()) {
			if (opt[i+1] == '=') {
				value = opt.substr(i+2);
			} else if (opt[i+1] == 0 && next != NULL) { 
				value = next;
				m_use_next = true;
			} else {
				return setError("lost argument of \"" + key + "\"");
			}
			i = opt.length();
		}
		if ( !(*option->m_invoker) (key, value) ) {
			return setError("Bad value \"" + value + "\" for \"" + key + "\"");
		}
	}
	return true;
}

const Option* ProgramOption::findOption( const string& long_key, char short_key )
{
	for (int i=0;i<m_options.size();++i) {
		const Option& opt = m_options[i];
		if ( (opt.m_long == long_key && !long_key.empty() ) ||
			 (opt.m_short == short_key && short_key != 0 ) ) {
			return &opt;
		}
	}
	return NULL;
}

void ProgramOption::appendDesc(ostream& os, const string& first_line, const string& desc) const
{
	istringstream iss(desc);
	string line;
	bool first = true;
	int width = first_line.length();
	while (getline(iss, line)) {
		if (first) {
			os << first_line << line << endl;
			first = false;
		} else {
			os << string(width, ' ') << line << endl;
		}
	}
}

string ProgramOption::usage() const
{
	ostringstream oss ;
	oss << "Usage: " << m_progname;
	oss << (m_show_options > 0 && !testFlag(OPTION_IN_END)?" [options]":"") ;
	for (unsigned int i=0;i<m_default_options.size();++i) {
		const Option& opt = m_default_options[i];
		oss << " "
			<< (opt.check_is_optional() ? "[": "")
			<< opt.m_name
			<< (opt.check_is_arg_list() ? " ... ": "") 
			<< (opt.check_is_optional() ? "]": "") ;
	}
	oss << (m_show_options > 0 && testFlag(OPTION_IN_END)?" [options]":"") ;
	oss << endl;
	if (!m_desc.empty()) {
		oss << m_desc << endl << endl;
	}

	for (unsigned int i=0;i<m_default_options.size();++i) {
		const Option& opt = m_default_options[i];
		if (!opt.m_desc.empty()) {
			char buf[64];
			sprintf(buf, "  %-*s: ", m_max_def_width, opt.m_name.c_str());
			appendDesc(oss, buf, opt.m_desc);
		}
	}
	if (m_show_options > 0) {
		oss << "options: " << endl;
	}
	for (unsigned int i=0;i<m_options.size();++i) {
		const Option& opt = m_options[i];
		if (opt.check_is_invisible()) continue;

		char buf[64];
		if (opt.has_long() && opt.has_short()) 
			sprintf(buf, "--%s, -%c", opt.m_long.c_str(), opt.m_short);
		else if (!opt.has_long() && opt.has_short()) 
			sprintf(buf, " -%c", opt.m_short);
		else if (opt.has_long() && !opt.has_short()) 
			sprintf(buf, "--%s", opt.m_long.c_str());
		string sbuf = buf;
		if (!opt.check_is_no_arg()) sbuf += " " + opt.m_name;
		
		sprintf(buf, "  %-*s  ", m_max_opt_width, sbuf.c_str());
		appendDesc(oss, buf, opt.m_desc);
	}

	return oss.str() ;
}

void ProgramOption::reset()
{

}

void ProgramOption::exitError( const string& msg )
{
	cerr << msg << endl;
	exit(1);
}

bool ProgramOption::setError( const string& msg )
{
	m_errormsg = msg ;
	return false;
}

const string& ProgramOption::getError() const
{
	return m_errormsg;
}

class InvokeHelp : public BaseInvoker
{
	const ProgramOption& m_po ;
	ostream& m_os;
public:
	InvokeHelp(const ProgramOption& po, ostream& os) : m_po(po), m_os(os) {}
	bool operator()(const string& key, const string& value) { m_os << m_po.usage() ; exit(0); return true ; }
};

BaseInvoker* ProgramOption::invoke_help( ostream& os ) const 
{
	return new InvokeHelp(*this, os);
}

void ProgramOption::setFlag(int flag, bool on)
{
	if (on) {
		m_flag |= (1 << flag);
	} else {
		m_flag &= ~(1 << flag);
	}
}

bool ProgramOption::testFlag(int flag) const
{
	return (m_flag & (1 << flag)) != 0;
}

}

