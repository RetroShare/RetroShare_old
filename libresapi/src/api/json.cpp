#include "json.h"
#include <stdlib.h>
#include <string>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <climits>
#include <string.h>
#include <functional>
#include <cctype>
#include <stack>

#ifndef WIN32
#define _stricmp strcasecmp
#endif

#ifdef _MSC_VER
#define snprintf sprintf_s
#endif

using namespace json;

namespace json
{
	enum StackDepthType
	{
		InObject,
		InArray
	};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Helper functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static std::string Trim(const std::string& str)
{
	std::string s = str;
	
	// remove white space in front
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	
	// remove trailing white space
	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	
	return s;
}

// Finds the position of the first " character that is NOT preceeded immediately by a \ character.
// In JSON, \" is valid and has a different meaning than the escaped " character.
static size_t GetQuotePos(const std::string& str, size_t start_pos = 0)
{
	bool found_slash = false;
	for (size_t i = start_pos; i < str.length(); i++)
	{
		char c = str[i];
		if ((c == '\\') && !found_slash)
		{
			found_slash = true;
			continue;
		}
		else if ((c == '\"') && !found_slash)
			return i;
		
		found_slash = false;
	}
	
	return std::string::npos;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Value::Value(const Value& v) : mValueType(v.mValueType)
{
	switch (mValueType)
	{
		case StringVal		: mStringVal = v.mStringVal; break;
		case IntVal			: mIntVal = v.mIntVal; mFloatVal = (float)v.mIntVal; mDoubleVal = (double)v.mIntVal; break;
		case FloatVal		: mFloatVal = v.mFloatVal; mIntVal = (int)v.mFloatVal; mDoubleVal = (double)v.mDoubleVal; break;
		case DoubleVal		: mDoubleVal = v.mDoubleVal; mIntVal = (int)v.mDoubleVal; mFloatVal = (float)v.mDoubleVal; break;
		case BoolVal		: mBoolVal = v.mBoolVal; break;
		case ObjectVal		: mObjectVal = v.mObjectVal; break;
		case ArrayVal		: mArrayVal = v.mArrayVal; break;
		default				: break;
	}
}

Value& Value::operator =(const Value& v)
{
	if (&v == this)
		return *this;

	mValueType = v.mValueType;

	switch (mValueType)
	{
		case StringVal		: mStringVal = v.mStringVal; break;
		case IntVal			: mIntVal = v.mIntVal; mFloatVal = (float)v.mIntVal; mDoubleVal = (double)v.mIntVal; break;
		case FloatVal		: mFloatVal = v.mFloatVal; mIntVal = (int)v.mFloatVal; mDoubleVal = (double)v.mDoubleVal; break;
		case DoubleVal		: mDoubleVal = v.mDoubleVal; mIntVal = (int)v.mDoubleVal; mFloatVal = (float)v.mDoubleVal; break;
		case BoolVal		: mBoolVal = v.mBoolVal; break;
		case ObjectVal		: mObjectVal = v.mObjectVal; break;
		case ArrayVal		: mArrayVal = v.mArrayVal; break;
		default				: break;
	}

	return *this;
}

Value& Value::operator [](size_t idx)
{
	assert(mValueType == ArrayVal);
	return mArrayVal[idx];
}

const Value& Value::operator [](size_t idx) const
{
	assert(mValueType == ArrayVal);
	return mArrayVal[idx];
}

Value& Value::operator [](const std::string& key)
{
	assert(mValueType == ObjectVal);
	return mObjectVal[key];
}

Value& Value::operator [](const char* key)
{
	assert(mValueType == ObjectVal);
	return mObjectVal[key];
}

const Value& Value::operator [](const char* key) const
{
	assert(mValueType == ObjectVal);
	return mObjectVal[key];
}

const Value& Value::operator [](const std::string& key) const
{
	assert(mValueType == ObjectVal);
	return mObjectVal[key];
}

void Value::Clear()
{
	mValueType = NULLVal;
}

size_t Value::size() const
{
	if ((mValueType != ObjectVal) && (mValueType != ArrayVal))
		return 1;

	return mValueType == ObjectVal ? mObjectVal.size() : mArrayVal.size();
}

bool Value::HasKey(const std::string &key) const
{
	assert(mValueType == ObjectVal);
	return mObjectVal.HasKey(key);
}

int Value::HasKeys(const std::vector<std::string> &keys) const
{
	assert(mValueType == ObjectVal);
	return mObjectVal.HasKeys(keys);
}

int Value::HasKeys(const char **keys, int key_count) const
{
	assert(mValueType == ObjectVal);
	return mObjectVal.HasKeys(keys, key_count);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Array::Array()
{
}

Array::Array(const Array& a) : mValues(a.mValues)
{
}

Array& Array::operator =(const Array& a)
{
	if (&a == this)
		return *this;

	Clear();
	mValues = a.mValues;

	return *this;
}

Value& Array::operator [](size_t i)
{
	return mValues[i];
}

const Value& Array::operator [](size_t i) const
{
	return mValues[i];
}


Array::ValueVector::const_iterator Array::begin() const
{
	return mValues.begin();
}

Array::ValueVector::const_iterator Array::end() const
{
	return mValues.end();
}

Array::ValueVector::iterator Array::begin()
{
	return mValues.begin();
}

Array::ValueVector::iterator Array::end()
{
	return mValues.end();
}

void Array::push_back(const Value& v)
{
	mValues.push_back(v);
}

void Array::insert(size_t index, const Value& v)
{
	mValues.insert(mValues.begin() + index, v);
}

size_t Array::size() const
{
	return mValues.size();
}

void Array::Clear()
{
	mValues.clear();
}

Array::ValueVector::iterator Array::find(const Value& v)
{
	return std::find(mValues.begin(), mValues.end(), v);
}

Array::ValueVector::const_iterator Array::find(const Value& v) const
{
	return std::find(mValues.begin(), mValues.end(), v);
}

bool Array::HasValue(const Value& v) const
{
	return find(v) != end();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Object::Object()
{
}

Object::Object(const Object& obj) : mValues(obj.mValues)
{

}

Object& Object::operator =(const Object& obj)
{
	if (&obj == this)
		return *this;

	Clear();
	mValues = obj.mValues;

	return *this;
}

Value& Object::operator [](const std::string& key)
{
	return mValues[key];
}

const Value& Object::operator [](const std::string& key) const
{
	ValueMap::const_iterator it = mValues.find(key);
	return it->second;
}

Value& Object::operator [](const char* key)
{
	return mValues[key];
}

const Value& Object::operator [](const char* key) const
{
	ValueMap::const_iterator it = mValues.find(key);
	return it->second;
}

Object::ValueMap::const_iterator Object::begin() const
{
	return mValues.begin();
}

Object::ValueMap::const_iterator Object::end() const
{
	return mValues.end();
}

Object::ValueMap::iterator Object::begin()
{
	return mValues.begin();
}

Object::ValueMap::iterator Object::end()
{
	return mValues.end();
}

Object::ValueMap::iterator Object::find(const std::string& key)
{
	return mValues.find(key);
}

Object::ValueMap::const_iterator Object::find(const std::string& key) const
{
	return mValues.find(key);
}

bool Object::HasKey(const std::string& key) const
{
	return find(key) != end();
}

int Object::HasKeys(const std::vector<std::string>& keys) const
{
	for (size_t i = 0; i < keys.size(); i++)
	{
		if (!HasKey(keys[i]))
			return (int)i;
	}
	
	return -1;
}

int Object::HasKeys(const char** keys, int key_count) const
{
	for (int i = 0; i < key_count; i++)
		if (!HasKey(keys[i]))
			return i;
	
	return -1;
}

void Object::Clear()
{
	mValues.clear();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::string SerializeArray(const Array& a);

std::string SerializeValue(const Value& v)
{
	std::string str;

	static const int BUFF_SZ = 500;
	char buff[BUFF_SZ];
	switch (v.GetType())
	{
		case IntVal			: snprintf(buff, BUFF_SZ, "%d", (int)v); str = buff; break;
		case FloatVal		: snprintf(buff, BUFF_SZ, "%f", (float)v); str = buff; break;
		case DoubleVal		: snprintf(buff, BUFF_SZ, "%f", (double)v); str = buff; break;
		case BoolVal		: str = v ? "true" : "false"; break;
		case NULLVal		: str = "null"; break;
		case ObjectVal		: str = Serialize(v); break;
		case ArrayVal		: str = SerializeArray(v); break;
		case StringVal		: str = std::string("\"") + (std::string)v + std::string("\""); break;
	}

	return str;
}

std::string SerializeArray(const Array& a)
{
	std::string str = "[";

	bool first = true;
	for (size_t i = 0; i < a.size(); i++)
	{
		const Value& v = a[i];
		if (!first)
			str += std::string(",");

		str += SerializeValue(v);

		first = false;
	}

	str += "]";
	return str;
}

std::string json::Serialize(const Value& v)
{
	std::string str;

	bool first = true;
	
	if (v.GetType() == ObjectVal)
	{
		str = "{";
		Object obj = v.ToObject();
		for (Object::ValueMap::const_iterator it = obj.begin(); it != obj.end(); it++)
		{
			if (!first)
				str += std::string(",");

			str += std::string("\"") + it->first + std::string("\":") + SerializeValue(it->second);
			first = false;
		}

		str += "}";
	}
	else if (v.GetType() == ArrayVal)
	{
		str = "[";
		Array a = v.ToArray();
		for (Array::ValueVector::const_iterator it = a.begin(); it != a.end(); it++)
		{
			if (!first)
				str += std::string(",");
			
			str += SerializeValue(*it);
			first = false;
		}
		
		str += "]";
			
	}
	//else error
	
	return str;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static Value DeserializeArray(std::string& str, std::stack<StackDepthType>& depth_stack);
static Value DeserializeObj(const std::string& _str, std::stack<StackDepthType>& depth_stack);

static Value DeserializeInternal(const std::string& _str, std::stack<StackDepthType>& depth_stack)
{
	Value v;
	
	std::string str = Trim(_str);
	if (str[0] == '{')
	{
		// Error: Began with a { but doesn't end with one
		if (str[str.length() - 1] != '}')
			return Value();
		
		depth_stack.push(InObject);
		v = DeserializeObj(str, depth_stack);
		if ((v.GetType() == NULLVal) || (depth_stack.top() != InObject))
			return v;
		
		depth_stack.pop();
	}
	else if (str[0] == '[')
	{
		// Error: Began with a [ but doesn't end with one
		if (str[str.length() - 1] != ']')
			return Value();
		
		depth_stack.push(InArray);
		v = DeserializeArray(str, depth_stack);
		if ((v.GetType() == NULLVal) || (depth_stack.top() != InArray))
			return v;
		
		depth_stack.pop();
	}
	else
	{
		// Will never get here unless _str is not valid JSON
		return Value();
	}
	
	return v;
}

static size_t GetEndOfArrayOrObj(const std::string& str, std::stack<StackDepthType>& depth_stack)
{
	size_t i = 1;
	bool in_quote = false;
	size_t original_count = depth_stack.size();
	
	for (; i < str.length(); i++)
	{
		if (str[i] == '\"')
		{
			if (str[i - 1] != '\\')
				in_quote = !in_quote;
		}
		else if (!in_quote)
		{
			if (str[i] == '[')
				depth_stack.push(InArray);
			else if (str[i] == '{')
				depth_stack.push(InObject);
			else if (str[i] == ']')
			{
				StackDepthType t = depth_stack.top();
				if (t != InArray)
				{
					// expected to be closing an array but instead we're inside an object block.
					// Example problem: {]}
					return std::string::npos;
				}
				
				size_t count = depth_stack.size();
				depth_stack.pop();
				if (count == original_count)
					break;
			}
			else if (str[i] == '}')
			{
				StackDepthType t = depth_stack.top();
				if (t != InObject)
				{
					// expected to be closing an object but instead we're inside an array.
					// Example problem: [}]
					return std::string::npos;
				}
					
				size_t count = depth_stack.size();
				depth_stack.pop();
				if (count == original_count)
					break;
			}
		}
	}
	
	return i;
}

static std::string UnescapeJSONString(const std::string& str)
{
	std::string s = "";
	
	for (int i = 0; i < str.length(); i++)
	{
		char c = str[i];
		if ((c == '\\') && (i + 1 < str.length()))
		{
			int skip_ahead = 1;
			unsigned int hex;
			std::string hex_str;
			
			switch (str[i+1])
			{
				case '"' : 	s.push_back('\"'); break;
				case '\\': 	s.push_back('\\'); break;
				case '/' : 	s.push_back('/'); break;
				case 't' : 	s.push_back('\t'); break;
				case 'n' : 	s.push_back('\n'); break;
				case 'r' : 	s.push_back('\r'); break;
				case 'b' :	s.push_back('\b'); break;
				case 'f' : 	s.push_back('\f'); break;
				case 'u' : 	skip_ahead = 5;
					hex_str = str.substr(i + 4, 2);
                    hex = (unsigned int)std::strtoul(hex_str.c_str(), NULL, 16);
					s.push_back((char)hex);
					break;
					
				default: break;
			}
			
			i += skip_ahead;
		}
		else
			s.push_back(c);
	}
	
	return Trim(s);
}

static Value DeserializeValue(std::string& str, bool* had_error, std::stack<StackDepthType>& depth_stack)
{
	Value v;

	*had_error = false;
	str = Trim(str);

	if (str.length() == 0)
		return v;

	if (str[0] == '[')
	{
		depth_stack.push(InArray);
		size_t i = GetEndOfArrayOrObj(str, depth_stack);
		if (i == std::string::npos)
		{
			*had_error = true;
			return Value();
		}
		
		std::string array_str = str.substr(0, i + 1);
		v = Value(DeserializeArray(array_str, depth_stack));
		str = str.substr(i + 1, str.length());
	}
	else if (str[0] == '{')
	{
		depth_stack.push(InObject);
		size_t i = GetEndOfArrayOrObj(str, depth_stack);

		if (i == std::string::npos)
		{
			*had_error = true;
			return Value();
		}

		std::string obj_str = str.substr(0, i + 1);
		v = Value(DeserializeInternal(obj_str, depth_stack));
		str = str.substr(i + 1, str.length());
	}
	else if (str[0] == '\"')
	{
		size_t end_quote = GetQuotePos(str, 1);
		if (end_quote == std::string::npos)
		{
			*had_error = true;
			return Value();
		}

		v = Value(UnescapeJSONString(str.substr(1, end_quote - 1)));
		str = str.substr(end_quote + 1, str.length());
	}
	else
	{
		bool has_dot = false;
		bool has_e = false;
		std::string temp_val;
		size_t i = 0;
		for (; i < str.length(); i++)
		{
			if (str[i] == '.')
				has_dot = true;
			else if (str[i] == 'e')
				has_e = true;
			else if (str[i] == ']')
			{
				if (depth_stack.top() != InArray)
				{
					*had_error = true;
					return Value();
				}
				
				depth_stack.pop();
			}
			else if (str[i] == '}')
			{
				if (depth_stack.top() != InObject)
				{
					*had_error = true;
					return Value();
				}
				
				depth_stack.pop();
			}
			else if (str[i] == ',')
				break;

			if (!std::isspace(str[i]))
				temp_val += str[i];
		}

		// store all floating point as doubles. This will also set the float and int values as well.
		if (_stricmp(temp_val.c_str(), "true") == 0)
			v = Value(true);
		else if (_stricmp(temp_val.c_str(), "false") == 0)
			v = Value(false);
		else if (has_e || has_dot)
			v = Value(atof(temp_val.c_str()));
		else if (_stricmp(temp_val.c_str(), "null") == 0)
			v = Value();
		else
		{
			// Check if the value is beyond the size of an int and if so, store it as a double
			double tmp_val = atof(temp_val.c_str());
			if ((tmp_val >= (double)INT_MIN) && (tmp_val <= (double)INT_MAX))
				v = Value(atoi(temp_val.c_str()));
			else
				v = Value(tmp_val);
		}

		str = str.substr(i, str.length());
	}

	return v;
}

static Value DeserializeArray(std::string& str, std::stack<StackDepthType>& depth_stack)
{
	Array a;
	bool had_error = false;

	str = Trim(str);

	if ((str[0] == '[') && (str[str.length() - 1] == ']'))
		str = str.substr(1, str.length() - 2);
	else
		return Value();

	while (str.length() > 0)
	{
		std::string tmp;

		size_t i = 0;
		for (; i < str.length(); i++)
		{
			// If we get to an object or array, parse it:
			if ((str[i] == '{') || (str[i] == '['))
			{
				Value v = DeserializeValue(str, &had_error, depth_stack);
				if (had_error)
					return Value();
				
				if (v.GetType() != NULLVal)
					a.push_back(v);

				break;
			}

			bool terminate_parsing = false;

			if ((str[i] == ',') || (str[i] == ']'))
				terminate_parsing = true;			// hit the end of a value, parse it in the next block
			else
			{
				// keep grabbing chars to build up the value
				tmp += str[i];
				if  (i == str.length() - 1)
					terminate_parsing = true; // end of string, finish parsing
			}

			if (terminate_parsing)
			{
				Value v = DeserializeValue(tmp, &had_error, depth_stack);
				if (had_error)
					return Value();
				
				if (v.GetType() != NULLVal)
					a.push_back(v);

				str = str.substr(i + 1, str.length());
				break;
			}
		}
	}

	return a;
}

static Value DeserializeObj(const std::string& _str, std::stack<StackDepthType>& depth_stack)
{
	Object obj;

	std::string str = Trim(_str);

	if ((str[0] != '{') && (str[str.length() - 1] != '}'))
		return Value();
	else
		str = str.substr(1, str.length() - 2);

	while (str.length() > 0)
	{
		// Get the key name
		size_t start_quote_idx = GetQuotePos(str);
		size_t end_quote_idx = GetQuotePos(str, start_quote_idx + 1);
		size_t colon_idx = str.find(':', end_quote_idx);

		if ((start_quote_idx == std::string::npos) || (end_quote_idx == std::string::npos) || (colon_idx == std::string::npos))
			return Value();	// can't find key name
		
		std::string key = str.substr(start_quote_idx + 1, end_quote_idx - start_quote_idx - 1);
		if (key.length() == 0)
			return Value();

		bool had_error = false;
		str = str.substr(colon_idx + 1, str.length());
		obj[key] = DeserializeValue(str, &had_error, depth_stack);
		if (had_error)
			return Value();
	}

	return obj;
}

Value json::Deserialize(const std::string &str)
{
	std::stack<StackDepthType> depth_stack;
	return DeserializeInternal(str, depth_stack);
}

