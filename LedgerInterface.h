//
//  LedgerInterface.h
//  MoneyWise
//
//  Created by John Wiegley on Sun Jul 03 2005.
//  Copyright (c) 2005 New Artisans LLC. All rights reserved.
//

#include <ledger.h>

#define PERIOD_NONE	 0
#define PERIOD_DAILY	 1
#define PERIOD_WEEKLY	 2
#define PERIOD_MONTHLY	 3
#define PERIOD_QUARTERLY 4
#define PERIOD_YEARLY	 5

#define OPTION_COLLAPSED  0
#define OPTION_RELATED	  1
#define OPTION_BUDGET	  2
#define OPTION_BY_WEEKDAY 3
#define OPTION_SUBTOTALED 4
#define OPTION_BY_PAYEE	  5

#define REPORT_COMMODITY  0
#define REPORT_MARKET     1
#define REPORT_BASIS      2
#define REPORT_CUSTOM     3

#define REPORT_TOTALS     100
#define REPORT_AVERAGE    101
#define REPORT_DEVIATION  102

class ledger_interface
{
  ledger::session_t session;

public:
  ledger_interface();
  ~ledger_interface();

  void set_report_period(int period);
  void set_query_option(int option, bool enable);
  void set_report_type(int type, bool show_revalued   = false,
                       const std::string& amount_expr = "a",
                       const std::string& total_expr  = "O");

  void perform_query
    (ledger::journal_t * journal,
     ledger::acct_handler_ptr accounts_functor,
     ledger::xact_handler_ptr entries_functor);

  void clear_query() {
    session.clean_all();
  }
};
