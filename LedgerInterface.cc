//
//  JournalController.mm
//  MoneyWise
//
//  Created by John Wiegley on Sun Jul 03 2005.
//  Copyright (c) 2005 New Artisans LLC. All rights reserved.
//

#import "LedgerInterface.h"

using namespace ledger;

ledger_interface::ledger_interface()
{
  set_session_context(&session);
  session.current_report.reset(new report_t(session));
}

ledger_interface::~ledger_interface()
{
  set_session_context();
}

void ledger_interface::set_report_period(int period)
{
  switch (period) {
  case PERIOD_NONE:
    config->report_period = "";
    break;
  case PERIOD_DAILY:
    config->report_period = "daily";
    break;
  case PERIOD_WEEKLY:
    config->report_period = "weekly";
    break;
  case PERIOD_MONTHLY:
    config->report_period = "monthly";
    break;
  case PERIOD_QUARTERLY:
    config->report_period = "quarterly";
    break;
  case PERIOD_YEARLY:
    config->report_period = "yearly";
    break;
  }
}

void ledger_interface::set_query_option(int option, bool enable)
{
  switch (option) {
  case OPTION_COLLAPSED:
    config->show_collapsed = enable;
    break;
  case OPTION_RELATED:
    config->show_related = enable;
    break;
  case OPTION_BUDGET:
    if (enable)
      config->budget_flags = BUDGET_BUDGETED;
    else
      config->budget_flags = BUDGET_NO_BUDGET;
    break;
  case OPTION_BY_WEEKDAY:
    config->days_of_the_week = enable;
    break;
  case OPTION_SUBTOTALED:
    config->show_subtotal = enable;
    break;
  case OPTION_BY_PAYEE:
    config->by_payee = enable;
    break;
  }
}

void ledger_interface::set_report_type(int type, bool show_revalued,
                                       const std::string& amount_expr,
                                       const std::string& total_expr)
{
#if 0
  switch (type) {
  case REPORT_COMMODITY:
    config->show_revalued = false;
    config->amount_expr   = "a";
    config->total_expr    = "O";

    base_total_expr = config->total_expr;
    break;

  case REPORT_MARKET:
    config->show_revalued = true;
    config->amount_expr   = "v";
    config->total_expr    = "V";

    base_total_expr = config->total_expr;
    break;

  case REPORT_BASIS:
    config->show_revalued = false;
    config->amount_expr   = "b";
    config->total_expr    = "B";

    base_total_expr = config->total_expr;
    break;

  case REPORT_CUSTOM:
    config->show_revalued = show_revalued;
    config->amount_expr   = amount_expr;
    config->total_expr    = total_expr;

    base_total_expr = config->total_expr;
    break;

  case REPORT_TOTALS:
    config->total_expr = base_total_expr;
    break;
  case REPORT_AVERAGE:
    config->total_expr = std::string("A(") + base_total_expr + ")";
    break;
  case REPORT_DEVIATION:
    config->total_expr = std::string("t-A(") + base_total_expr + ")";
    break;
  }
#endif
}

void ledger_interface::perform_query
  (journal_t * journal,
   acct_handler_ptr accounts_functor,
   xact_handler_ptr entries_functor)
{
#if 0
  // Remove all (possible) previous query results for the given
  // journal
  clear_query(journal);

  // Collect all the revelant transactions
  clear_formatter_ptrs();
  item_handler<xact_t> * formatter
    = config->chain_xact_handlers("r", entries_functor, journal,
				  journal->master, *formatter_ptrs);
  walk_entries(journal->entries, *formatter);
  formatter->flush();

  // Leave clean of the `entries_functor' to the caller
  formatter_ptrs->remove(entries_functor);

  // Sum the account balances
  sum_accounts(*journal->master);
  walk_accounts(*journal->master, *accounts_functor, config->sort_string);
  accounts_functor->flush();

  // Propogate the master account total if there is one
  if (account_has_xdata(*journal->master)) {
    account_xdata_t& xdata = account_xdata(*journal->master);
    if (xdata.total)
      xdata.value = xdata.total;
  }
#endif
}
