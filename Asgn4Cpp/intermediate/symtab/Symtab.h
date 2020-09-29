/**
 * <h1>Symtab</h1>
 *
 * <p>The symbol table.</p>
 *
 * <p>Copyright (c) 2020 by Ronald Mak</p>
 * <p>For instructional purposes only.  No warranties.</p>
 */
#ifndef SYMTABIMPL_H_
#define SYMTABIMPL_H_

#include <string>
#include <map>

#include "SymtabEntry.h"

namespace intermediate { namespace symtab {

using namespace std;
using namespace intermediate;

class Symtab
{
private:
    map<string, SymtabEntry *> contents;

public:
    /**
     * Make an entry.
     * @param name the entry's name.
     */
    SymtabEntry *enter(string name)
    {
        SymtabEntry *entry = new SymtabEntry(name);
        contents[name] = entry;

        return entry;
    }

    /**
     * Look up an entry.
     * @param name the entry's name.
     * @return the entry or null if it's not in the symbol table.
     */
    SymtabEntry *lookup(string name)
    {
        return contents.find(name) != contents.end() ? contents[name]
                                                     : nullptr;
    }
};

}}  // namespace intermediate::symtab

#endif /* SYMTABIMPL_H_ */
