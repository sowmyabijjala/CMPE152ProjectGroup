/**
 * Symbol table entry class for a simple interpreter.
 *
 * (c) 2020 by Ronald Mak
 * Department of Computer Science
 * San Jose State University
 */
#ifndef SYMTABENTRY_H_
#define SYMTABENTRY_H_

#include <string>

namespace intermediate {

using namespace std;

class SymtabEntry
{
private:
    string name;
    double value;

public:
    SymtabEntry(string name) : name(name), value(0.0) {}

    string getName()  const { return name;  }
    double getValue() const { return value; }

    void setValue(const double value) { this->value = value; }
};

}  // namespace intermediate

#endif /* SYMTABENTRY_H_ */
