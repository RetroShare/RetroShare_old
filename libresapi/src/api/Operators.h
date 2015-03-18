#pragma once

#include <retroshare/rstypes.h>
#include "ApiTypes.h"

namespace resource_api
{
// note: pass the KeyValueReference and ValueReference objects by value to enable such things:
// stream << somefunc(); // can't get a reference to the return value of somefunc

// uint32_t to std::string with decimal numbers
StreamBase& operator <<(StreamBase& left, KeyValueReference<uint32_t> ref);


// convert retroshare ids to strings and back
//template<uint32_t ID_SIZE, bool ID_UPPER, uint32_t ID_ID>
//StreamBase& operator <<(StreamBase& left, t_RsGenericIdType<ID_SIZE, ID_UPPER, ID_ID>& id);

// operators for retroshare ids
/*
template<class T_ID>
StreamBase& operator <<(StreamBase& left, ValueReference<T_ID>& ref);
*/

template<class T_ID>
StreamBase& operator <<(StreamBase& left, KeyValueReference<T_ID> ref);

template<class T_ID>
StreamBase& operator <<(StreamBase& left, ValueReference<T_ID> ref);

//template<uint32_t ID_SIZE, bool ID_UPPER, uint32_t ID_ID>
//StreamBase& operator <<(StreamBase& left, KeyValueReference<t_RsGenericIdType<ID_SIZE, ID_UPPER, ID_ID> >& ref);






// implementations

// idea: each rs generic id type has a number
// put this number in front of the id data to make the ids type safe, even across languages

template<class T_ID>
StreamBase& operator <<(StreamBase& left, KeyValueReference<T_ID> ref)
//template<uint32_t ID_SIZE, bool ID_UPPER, uint32_t ID_ID>
//StreamBase& operator <<(StreamBase& left, KeyValueReference<t_RsGenericIdType<ID_SIZE, ID_UPPER, ID_ID> >& ref)
{
    if(left.serialise())
    {
        std::string idStr = ref.value.toStdString();
        left << makeKeyValueReference(ref.key, idStr);
    }
    else
    {
        std::string str;
        left << makeKeyValueReference(ref.key, str);
        //t_RsGenericIdType<ID_SIZE, ID_UPPER, ID_ID> id(str);
        T_ID id(str);
        if(id.isNull())
        {
            left.setError();
            left.addErrorMsg("operator for retroshare id keyValue: id is null\n");
        }
        ref.value = id;
    }
    return left;
}

template<class T_ID>
StreamBase& operator <<(StreamBase& left, ValueReference<T_ID> ref)
{
    if(left.serialise())
    {
        std::string idStr = ref.value.toStdString();
        left << makeValueReference(idStr);
    }
    else
    {
        std::string str;
        left << makeValueReference(str);
        T_ID id(str);
        if(id.isNull())
        {
            left.setError();
            left.addErrorMsg("operator for retroshare id Value: id is null\n");
        }
        ref.value = id;
    }
    return left;
}

} // namespace resource_api
