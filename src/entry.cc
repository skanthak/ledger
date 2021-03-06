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

#include "entry.h"
#include "journal.h"
#include "format.h"
#include "session.h"
#include "report.h"

namespace ledger {

entry_base_t::entry_base_t(const entry_base_t& e)
  : item_t(), journal(NULL)
{
  TRACE_CTOR(entry_base_t, "copy");
  xacts.insert(xacts.end(), e.xacts.begin(), e.xacts.end());
}

entry_base_t::~entry_base_t()
{
  TRACE_DTOR(entry_base_t);

  foreach (xact_t * xact, xacts) {
    // If the transaction is a temporary, it will be destructed when the
    // temporary is.  If it's from a binary cache, we can safely destruct it
    // but its memory will be deallocated with the cache.
    if (! xact->has_flags(ITEM_TEMP)) {
      if (! xact->has_flags(ITEM_IN_CACHE))
	checked_delete(xact);
      else
	xact->~xact_t();
    }
  }
}

item_t::state_t entry_base_t::state() const
{
  bool	  first	 = true;
  state_t result = UNCLEARED;

  foreach (xact_t * xact, xacts) {
    if ((result == UNCLEARED && xact->_state != UNCLEARED) ||
	(result == PENDING   && xact->_state == CLEARED))
      result = xact->_state;
  }

  return result;
}

void entry_base_t::add_xact(xact_t * xact)
{
  xacts.push_back(xact);
}

bool entry_base_t::remove_xact(xact_t * xact)
{
  xacts.remove(xact);
  return true;
}

bool entry_base_t::finalize()
{
  // Scan through and compute the total balance for the entry.  This is used
  // for auto-calculating the value of entries with no cost, and the per-unit
  // price of unpriced commodities.

  // (let ((balance 0)
  //       null-xact)

  value_t  balance;
  xact_t * null_xact = NULL;

  foreach (xact_t * xact, xacts) {
    if (xact->must_balance()) {
      amount_t& p(xact->cost ? *xact->cost : xact->amount);
      DEBUG("entry.finalize", "xact must balance = " << p);
      if (! p.is_null()) {
	add_or_set_value(balance, p);
      } else {
	if (null_xact)
	  throw_(std::logic_error,
		 "Only one xact with null amount allowed per entry");
	else
	  null_xact = xact;
      }
    }
  }
  assert(balance.valid());

  DEBUG("entry.finalize", "initial balance = " << balance);

  // If there is only one xact, balance against the default account if
  // one has been set.

  if (journal && journal->basket && xacts.size() == 1 && ! balance.is_null()) {
    // jww (2008-07-24): Need to make the rest of the code aware of what to do
    // when it sees a generated xact.
    null_xact = new xact_t(journal->basket, ITEM_GENERATED);
    null_xact->_state = (*xacts.begin())->_state;
    add_xact(null_xact);
  }

  if (null_xact != NULL) {
    // If one xact has no value at all, its value will become the
    // inverse of the rest.  If multiple commodities are involved, multiple
    // xacts are generated to balance them all.

    if (balance.is_balance()) {
      bool first = true;
      const balance_t& bal(balance.as_balance());
      foreach (const balance_t::amounts_map::value_type& pair, bal.amounts) {
	if (first) {
	  null_xact->amount = pair.second.negate();
	  first = false;
	} else {
	  add_xact(new xact_t(null_xact->account, pair.second.negate(),
			      ITEM_GENERATED));
	}
      }
    }
    else if (balance.is_amount()) {
      null_xact->amount = balance.as_amount().negate();
      null_xact->add_flags(XACT_CALCULATED);
    }
    else if (! balance.is_null() && ! balance.is_realzero()) {
      throw_(balance_error, "Entry does not balance");
    }
    balance = NULL_VALUE;

  }
  else if (balance.is_balance() &&
	   balance.as_balance().amounts.size() == 2) {
    // When an entry involves two different commodities (regardless of how
    // many xacts there are) determine the conversion ratio by dividing
    // the total value of one commodity by the total value of the other.  This
    // establishes the per-unit cost for this xact for both
    // commodities.

    const balance_t& bal(balance.as_balance());

    balance_t::amounts_map::const_iterator a = bal.amounts.begin();
    
    const amount_t& x((*a++).second);
    const amount_t& y((*a++).second);

    if (! y.is_realzero()) {
      amount_t per_unit_cost = (x / y).abs();

      commodity_t& comm(x.commodity());

      foreach (xact_t * xact, xacts) {
	const amount_t& amt(xact->amount);

	if (! (xact->cost || ! xact->must_balance() ||
	       amt.commodity() == comm)) {
	  balance -= amt;
	  xact->cost = per_unit_cost * amt;
	  balance += *xact->cost;
	}

      }
    }

    DEBUG("entry.finalize", "resolved balance = " << balance);
  }

  // Now that the xact list has its final form, calculate the balance
  // once more in terms of total cost, accounting for any possible gain/loss
  // amounts.

  foreach (xact_t * xact, xacts) {
    if (xact->cost) {
      if (xact->amount.commodity() == xact->cost->commodity())
	throw_(balance_error, "Transaction's cost must be of a different commodity");

      commodity_t::cost_breakdown_t breakdown =
	commodity_t::exchange(xact->amount, *xact->cost);

      if (xact->amount.is_annotated())
	add_or_set_value(balance, breakdown.basis_cost - breakdown.final_cost);
      else
	xact->amount = breakdown.amount;
    }
  }

  DEBUG("entry.finalize", "final balance = " << balance);

  if (! balance.is_null()) {
    balance.in_place_round();
    if (! balance.is_zero()) {
#if 0
      new entry_context(*this, "While balancing entry:");
#endif
      add_error_context("Unbalanced remainder is: ");
      add_error_context(value_context(balance));
      throw_(balance_error, "Entry does not balance");
    }
  }

  // Add the final calculated totals each to their related account

  if (dynamic_cast<entry_t *>(this)) {
    bool all_null = true;
    foreach (xact_t * xact, xacts) {
      if (! xact->amount.is_null()) {
	all_null = false;

	// jww (2008-08-09): For now, this feature only works for
	// non-specific commodities.
	add_or_set_value(xact->account->xdata().value, xact->amount);

	DEBUG("entry.finalize.totals",
	      "Total for " << xact->account->fullname() << " + "
	      << xact->amount.strip_annotations() << ": "
	      << xact->account->xdata().value.strip_annotations());
      }
    }
    if (all_null)
      return false;		// ignore this entry completely
  }

  return true;
}

entry_t::entry_t(const entry_t& e)
  : entry_base_t(e), code(e.code), payee(e.payee)
{
  TRACE_CTOR(entry_t, "copy");

  foreach (xact_t * xact, xacts)
    xact->entry = this;
}

void entry_t::add_xact(xact_t * xact)
{
  xact->entry = this;
  entry_base_t::add_xact(xact);
}

namespace {
  value_t get_code(entry_t& entry) {
    if (entry.code)
      return string_value(*entry.code);
    else
      return string_value(empty_string);
  }

  value_t get_payee(entry_t& entry) {
    return string_value(entry.payee);
  }

  template <value_t (*Func)(entry_t&)>
  value_t get_wrapper(call_scope_t& scope) {
    return (*Func)(find_scope<entry_t>(scope));
  }
}

expr_t::ptr_op_t entry_t::lookup(const string& name)
{
  switch (name[0]) {
  case 'c':
    if (name == "code")
      return WRAP_FUNCTOR(get_wrapper<&get_code>);
    break;

  case 'p':
    if (name[1] == '\0' || name == "payee")
      return WRAP_FUNCTOR(get_wrapper<&get_payee>);
    break;
  }

  return item_t::lookup(name);
}

bool entry_t::valid() const
{
  if (! _date || ! journal) {
    DEBUG("ledger.validate", "entry_t: ! _date || ! journal");
    return false;
  }

  foreach (xact_t * xact, xacts)
    if (xact->entry != this || ! xact->valid()) {
      DEBUG("ledger.validate", "entry_t: xact not valid");
      return false;
    }

  return true;
}

#if 0
void entry_context::describe(std::ostream& out) const throw()
{
  if (! desc.empty())
    out << desc << std::endl;

  print_entry(out, entry, "  ");
}
#endif

void auto_entry_t::extend_entry(entry_base_t& entry, bool post)
{
  xacts_list initial_xacts(entry.xacts.begin(),
				  entry.xacts.end());

  foreach (xact_t * initial_xact, initial_xacts) {
    if (predicate(*initial_xact)) {
      foreach (xact_t * xact, xacts) {
	amount_t amt;
	assert(xact->amount);
	if (! xact->amount.commodity()) {
	  if (! post)
	    continue;
	  assert(initial_xact->amount);
	  amt = initial_xact->amount * xact->amount;
	} else {
	  if (post)
	    continue;
	  amt = xact->amount;
	}

	account_t * account  = xact->account;
	string fullname = account->fullname();
	assert(! fullname.empty());
	if (fullname == "$account" || fullname == "@account")
	  account = initial_xact->account;

	// Copy over details so that the resulting xact is a mirror of
	// the automated entry's one.
	xact_t * new_xact = new xact_t(account, amt);
	new_xact->copy_details(*xact);
	new_xact->add_flags(XACT_AUTO);

	entry.add_xact(new_xact);
      }
    }
  }
}

void extend_entry_base(journal_t * journal, entry_base_t& base, bool post)
{
  foreach (auto_entry_t * entry, journal->auto_entries)
    entry->extend_entry(base, post);
}

} // namespace ledger
