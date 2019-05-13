#ifndef _PLINQ_DETAIL_QUERIES_H_
#define _PLINQ_DETAIL_QUERIES_H_

// payload
//  Class and template with payload postfix is payload class/template.
//  This type of class stores the necessary data to perform the corresponding query, e.g. count_payload is the payload class for query count.
//  There are two kind of data, the first kind is those which aren't tied to a specific execution of query, e.g. the predicate of query select.
//  The second kind is those which are tied to a specific execution, e.g. a count of size_t type of query count, which should be re-initilized every time
//  the query is executed. (Multi-execution is not supported yet)
// -class (template) Payload should declare the following members (excepts aggregator_payload):
//  template <class> struct output_type { using type = ***; };
//  template <size_t, class> struct actor;
//
//
// actor
//  Nested class template of payload named actor is actor class template.
//  This type of template stores the second type of data, which should be re-initialized in each execution of the query.
// -Actor template should be defined as:
//  template <class idx, class Payload> struct actor;
//  where the idx is his pos in the query sequence, started with 0
//  Payload is all the payload of the query. (TODO: Why all the payload??)
//
// -Actor template should defined the following member:
//   using output_type = ***;
//   using aggregator_type = ***;
//   actor();
//   template <class Act> void init(Act &);
//   template <class Act> void apply(Act &); // can declare additional parameters, and generally should.
//   template <class Act> void done(Act &); // can declare additional parameters.
//   *** get_result();
//
//  init is called when the query is about to begin.
//  apply is called during the query, to perform the actual functionality of the query.
//  done is called when the query ends.
// -Actor should call apply_next and done_next on the param to inform the next query when necessary.
//

#include "queries/concurrent-range-scheduler-payload.h"
#include "queries/concurrent-scheduler-payload.h"
#include "queries/count-payload.h"
#include "queries/range-payload.h"
#include "queries/select-payload.h"
#include "queries/source-payload.h"

#endif // _PLINQ_DETAIL_QUERIES_H_
