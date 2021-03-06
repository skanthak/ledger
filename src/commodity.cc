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

/**
 * @file   commodity.cc
 * @author John Wiegley
 * @date   Thu Apr 26 15:19:46 2007
 *
 * @brief  Types for dealing with commodities.
 *
 * This file defines member functions for flavors of commodity_t.
 */

#include "amount.h"

namespace ledger {

void commodity_t::base_t::history_t::add_price(const datetime_t& date,
					       const amount_t&	 price)
{
  DEBUG("commodity.prices",
	"add_price: " << date << ", " << price);

  history_map::iterator i = prices.find(date);
  if (i != prices.end()) {
    (*i).second = price;
  } else {
    std::pair<history_map::iterator, bool> result
      = prices.insert(history_map::value_type(date, price));
    assert(result.second);
  }
}

bool commodity_t::base_t::history_t::remove_price(const datetime_t& date)
{
  DEBUG("commodity.prices", "remove_price: " << date);

  history_map::size_type n = prices.erase(date);
  if (n > 0)
    return true;
  return false;
}

void commodity_t::base_t::varied_history_t::add_price(const datetime_t& date,
						      const amount_t&	price)
{
  DEBUG("commodity.prices",
	"varied_add_price: " << date << ", " << price);

  optional<history_t&> hist = history(price.commodity());
  if (! hist) {
    std::pair<history_by_commodity_map::iterator, bool> result
      = histories.insert(history_by_commodity_map::value_type
			 (&price.commodity(), history_t()));
    assert(result.second);

    hist = (*result.first).second;
  }
  assert(hist);

  hist->add_price(date, price);
}

bool commodity_t::base_t::varied_history_t::remove_price(const datetime_t&  date,
							 commodity_t& comm)
{
  DEBUG("commodity.prices",
	"varied_remove_price: " << date << ", " << comm);

  if (optional<history_t&> hist = history(comm))
    return hist->remove_price(date);
  return false;
}

optional<amount_t>
commodity_t::base_t::history_t::find_price(const optional<datetime_t>& moment)
{
  optional<datetime_t> age;
  optional<amount_t>   price;

#if defined(DEBUG_ON)
  if (moment) {
    DEBUG("commodity.prices", "find_price: " << *moment);
  } else {
    DEBUG("commodity.prices", "find_price");
  }
#endif

  if (prices.size() == 0) {
    DEBUG("commodity.prices", "  there are no prices in the history");
    return none;
  }

  if (! moment) {
    history_map::const_reverse_iterator r = prices.rbegin();
    age   = (*r).first;
    price = (*r).second;

    DEBUG("commodity.prices",
	  "  returning most recent price: " << age << ", " << price);
  } else {
    history_map::const_iterator i = prices.lower_bound(*moment);
    if (i == prices.end()) {
      history_map::const_reverse_iterator r = prices.rbegin();
      age   = (*r).first;
      price = (*r).second;

      DEBUG("commodity.prices",
	    "  returning last price: " << age << ", " << price);
    } else {
      age = (*i).first;
      if (*moment != *age) {
	if (i != prices.begin()) {
	  --i;
	  age	  = (*i).first;
	  price = (*i).second;
	} else {
	  age   = none;
	}
      } else {
	price = (*i).second;
      }

      DEBUG("commodity.prices",
	    "  returning found price: " << age << ", " << price);
    }
  }

#if 0
  if (! has_flags(COMMODITY_STYLE_NOMARKET) && parent().get_quote) {
    if (optional<amount_t> quote = parent().get_quote
	(*this, age, moment,
	 (hist && hist->prices.size() > 0 ?
	  (*hist->prices.rbegin()).first : optional<datetime_t>())))
      return *quote;
  }
#endif

  return price;
}

optional<amount_t>
commodity_t::base_t::varied_history_t::find_price
  (const optional<commodity_t&>& commodity,
   const optional<datetime_t>&	 moment)
{
  optional<amount_t> amt;

#if defined(DEBUG_ON)
  DEBUG("commodity.prices", "varied_find_price");

  if (commodity)
    DEBUG("commodity.prices", "  looking for commodity '" << *commodity << "'");
  else
    DEBUG("commodity.prices", "  looking for any commodity");

  if (moment)
    DEBUG("commodity.prices", "  time index: " << *moment);
#endif

  if (optional<history_t&> hist = history(commodity)) {
    DEBUG("commodity.prices", "  found a history for the commodity");

    amt = hist->find_price(moment);

#if defined(DEBUG_ON)
    if (amt)
      DEBUG("commodity.prices", "  found price in that history: " << *amt);
    else
      DEBUG("commodity.prices", "  found no price in that history");
#endif
  }

  // Either we couldn't find a history for the target commodity, or we
  // couldn't find a price.  In either case, search all histories known
  // to this commodity for a price which we can calculate in terms of
  // the goal commodity.
  if (! amt && commodity) {
    foreach (history_by_commodity_map::value_type hist, histories) {
      commodity_t& comm(*hist.first);

      DEBUG("commodity.prices",
	    "  searching for price via commodity '" << comm << "'");

      amt = comm.find_price(commodity, moment);
      // jww (2008-09-24): look for the most recent match

#if defined(DEBUG_ON)
      if (amt)
	DEBUG("commodity.prices", "  found price there: " << *amt);
      else
	DEBUG("commodity.prices", "  found no price there");
#endif
    }
  }

  return amt;
}

optional<amount_t>
commodity_t::base_t::varied_history_t::find_price
  (const std::vector<commodity_t *>& commodities,
   const optional<datetime_t>&	     moment)
{
  foreach (commodity_t * commodity, commodities) {
    if (optional<amount_t> amt = find_price(*commodity, moment))
      return amt;
  }
  return none;
}

optional<commodity_t::base_t::history_t&>
commodity_t::base_t::varied_history_t::history
  (const optional<commodity_t&>& commodity)
{
  commodity_t * comm = NULL;
  if (! commodity) {
    if (histories.size() > 1)
      // jww (2008-09-20): Document which option switch to use here
      throw_(commodity_error,
	     "Cannot determine price history: prices known for multiple commodities (use -?)");
    comm = (*histories.begin()).first;
  } else {
    comm = &(*commodity);
  }

  history_by_commodity_map::iterator i = histories.find(comm);
  if (i != histories.end())
    return (*i).second;

  return none;
}

optional<commodity_t::history_t&>
commodity_t::base_t::varied_history_t::history
  (const std::vector<commodity_t *>& commodities)
{
  // This function differs from the single commodity case avoid in that
  // 'commodities' represents a list of preferred valuation commodities.
  // If no price can be located in terms of the first commodity, then
  // the second is chosen, etc.

  foreach (commodity_t * commodity, commodities) {
    if (optional<history_t&> hist = history(*commodity))
      return hist;
  }
  return none;
}

void commodity_t::exchange(commodity_t&	     commodity,
			   const amount_t&   per_unit_cost,
			   const datetime_t& moment)
{
  if (! commodity.has_flags(COMMODITY_STYLE_NOMARKET)) {
    commodity_t& base_commodity
      (commodity.annotated ?
       as_annotated_commodity(commodity).referent() : commodity);

    base_commodity.add_price(moment, per_unit_cost);
  }
}

commodity_t::cost_breakdown_t
commodity_t::exchange(const amount_t&		  amount,
		      const amount_t&		  cost,
		      const bool		  is_per_unit,
		      const optional<datetime_t>& moment,
		      const optional<string>&     tag)
{
  // (let* ((commodity (amount-commodity amount))
  //        (current-annotation
  //         (and (annotated-commodity-p commodity)
  //              (commodity-annotation commodity)))
  //        (base-commodity (if (annotated-commodity-p commodity)
  //                            (get-referent commodity)
  //                            commodity))
  //        (per-unit-cost (or per-unit-cost
  //                           (divide total-cost amount)))
  //        (total-cost (or total-cost
  //                        (multiply per-unit-cost amount))))

  commodity_t& commodity(amount.commodity());

  annotation_t * current_annotation = NULL;
  if (commodity.annotated)
    current_annotation = &as_annotated_commodity(commodity).details;

  commodity_t& base_commodity
    (current_annotation ?
     as_annotated_commodity(commodity).referent() : commodity);

  amount_t per_unit_cost(is_per_unit ? cost : cost / amount);

  cost_breakdown_t breakdown;
  breakdown.final_cost = ! is_per_unit ? cost : cost * amount;

  // Add a price history entry for this conversion if we know when it took
  // place

  // (if (and moment (not (commodity-no-market-price-p base-commodity)))
  //     (add-price base-commodity per-unit-cost moment))

  if (moment && ! commodity.has_flags(COMMODITY_STYLE_NOMARKET))
    base_commodity.add_price(*moment, per_unit_cost);

  // ;; returns: ANNOTATED-AMOUNT TOTAL-COST BASIS-COST
  // (values (annotate-commodity
  //          amount
  //          (make-commodity-annotation :price per-unit-cost
  //                                     :date  moment
  //                                     :tag   tag))
  //         total-cost
  //         (if current-annotation
  //             (multiply (annotation-price current-annotation) amount)
  //             total-cost))))

  if (current_annotation && current_annotation->price)
    breakdown.basis_cost = *current_annotation->price * amount;
  else
    breakdown.basis_cost = breakdown.final_cost;

  breakdown.amount =
    amount_t(amount, annotation_t (per_unit_cost, moment ?
				   moment->date() : optional<date_t>(), tag));

  return breakdown;
}

commodity_t::operator bool() const
{
  return this != parent().null_commodity;
}

bool commodity_t::symbol_needs_quotes(const string& symbol)
{
  foreach (char ch, symbol)
    if (std::isspace(ch) || std::isdigit(ch) || ch == '-' || ch == '.')
      return true;

  return false;
}

void commodity_t::parse_symbol(std::istream& in, string& symbol)
{
  // Invalid commodity characters:
  //   SPACE, TAB, NEWLINE, RETURN
  //   0-9 . , ; - + * / ^ ? : & | ! =
  //   < > { } [ ] ( ) @

  static int invalid_chars[256] = {
    /* 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f */
    /* 00 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0,
    /* 10 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 20 */ 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1,
    /* 30 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* 40 */ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 50 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0,
    /* 60 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 70 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0,
    /* 80 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 90 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* a0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* b0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* c0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* d0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* e0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* f0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  };

  char buf[256];
  char c = peek_next_nonws(in);
  if (c == '"') {
    in.get(c);
    READ_INTO(in, buf, 255, c, c != '"');
    if (c == '"')
      in.get(c);
    else
      throw_(amount_error, "Quoted commodity symbol lacks closing quote");
  } else {
    READ_INTO(in, buf, 255, c, ! invalid_chars[static_cast<unsigned char>(c)]);
  }
  symbol = buf;
}

void commodity_t::parse_symbol(char *& p, string& symbol)
{
  if (*p == '"') {
    char * q = std::strchr(p + 1, '"');
    if (! q)
      throw_(amount_error, "Quoted commodity symbol lacks closing quote");
    symbol = string(p + 1, 0, q - p - 1);
    p = q + 2;
  } else {
    char * q = next_element(p);
    symbol = p;
    if (q)
      p = q;
    else
      p += symbol.length();
  }
  if (symbol.empty())
    throw_(amount_error, "Failed to parse commodity");
}

bool commodity_t::valid() const
{
  if (symbol().empty() && this != parent().null_commodity) {
    DEBUG("ledger.validate",
	   "commodity_t: symbol().empty() && this != null_commodity");
    return false;
  }

  if (annotated && ! base) {
    DEBUG("ledger.validate", "commodity_t: annotated && ! base");
    return false;
  }

  if (precision() > 16) {
    DEBUG("ledger.validate", "commodity_t: precision() > 16");
    return false;
  }

  return true;
}

void annotation_t::parse(std::istream& in)
{
  do {
    char buf[256];
    char c = peek_next_nonws(in);
    if (c == '{') {
      if (price)
	throw_(amount_error, "Commodity specifies more than one price");

      in.get(c);
      READ_INTO(in, buf, 255, c, c != '}');
      if (c == '}')
	in.get(c);
      else
	throw_(amount_error, "Commodity price lacks closing brace");

      amount_t temp;
      temp.parse(buf, AMOUNT_PARSE_NO_MIGRATE);
      temp.in_place_reduce();

      // Since this price will maintain its own precision, make sure
      // it is at least as large as the base commodity, since the user
      // may have only specified {$1} or something similar.

      if (temp.has_commodity() &&
	  temp.precision() < temp.commodity().precision())
	temp = temp.round();	// no need to retain individual precision

      price = temp;
    }
    else if (c == '[') {
      if (date)
	throw_(amount_error, "Commodity specifies more than one date");

      in.get(c);
      READ_INTO(in, buf, 255, c, c != ']');
      if (c == ']')
	in.get(c);
      else
	throw_(amount_error, "Commodity date lacks closing bracket");

      date = parse_date(buf);
    }
    else if (c == '(') {
      if (tag)
	throw_(amount_error, "Commodity specifies more than one tag");

      in.get(c);
      READ_INTO(in, buf, 255, c, c != ')');
      if (c == ')')
	in.get(c);
      else
	throw_(amount_error, "Commodity tag lacks closing parenthesis");

      tag = buf;
    }
    else {
      break;
    }
  } while (true);

  DEBUG("amounts.commodities",
	"Parsed commodity annotations: " << std::endl << *this);
}

bool annotated_commodity_t::operator==(const commodity_t& comm) const
{
  // If the base commodities don't match, the game's up.
  if (base != comm.base)
    return false;

  assert(annotated);
  if (! comm.annotated)
    return false;

  if (details != as_annotated_commodity(comm).details)
    return false;

  return true;
}

commodity_t&
annotated_commodity_t::strip_annotations(const bool _keep_price,
					 const bool _keep_date,
					 const bool _keep_tag)
{
  DEBUG("commodity.annotated.strip",
	"Reducing commodity " << *this << std::endl
	 << "  keep price " << _keep_price << " "
	 << "  keep date "  << _keep_date << " "
	 << "  keep tag "   << _keep_tag);

  commodity_t * new_comm;

  if ((_keep_price && details.price) ||
      (_keep_date  && details.date) ||
      (_keep_tag   && details.tag))
  {
    new_comm = parent().find_or_create
      (referent(),
       annotation_t(_keep_price ? details.price : none,
		    _keep_date  ? details.date  : none,
		    _keep_tag   ? details.tag   : none));
  } else {
    new_comm = parent().find_or_create(base_symbol());
  }

  assert(new_comm);
  return *new_comm;
}

void annotated_commodity_t::write_annotations(std::ostream&       out,
					      const annotation_t& info)
{
  if (info.price)
    out << " {" << *info.price << '}';

  if (info.date)
    out << " [" << format_date(*info.date) << ']';

  if (info.tag)
    out << " (" << *info.tag << ')';
}

bool compare_amount_commodities::operator()(const amount_t * left,
					    const amount_t * right) const
{
  commodity_t& leftcomm(left->commodity());
  commodity_t& rightcomm(right->commodity());

  DEBUG("commodity.compare", " left symbol (" << leftcomm << ")");
  DEBUG("commodity.compare", "right symbol (" << rightcomm << ")");

  int cmp = leftcomm.base_symbol().compare(rightcomm.base_symbol());
  if (cmp != 0)
    return cmp < 0;

  if (! leftcomm.annotated) {
    return rightcomm.annotated;
  }
  else if (! rightcomm.annotated) {
    return ! leftcomm.annotated;
  }
  else {
    annotated_commodity_t& aleftcomm(static_cast<annotated_commodity_t&>(leftcomm));
    annotated_commodity_t& arightcomm(static_cast<annotated_commodity_t&>(rightcomm));

    if (! aleftcomm.details.price && arightcomm.details.price)
      return true;
    if (aleftcomm.details.price && ! arightcomm.details.price)
      return false;

    if (aleftcomm.details.price && arightcomm.details.price) {
      amount_t leftprice(*aleftcomm.details.price);
      leftprice.in_place_reduce();
      amount_t rightprice(*arightcomm.details.price);
      rightprice.in_place_reduce();

      if (leftprice.commodity() == rightprice.commodity()) {
	return (leftprice - rightprice).sign() < 0;
      } else {
	// Since we have two different amounts, there's really no way
	// to establish a true sorting order; we'll just do it based
	// on the numerical values.
	leftprice.clear_commodity();
	rightprice.clear_commodity();
	return (leftprice - rightprice).sign() < 0;
      }
    }

    if (! aleftcomm.details.date && arightcomm.details.date)
      return true;
    if (aleftcomm.details.date && ! arightcomm.details.date)
      return false;

    if (aleftcomm.details.date && arightcomm.details.date) {
      date_duration_t diff = *aleftcomm.details.date - *arightcomm.details.date;
      return diff.is_negative();
    }

    if (! aleftcomm.details.tag && arightcomm.details.tag)
      return true;
    if (aleftcomm.details.tag && ! arightcomm.details.tag)
      return false;

    if (aleftcomm.details.tag && arightcomm.details.tag)
      return *aleftcomm.details.tag < *arightcomm.details.tag;

    assert(false);
    return true;
  }
}

commodity_pool_t::commodity_pool_t() : default_commodity(NULL)
{
  TRACE_CTOR(commodity_pool_t, "");
  null_commodity = create("");
  null_commodity->add_flags(COMMODITY_STYLE_NOMARKET |
			    COMMODITY_STYLE_BUILTIN);
}

commodity_t * commodity_pool_t::create(const string& symbol)
{
  shared_ptr<commodity_t::base_t>
    base_commodity(new commodity_t::base_t(symbol));
  std::auto_ptr<commodity_t> commodity(new commodity_t(this, base_commodity));

  DEBUG("amounts.commodities", "Creating base commodity " << symbol);

  // Create the "qualified symbol" version of this commodity's symbol
  if (commodity_t::symbol_needs_quotes(symbol)) {
    commodity->qualified_symbol = "\"";
    *commodity->qualified_symbol += symbol;
    *commodity->qualified_symbol += "\"";
  }

  DEBUG("amounts.commodities",
	"Creating commodity '" << commodity->symbol() << "'");

  commodity->ident = commodities.size();

  commodities.push_back(commodity.get());
  return commodity.release();
}

commodity_t * commodity_pool_t::find_or_create(const string& symbol)
{
  DEBUG("amounts.commodities", "Find-or-create commodity " << symbol);

  commodity_t * commodity = find(symbol);
  if (commodity)
    return commodity;
  return create(symbol);
}

commodity_t * commodity_pool_t::find(const string& symbol)
{
  DEBUG("amounts.commodities", "Find commodity " << symbol);

  typedef commodity_pool_t::commodities_t::nth_index<1>::type
    commodities_by_name;

  commodities_by_name& name_index = commodities.get<1>();
  commodities_by_name::const_iterator i = name_index.find(symbol);
  if (i != name_index.end())
    return *i;
  else
    return NULL;
}

commodity_t * commodity_pool_t::find(const commodity_t::ident_t ident)
{
  DEBUG("amounts.commodities", "Find commodity by ident " << ident);

  typedef commodity_pool_t::commodities_t::nth_index<0>::type
    commodities_by_ident;

  commodities_by_ident& ident_index = commodities.get<0>();
  return ident_index[ident];
}

commodity_t *
commodity_pool_t::create(const string& symbol, const annotation_t& details)
{
  commodity_t * new_comm = create(symbol);
  if (! new_comm)
    return NULL;

  if (details)
    return find_or_create(*new_comm, details);
  else
    return new_comm;
}

namespace {
  string make_qualified_name(const commodity_t&  comm,
			     const annotation_t& details)
  {
    assert(details);

    if (details.price && details.price->sign() < 0)
      throw_(amount_error, "A commodity's price may not be negative");

    std::ostringstream name;
    comm.print(name);
    annotated_commodity_t::write_annotations(name, details);

    DEBUG("amounts.commodities", "make_qualified_name for "
	  << comm.qualified_symbol << std::endl << details);
    DEBUG("amounts.commodities", "qualified_name is " << name.str());

    return name.str();
  }
}

commodity_t *
commodity_pool_t::find(const string& symbol, const annotation_t& details)
{
  commodity_t * comm = find(symbol);
  if (! comm)
    return NULL;

  if (details) {
    string name = make_qualified_name(*comm, details);

    if (commodity_t * ann_comm = find(name)) {
      assert(ann_comm->annotated && as_annotated_commodity(*ann_comm).details);
      return ann_comm;
    }
    return NULL;
  } else {
    return comm;
  }
}

commodity_t *
commodity_pool_t::find_or_create(const string& symbol,
				 const annotation_t& details)
{
  commodity_t * comm = find(symbol);
  if (! comm)
    return NULL;

  if (details)
    return find_or_create(*comm, details);
  else
    return comm;
}

commodity_t *
commodity_pool_t::create(commodity_t&	     comm,
			 const annotation_t& details,
			 const string&	     mapping_key)
{
  assert(comm);
  assert(details);
  assert(! mapping_key.empty());

  std::auto_ptr<commodity_t> commodity
    (new annotated_commodity_t(&comm, details));

  commodity->qualified_symbol = comm.symbol();
  assert(! commodity->qualified_symbol->empty());

  DEBUG("amounts.commodities", "Creating annotated commodity "
	<< "symbol " << commodity->symbol()
	<< " key "   << mapping_key << std::endl << details);

  // Add the fully annotated name to the map, so that this symbol may
  // quickly be found again.
  commodity->ident	  = commodities.size();
  commodity->mapping_key_ = mapping_key;

  commodities.push_back(commodity.get());
  return commodity.release();
}

commodity_t * commodity_pool_t::find_or_create(commodity_t&	   comm,
					       const annotation_t& details)
{
  assert(comm);
  assert(details);

  string name = make_qualified_name(comm, details);
  assert(! name.empty());

  if (commodity_t * ann_comm = find(name)) {
    assert(ann_comm->annotated && as_annotated_commodity(*ann_comm).details);
    return ann_comm;
  }
  return create(comm, details, name);
}

} // namespace ledger
