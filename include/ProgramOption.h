// Copyright (C) 2014-2014 tfliao <tingfuliao@gmail.com>
#ifndef __PROGRAMOPTION_H__
#define __PROGRAMOPTION_H__

#include "Option.h"
#include "cstdlib"
#include <iostream>
#include <string>
#include <vector>
#include <set>

using std::string ;

namespace program_option {

class ProgramOption
{
private:
	string m_progname;
	string m_desc;
	string m_group;

	string m_errormsg;

	std::vector<Option> m_default_options;
	std::vector<Option> m_options;

	/// checkers
	std::set<string> m_long_opts;
	std::set<char> m_short_opts;

	struct DisplayConf
	{
		int max_opt_width;
		bool has_options;
		int max_def_width;
	};

	// parsing
	int m_idx_def;
	bool m_use_next;

	enum Flags {
		OPTION_IN_END = 0, // display [option] after default arguments
	};
	unsigned int m_flag;

public:
	ProgramOption(const string& progname, const string& desc = "")
		: m_progname(progname)
		, m_desc(desc)
	{ reset(); }

	bool addOption(const Option& option) ;

	bool parse (int argc, char** argv) ;
	string usage(int level = 0) const ;
	void reset () ;
	const string& getError() const;
	bool setGroup(const string& group);

	BaseInvoker* invoke_help(std::ostream& os, int level = 0) const ;
	BaseInvoker* invoke_set_group() ;

	void setFlag(int flag, bool on = true);

private:
	bool parseDefault(const string& opt);
	bool parseLong(const string& opt, const char* next);
	bool parseShort(const string& opt, const char* next);
	const Option* findOption(const string& long_key, char short_key) const;

	void exitError (const string& msg) ;
	bool setError (const string& msg) ;

	void appendDesc(std::ostream& os, const string& first_line, const string& desc) const;
	bool testFlag(int flag) const;

	void analysisDisplayConf(DisplayConf& conf, int level = 0) const;
};

}

#endif