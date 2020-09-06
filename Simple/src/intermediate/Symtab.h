/**
 * Symbol table class for a simple interpreter.
 *
 * (c) 2020 by Ronald Mak
 * Department of Computer Science
 * San Jose State University
 */
#ifndef SYMTAB_H_
#define SYMTAB_H_

#include <string>
#include <map>

#include "SymtabEntry.h"

namespace intermediate {

using namespace std;

class Symtab
{
private:
    map<string, SymtabEntry *> contents;

public:
    SymtabEntry *enter(string name)
    {
        SymtabEntry *entry = new SymtabEntry(name);
        contents[name] = entry;

        return entry;
    }

    SymtabEntry *lookup(string name)
    {
        return contents.find(name) != contents.end() ? contents[name]
                                                     : nullptr;
    }
};

}  // namespace intermediate

#endif /* SYMTAB_H_ */
