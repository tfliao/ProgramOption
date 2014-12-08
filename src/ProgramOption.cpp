// Copyright (C) 2014-2014 tfliao <tingfuliao@gmail.com>
#include "ProgramOption.h"
#include <iostream>
#include <sstream>
#include <cstdio>
#include <algorithm>
using namespace std;

#pragma warning( disable : 4996 )

namespace program_option {

bool ProgramOption::addOption( const Option& option )
{
	string msg ;
	if (!option.validate(msg)) {
		return setError(msg);
	}

	if (option.has_group()) {
		m_groups.insert(option.m_group);
	}

	if (option.has_short()) {
		std::set<char>& checker = m_short_checker[option.m_group];
		if (checker.count(option.m_short) != 0) {
			ostringstream oss ;
			oss << "duplicate short key ";
			if (option.has_group())
				oss << "[" << option.m_group << "].[" << option.m_short << "]";
			else
				oss << "[" << option.m_short << "]";
			return setError(oss.str());
		}
		checker.insert(option.m_short);
	}

	if (option.has_long()) {
		std::set<string>& checker = m_long_checker[option.m_group];
		if (checker.count(option.m_long) != 0) {
			ostringstream oss ;
			oss << "duplicate long key ";
			if (option.has_group())
				oss << "[" << option.m_group << "].[" << option.m_long << "]";
			else
				oss << "[" << option.m_long << "]";
			return setError(oss.str());
		}
		checker.insert(option.m_long);
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
		if (option.check_is_exist_first()) ++ m_exist_first;
		m_default_options.push_back(option);
	} else {
		m_options.push_back(option);
		stable_sort(m_options.begin(), m_options.end(), Option::group_cmp);
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
				if (m_exist_first) {
					return setError("some necessary option not appear");
				}
				m_use_next = false;
				if (!parseShort(opt, next)) return false;
				if (m_use_next) ++i;
				break;
			case 2: // long option
				if (m_exist_first) {
					return setError("some necessary option not appear");
				}
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
	if (option.check_is_exist_first()) --m_exist_first;
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
			if (i+1 != opt.length()) {
				if (opt[i+1] == '=') {
					value = opt.substr(i+2);
				} else {
					value = opt.substr(i+1);
				}
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

const Option* ProgramOption::findOption( const string& long_key, char short_key ) const
{
	const Option* cand = NULL;
	for (int i=0;i<m_options.size();++i) {
		const Option& opt = m_options[i];
		if ( (opt.m_long == long_key && !long_key.empty() ) ||
			 (opt.m_short == short_key && short_key != 0 ) ) {
			if (opt.m_group == this->m_group) {
				return &opt; // match both group and key
			} else if (!opt.has_group()) {
				cand = &opt; // match key without group
			}
		}
	}
	return cand;
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

string ProgramOption::usage(int level) const
{
	DisplayConf conf;
	analysisDisplayConf(conf, level);

	ostringstream oss ;
	oss << "Usage: " << m_progname;
	oss << (conf.has_options && !testFlag(OPTION_IN_END)?" [options]":"") ;
	for (unsigned int i=0;i<m_default_options.size();++i) {
		const Option& opt = m_default_options[i];
		oss << " "
			<< (opt.check_is_optional() ? "[": "")
			<< opt.m_name
			<< (opt.check_is_arg_list() ? " ... ": "")
			<< (opt.check_is_optional() ? "]": "") ;
	}
	oss << (conf.has_options && testFlag(OPTION_IN_END)?" [options]":"") ;
	oss << endl;
	if (!m_desc.empty()) {
		oss << m_desc << endl << endl;
	}

	for (unsigned int i=0;i<m_default_options.size();++i) {
		const Option& opt = m_default_options[i];
		if (!opt.m_desc.empty()) {
			char buf[64];
			sprintf(buf, "  %-*s: ", conf.max_def_width, opt.m_name.c_str());
			appendDesc(oss, buf, opt.m_desc);
		}
	}
	if (conf.has_options) {
		oss << "options: " << endl;
	}
	string last_group ;
	for (unsigned int i=0;i<m_options.size();++i) {
		const Option& opt = m_options[i];
		if (!opt.check_visible_for(level)) continue;

		if (opt.m_group != last_group) {
			last_group = opt.m_group;
			oss << "[" << last_group << "]:" << endl;
		}

		char buf[64];
		if (opt.has_long() && opt.has_short())
			sprintf(buf, "--%s, -%c", opt.m_long.c_str(), opt.m_short);
		else if (!opt.has_long() && opt.has_short())
			sprintf(buf, " -%c", opt.m_short);
		else if (opt.has_long() && !opt.has_short())
			sprintf(buf, "--%s", opt.m_long.c_str());
		string sbuf = buf;
		if (!opt.check_is_no_arg()) sbuf += " " + opt.m_name;

		sprintf(buf, "  %-*s  ", conf.max_opt_width, sbuf.c_str());
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
	int m_level;
public:
	InvokeHelp(const ProgramOption& po, ostream& os, int level) : m_po(po), m_os(os), m_level(level) {}
	bool operator()(const string& key, const string& value) { m_os << m_po.usage(m_level) ; exit(0); return true ; }
};

BaseInvoker* ProgramOption::invoke_help( ostream& os, int level ) const
{
	return new InvokeHelp(*this, os, level);
}

class GroupSetter : public BaseInvoker
{
	ProgramOption& m_po;
public:
	GroupSetter(ProgramOption& po) : m_po(po) {}
	bool operator()(const string& key, const string& value) { return m_po.setGroup(value); }
};

BaseInvoker* ProgramOption::invoke_set_group()
{
	return new GroupSetter(*this);
}

bool ProgramOption::setGroup(const string& group)
{
	if (!m_groups.count(group))
		return setError("Non-existed group");
	m_group = group;
	return true;

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

void ProgramOption::analysisDisplayConf(DisplayConf& conf, int level) const
{
	conf.has_options = false;
	conf.max_opt_width = 0;
	conf.max_def_width = 0;

	for (unsigned int i = 0; i < m_default_options.size(); ++i) {
		const Option& option = m_default_options[i];
		int width = (option.m_desc.empty()? 0: option.m_name.length());
		conf.max_def_width = max(conf.max_def_width, width);
	}

	for (unsigned int i = 0; i < m_options.size(); ++i) {
		const Option& option = m_options[i];
		if (option.help_level() <= level) {
			int width = 0;
			if (option.has_long()) width += 2 + option.m_long.length();
			if (option.has_short()) width += 2 + 2;
			if (!option.check_is_no_arg()) width += 1 + option.m_name.length();
			conf.max_opt_width = max(conf.max_opt_width, width);
			conf.has_options = true;
		}
	}
}

}

