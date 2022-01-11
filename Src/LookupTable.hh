#ifndef FRENDER_LOOKUPTABLE_HH
#define FRENDER_LOOKUPTABLE_HH

#include <cstdint>
#include <unordered_map>
namespace Frender
{

typedef uint32_t LookupId;

/**
Simple struct that stores the 4 locators
*/
struct RenderIndex
{
    int shader_section;
    int mat_section;
    int mesh_section;
    int index;
};

/**
A helper class that stores a lookup table to convert between "unique ids" and
the set of 4 handles required to locate an object
*/
class LookupTable
{
  public:
    LookupTable() : current_id(0), table()
    {
    }

    /// Insert a new element into the LookupTable and get it's id
    LookupId push(RenderIndex index);

    /// Get a copy of the data at the given id. Note that it is a copy so `set` must
    /// be used to change it
    ///
    /// *Warning:* This will raise an exception if the id could not be found
    RenderIndex at(LookupId id) const;

    /// Set the value of a certain id. If you want to change the value of the id, use
    /// `at` first to get a copy, which you can modify and put back into set
    ///
    /// *Warning:* This will raise an exception if the id could not be found
    void set(LookupId id, RenderIndex index);

    /// Removes an element from the lookup table
    ///
    /// *Warning:* This will raise an exception if the id could not be found
    void remove(LookupId id);

  private:
    uint32_t current_id;
    std::unordered_map<LookupId, RenderIndex> table;
};
} // namespace Frender

#endif
