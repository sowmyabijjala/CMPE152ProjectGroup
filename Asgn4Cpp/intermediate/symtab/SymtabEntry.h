/**
 * <h1>SymtabEntry</h1>
 *
 * <p>The symbol table entry for various kinds of identifiers.</p>
 *
 * <p>Copyright (c) 2020 by Ronald Mak</p>
 * <p>For instructional purposes only.  No warranties.</p>
 */
#ifndef SYMTABENTRY_H_
#define SYMTABENTRY_H_

#include <string>
#include <vector>

#include "antlr4-runtime.h"

#include "../../Object.h"

// Forward declaration of class Typespec in namespace intermediate::type.
namespace intermediate { namespace type {
    class Typespec;
}};

namespace intermediate { namespace symtab {

using namespace std;
using intermediate::type::Typespec;

// More forward class declarations but in the same namespace.
class Symtab;
class SymtabEntry;

class SymtabEntry
{
private:
    string name;
    double value;

public:
    /**
     * Constructor.
     * @param name the entry's name.
     */
    SymtabEntry(string name) : name(name), value(0.0) {}

    /**
     * Getter.
     * @return the entry's name.
     */
    string getName()  const { return name;  }

    /**
     * Getter.
     * @return the entry's valuel
     */
    double getValue() const { return value; }

    /**
     * Set the entry's value.
     * @param value the value to set.
     */
    void setValue(const double value) { this->value = value; }
};

}}  // namespace intermediate::symtab

#endif /* SYMTABENTRY_H_ */
