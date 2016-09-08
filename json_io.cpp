/***************************************************************************
 ***************************************************************************
  (c) 1999-2006 Ganymede Technologies                 All Rights Reserved
      Krakow, Poland                                  www.ganymede.eu
 ***************************************************************************
 ***************************************************************************/

#include "headers_all.h"
#include "glogic_lasttemple.h"

/***************************************************************************/
void try_cleanup_json_content(const std::string & name, json::Object & json_data)
{
	json::Object::iterator it = json_data.Find(name);
	if (it != json_data.End()) json_data.Erase(it);
}
/***************************************************************************/
json::Object get_json_from_string(const std::string & json_data)
{
	json::Object result;
	result.Clear();
	try
	{
		std::istringstream istr(json_data);
		json::Reader::Read(result, istr);
	}
	catch (json::Reader::ScanException & )
	{
		result.Clear();
	}
	catch (json::Reader::ParseException & )
	{
		result.Clear();
	}
	return result;
}
/***************************************************************************/
string write_json_to_string(json::Object & o)
{
	std::stringstream ostr;
	json::Writer::Write(o, ostr);
	return ostr.str();
}
/***************************************************************************/
INT32 get_INT32_from_json(const json::Object & o, const char * key, INT32 default_value)
{
	INT32 result = default_value;

	try
	{
		if (o.Find(key) != o.End())
		{
			result = (INT32)json::Number(o[key]);
		}
	}
	catch (json::Exception & )
	{
		result = default_value;
	}

	return result;
}
/***************************************************************************/
INT64 get_INT64_from_json(const json::Object & o, const char * key, INT64 default_value)
{
	INT64 result = default_value;

	try
	{
		if (o.Find(key) != o.End())
		{
			result = (INT64)json::Number(o[key]);
		}
	}
	catch (json::Exception & )
	{
		result = default_value;
	}

	return result;
}
/***************************************************************************/
string get_string_from_json(const json::Object & o, const char * key, const char * default_value)
{
	string result = default_value;

	try
	{
		if (o.Find(key) != o.End())
		{
			result = json::String(o[key]);
		}
	}
	catch (json::Exception & )
	{
		result = default_value;
	}

	return result;
}
/***************************************************************************/
void merge_objects(json::Object & o, const json::Object & new_fields, const string & preserve_field)
{
	json::Object::const_iterator it;
	for (it = new_fields.Begin(); it != new_fields.End(); it++)
	{
		if (preserve_field != "" && it->name == preserve_field)
		{
			continue;
		}
		if (o.Find(it->name) == o.End())
		{
			o[it->name] = new_fields[it->name];
		}
		else
		{
			try
			{
				merge_objects(o[it->name], new_fields[it->name], "");
			}
			catch (json::Exception & )
			{
				o[it->name] = new_fields[it->name];
			}
		}
	}
}
/***************************************************************************/
