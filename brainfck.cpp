#include <iostream>
#include <iterator>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

namespace brainfck
{

/// Copies chars from @a input to @a output up to the delimeter, @a target.
/// @note Disposes of the delimeter.
///
/// @return The number of characters copied.
static size_t
read_until (std::istream &input, std::ostream &output, char target)
{
  std::string buf;
  if (getline (input, buf, target))
  {
    std::copy (
      begin (buf), end (buf), std::ostreambuf_iterator <char> (output.rdbuf ())
    );
  }

  return buf.size ();
}

namespace
{

class context_t
{
public:
  typedef std::vector <char> code_container_t;
  typedef code_container_t::iterator code_iterator_t;

  /// Constructor.
  context_t ();

  /// Executes BF code.
  void
  execute (
    code_iterator_t code_begin, code_iterator_t code_end, std::istream &input,
    std::ostream &out
  );

private:
  typedef std::vector <unsigned char> slot_container_t;
  typedef std::stack <code_iterator_t> stash_container_t;

  void
  increment ();

  void
  decrement ();

  /// @throw std::underflow_error
  void
  prev_slot ();

  void
  next_slot ();

  void
  send_out (std::ostream &out);

  void
  read_in (std::istream &input);

  void
  start_loop (
    code_iterator_t *it, code_iterator_t code_end, stash_container_t *stash
  );

  void
  end_loop (code_iterator_t *it, stash_container_t *stash);

  /**/

  slot_container_t slots_;
  slot_container_t::iterator slot_;

  context_t (const context_t &) = delete;
  context_t & operator = (const context_t &) = delete;
};

} // anonymous namespace

context_t::context_t ()
: slots_   (1, 0),
  slot_    (begin (slots_))
{
}

void
context_t::execute (
  code_iterator_t code_begin, code_iterator_t code_end, std::istream &input,
  std::ostream &out )
{
  stash_container_t stash;
  for (auto cp = code_begin; cp != code_end; ++cp)
  {
    switch (*cp)
    {
    case '+': increment (); break;
    case '-': decrement (); break;
    case '<': prev_slot (); break;
    case '>': next_slot (); break;
    case '.': send_out (out); break;
    case ',': read_in (input); break;
    case '[': start_loop (&cp, code_end, &stash); break;
    case ']': end_loop (&cp, &stash); break;
    }
  }
}

void
context_t::increment ()
{
  ++*slot_;
}

void
context_t::decrement () {
  --*slot_;
}

void
context_t::prev_slot ()
{
  if (slot_ == begin (slots_))
    throw std::underflow_error ("slot underflow");
  --slot_;
}

void
context_t::next_slot ()
{
  if (next (slot_) == end (slots_))
  {
    slots_.push_back (0);
    slot_ = prev (end (slots_));
  }
  else
  {
    ++slot_;
  }
}

void
context_t::send_out (std::ostream &out)
{
  out << *slot_;
}

void
context_t::read_in (std::istream &input)
{
  if (EOF != input.peek ())
    *slot_ = input.get ();
}

void
context_t::start_loop (
  code_iterator_t *it, code_iterator_t code_end, stash_container_t *stash )
{
  if (*slot_)
  {
    stash->push (*it);
    return;
  }
  while (++*it != code_end)
  {
    if (']' == **it)
      return;
  }
}

void
context_t::end_loop (code_iterator_t *it, stash_container_t *stash)
{
  if (*slot_)
    *it = stash->top ();
  else
    stash->pop ();
}

static int
main ()
{
  size_t input_count, line_count;
  std::cin >> input_count >> line_count >> std::ws;

  std::stringstream input;
  size_t actual_input = read_until (std::cin, input, '$');
  if (actual_input != input_count)
  {
    std::cerr << "Invalid input, expected " << input_count << " characters, "
      << "received " << actual_input << std::endl;
    return 1;
  }

  std::cin >> std::ws;

  size_t lines = 0;
  std::vector <char> code;
  for (size_t i = 0; i < line_count; ++i)
  {
    std::string line;
    if (!getline (std::cin, line))
      break;

    std::copy (begin (line), end (line), std::back_inserter (code));
    ++lines;
  }

  if (lines != line_count)
  {
    std::cerr << "Expected " << line_count << " lines, received " << lines
      << std::endl;
    return 1;
  }

  context_t c;
  c.execute (begin (code), end (code), input, std::cout);

  std::cout << std::endl;
  return 0;
}

} // namespace brainfck

int
main ()
{
  return brainfck::main ();
}
