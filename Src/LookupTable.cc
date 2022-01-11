#include "LookupTable.hh"
#include <stdexcept>

Frender::LookupId Frender::LookupTable::push(RenderIndex index)
{
    LookupId id = current_id;
    current_id++;

    table.emplace(id, index);
    return id;
}

Frender::RenderIndex Frender::LookupTable::at(LookupId id) const
{
    try
    {
        return table.at(id);
    }
    catch (const std::out_of_range& orr)
    {
        throw "Could not find RenderObject in LookupTable";
    }
}

void Frender::LookupTable::set(LookupId id, RenderIndex index)
{
    try
    {
        table.at(id) = index;
    }
    catch (const std::out_of_range& orr)
    {
        throw "Could not find RenderObject in LookupTable";
    }
}

void Frender::LookupTable::remove(LookupId id)
{
    table.erase(id);
}
