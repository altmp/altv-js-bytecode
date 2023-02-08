#pragma once

#include "ConfigBase.h"
#include "toml.hpp"

class TomlConfig : public Config::ConfigBase<TomlConfig>
{
public:
    static ValuePtr ToValue(toml::node* node)
    {
        if (node->is_string())
        {
            return std::make_shared<Value>(node->as_string()->get());
        }
        if (node->is_number())
        {
            double val;
            if (node->is_integer()) val = node->as_integer()->get();
            else val = node->as_floating_point()->get();
            return std::make_shared<Value>(val);
        }
        if (node->is_boolean())
        {
            return std::make_shared<Value>(node->as_boolean()->get());
        }
        if (node->is_array())
        {
            Value::List value;
            auto list = node->as_array();
            for (size_t i = 0; i < list->size(); i++)
            {
                value.push_back(ToValue(list->get(i)));
            }
            return std::make_shared<Value>(value);
        }
        if (node->is_table())
        {
            Value::Dict value;
            auto dict = node->as_table();
            for (auto it = dict->begin(); it != dict->end(); ++it)
            {
                value.insert({ it->first.data(), ToValue(&it->second) });
            }
            return std::make_shared<Value>(value);
        }
        return std::make_shared<Value>();
    }
    static toml::node* FromValue(ValuePtr& value)
    {
        if (value->IsString()) return new toml::value(value->AsString());
        if (value->IsBool()) return new toml::value(value->AsBool());
        if (value->IsNumber())
        {
            double val = value->AsNumber();
            // Weird hack to check if the number has any decimal points
            if (abs(val - (int64_t)val) == 0) return new toml::value(value->AsNumber<int64_t>());
            return new toml::value(val);
        }
        if (value->IsList())
        {
            toml::array* list = new toml::array();
            list->reserve(value->GetSize());
            for (auto val : value->AsList())
            {
                list->push_back(*FromValue(val));
            }
            return list;
        }
        if (value->IsDict())
        {
            toml::table* dict = new toml::table();
            for (auto& pair : value->AsDict())
            {
                dict->insert(pair.first, *FromValue(pair.second));
            }
            return dict;
        }
        return nullptr;
    }

    static ValuePtr Parse(const std::string& input, std::string& error)
    {
        try
        {
            auto data = toml::parse(input);
            return ToValue(data.as_table());
        }
        catch (toml::parse_error& e)
        {
            auto description = e.description();
            auto& source = e.source();
            std::stringstream errStream;
            errStream << "Failed to parse TOML config file, line: " << source.begin.line << ", column: " << source.begin.column << " (" << description << ")";
            error = errStream.str();
            return std::make_shared<Value>();
        }
    }
    static std::string Format(ValuePtr config)
    {
        toml::node* node = FromValue(config);
        if (!node) return "";
        std::stringstream stream;
        stream << toml::toml_formatter(*node);
        delete node;
        return stream.str();
    }
};