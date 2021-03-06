/*
 * Copyright (c) 2003-2008, John Wiegley.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * - Neither the name of New Artisans LLC nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _TIMES_H
#define _TIMES_H

namespace ledger {

DECLARE_EXCEPTION(datetime_error, std::runtime_error);
DECLARE_EXCEPTION(date_error, std::runtime_error);

typedef boost::posix_time::ptime	datetime_t;
typedef datetime_t::time_duration_type	time_duration_t;

inline bool is_valid(const datetime_t& moment) {
  return ! moment.is_not_a_date_time();
}

typedef boost::gregorian::date          date_t;
typedef boost::gregorian::date_duration date_duration_t;

inline bool is_valid(const date_t& moment) {
  return ! moment.is_not_a_date();
}

extern const datetime_t& current_time;
extern const date_t&     current_date;
extern int		 current_year;
extern optional<string>  input_date_format;
extern string		 output_date_format;

inline datetime_t parse_datetime(const string& str) {
  return parse_datetime(str.c_str());
}
datetime_t parse_datetime(const char * str);

inline date_t parse_date(const string& str) {
  return parse_date(str.c_str());
}
date_t parse_date(const char * str);

inline std::time_t to_time_t(const ptime& t) 
{ 
  if( t == posix_time::neg_infin ) 
    return 0; 
  else if( t == posix_time::pos_infin ) 
    return LONG_MAX; 
  ptime start(date(1970,1,1)); 
  return (t-start).total_seconds(); 
}

inline string format_datetime(const datetime_t& when)
{
  char buf[256];
  time_t moment = to_time_t(when);
  std::strftime(buf, 255, (output_date_format + " %H:%M:%S").c_str(),
		std::localtime(&moment));
  return buf;
}

inline string format_date(const date_t& when,
			  const optional<string>& format = none)
{
  if (format || ! output_date_format.empty()) {
    char buf[256];
    std::tm moment = gregorian::to_tm(when);
    std::strftime(buf, 255, format ?
		  format->c_str() : output_date_format.c_str(), &moment);
    return buf;
  } else {
    return to_simple_string(when).substr(2);
  }
}

struct interval_t
{
  int	 years;
  int	 months;
  int	 days;
  date_t begin;
  date_t end;

  mutable bool advanced;

  interval_t(int _days = 0, int _months = 0, int _years = 0,
	     const date_t& _begin = date_t(),
	     const date_t& _end   = date_t())
    : years(_years), months(_months), days(_days),
      begin(_begin), end(_end), advanced(false) {
    TRACE_CTOR(interval_t, "int, int, int, const date_t&, const date_t&");
  }
  interval_t(const interval_t& other)
    : years(other.years),
      months(other.months),
      days(other.days),
      begin(other.begin),
      end(other.end),
      advanced(other.advanced) {
    TRACE_CTOR(interval_t, "copy");
  }
  interval_t(const string& desc)
    : years(0), months(0), days(0), begin(), end(), advanced(false) {
    TRACE_CTOR(interval_t, "const string&");
    std::istringstream stream(desc);
    parse(stream);
  }

  ~interval_t() throw() {
    TRACE_DTOR(interval_t);
  }

  operator bool() const {
    return years != 0 || months != 0  || days != 0;
  }

  void   set_start(const date_t& moment) {
    begin = moment;
  }

  date_t first(const optional<date_t>& moment = none) const;
  date_t increment(const date_t&) const;

  void   parse(std::istream& in);
};

} // namespace ledger

#endif // _TIMES_H
