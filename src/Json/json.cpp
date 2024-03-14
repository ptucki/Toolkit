#include <iostream>
#include <cctype>
#include <cassert>
#include <string>
#include <format>
#include <chrono>

#include "json.h"
#include "json_parser.h"

Json::Json()
  : key_{ "" }
  , parent_{ nullptr }
  , value_type_{ ValueType::Null }
{
}

Json::Json(std::string key, Json* parent)
  : key_{ key }
  , parent_ { parent }
  , value_type_{ ValueType::Null }
{

}

Json::Json(const Json& obj)
: parent_{ nullptr }
, key_{obj.key_}
, value_type_{obj.value_type_}
{
  if (std::holds_alternative<String>(obj.value_)) value_ = std::get<String>(obj.value_);
  if (std::holds_alternative<Number>(obj.value_)) value_ = std::get<Number>(obj.value_);
  if (std::holds_alternative<Bool>(obj.value_)) value_ = std::get<Bool>(obj.value_);

  if (std::holds_alternative<ChildrenList>(obj.value_))
  {
    auto& objChildren = std::get<ChildrenList>(obj.value_);
    
    value_ = ChildrenList();
    auto& copyChildren = std::get<ChildrenList>(value_);
    copyChildren.reserve(objChildren.size());


    for (auto& json : objChildren)
    {
      copyChildren.push_back(std::make_unique<Json>(*json));
      copyChildren.back()->parent_ = this;
    }
  }
}

Json::Json(Json&& obj) noexcept
  : parent_{ nullptr }
  , key_{ "" }
  , value_type_{ ValueType::Undefined }
{
  std::swap(parent_, obj.parent_);
  std::swap(key_, obj.key_);
  std::swap(value_type_, obj.value_type_);
  std::swap(value_, obj.value_);
}

Json& Json::operator=(const Json& obj)
{
  if (std::holds_alternative<String>(obj.value_)) value_ = std::get<String>(obj.value_);
  if (std::holds_alternative<Number>(obj.value_)) value_ = std::get<Number>(obj.value_);
  if (std::holds_alternative<Bool>(obj.value_)) value_ = std::get<Bool>(obj.value_);

  if (std::holds_alternative<ChildrenList>(obj.value_))
  {
    auto& objChildren = std::get<ChildrenList>(obj.value_);

    value_ = ChildrenList();
    auto& copyChildren = std::get<ChildrenList>(value_);
    copyChildren.reserve(objChildren.size());

    for (auto& json : objChildren)
    {
      copyChildren.push_back(std::make_unique<Json>(*json));
      copyChildren.back()->parent_ = this;
    }
  }

  return *this;
}

Json& Json::operator=(Json&& obj) noexcept
{
  parent_ = obj.parent_;
  key_ = obj.key_;
  value_type_ = obj.value_type_;

  obj.parent_ = nullptr;
  obj.key_ = "";
  obj.value_type_ = ValueType::Undefined;
  value_ = std::move(obj.value_);

  return *this;
}


std::unique_ptr<Json> Json::Parse(const std::string& data, ProgresCallback progress_callback)
{
  JsonParser parser;
  return parser.Parse(data, progress_callback);
}


bool Json::SetKey(std::string key)
{
  auto parent = this->GetParent();
  bool key_exists = false;

  if (std::holds_alternative<ChildrenList>(parent->value_))
  {
    auto& children = std::get<ChildrenList>(parent->value_);

    for (auto& child : children) {
      if (child->GetKey() == key) return false;
    }

  }
  key_ = key_exists ? key_ : key;
  return true;
}

void Json::SetType(ValueType type)
{
  value_type_ = type;
}

void Json::ConvertToArray()
{
  std::unique_ptr<Json> copy = std::make_unique<Json>(std::move(*this));
  this->SetType(ValueType::Array);
  this->key_ = copy->key_;
  this->parent_ = copy->parent_;

  copy->key_ = "";
  copy->SetParent(this);
  this->value_ = ChildrenList();
  auto& children = std::get<ChildrenList>(value_);

  children.push_back(std::move(copy));
}

void Json::ToString(std::string& str) const
{

  switch (value_type_)
  {
  case Json::ValueType::String:
    {
      auto& value = std::get<String>(value_);

      if (IsArrayElement())
      {
        str += std::format("\"{}\"", value);
      }
      else
      {
        str += std::format("\"{}\":\"{}\"", key_, value);
      }
    }
    break;

  case Json::ValueType::Number:
    {
      auto& value = std::get<Number>(value_);

      if (IsArrayElement())
      {
        str += std::to_string(value);
      }
      else
      {
        str += std::format("\"{}\":{}", key_, std::to_string(value));
      }
    }
    break;

  case Json::ValueType::Bool:
    {
      auto& value = std::get<Bool>(value_);

      if (IsArrayElement())
      {
        str += (value == true ? "true" : "false");
      }
      else
      {
        std::string bool_value = value == true ? "true" : "false";
        str += std::format("\"{}\":{}", key_, bool_value);
      }
    }
    break;

  case Json::ValueType::Object:
    {
      if (IsArrayElement() || IsRoot())
      {
        str.append(1, '{');
      }
      else
      {
        str += std::format("\"{}\":{{", key_);
      }
      
      ForEachChild([&str](const Json& child) -> void {
        child.ToString(str);
        if (!child.IsLastChild()) str.append(1, ',');
        });
      str.append(1, '}');
    }
    break;

  case Json::ValueType::Array:

    if (IsArrayElement())
    {
      str.append(1, '[');
    }
    else
    {
      str += std::format("\"{}\":[", key_);
    }

    ForEachChild([&str](const Json& child) -> void {
      child.ToString(str);
      if (!child.IsLastChild()) str.append(1, ',');
      });
    str.append(1, ']');
    break;

  case Json::ValueType::Null:
  {
    if (IsArrayElement())
    {
      str += "null";
    }
    else
    {
      str += std::format("\"{}\":null", key_);
    }
  }
    break;

  default:
    str.clear();
    return;
  }
}

Json::ValueType Json::GetType() const
{
  return value_type_;
}

const std::string& Json::GetKey() const
{
  return key_;
}

Json* Json::GetParent() const
{
  return parent_;
}

const JsonValue& Json::GetValue() const
{
  return value_;
}

void Json::SetParent(Json* parent)
{
  parent_ = parent;
}

void Json::SetValue(bool data)
{
  value_ = data;
  value_type_ = ValueType::Bool;
}

void Json::ClearValue()
{
  value_ = JsonValue();
  value_type_ = ValueType::Null;
}

std::unique_ptr<Json> Json::Detach()
{
  if (this->IsRoot()) return std::unique_ptr<Json>(nullptr);

  std::unique_ptr<Json> json;

  auto parent = this->GetParent();

  auto& value = parent->value_;

  if (std::holds_alternative<ChildrenList>(value))
  {
    auto& children = std::get<ChildrenList>(value);

    for (auto it = std::begin(children); it != std::end(children); it++)
    {
      if (it->get() == this)
      {
        json = std::unique_ptr<Json>(it->release());
        children.erase(it);
        return json;
      }
    }
  }

  return std::unique_ptr<Json>(nullptr);
}

bool Json::RemoveChild(int index)
{
  if (!std::holds_alternative<ChildrenList>(value_)) return false;

  auto& children = std::get<ChildrenList>(value_);
  children.erase(std::begin(children) + index);

  return true;
}

bool Json::IsValid() const
{
  return value_type_ == ValueType::Undefined ? false : true;
}

bool Json::IsRoot() const
{
  return parent_ == nullptr ? true : false;
}

bool Json::IsArrayElement() const
{
  if (parent_ == nullptr) return false;
  return parent_->GetType() == ValueType::Array;
}

bool Json::IsLastChild() const
{
  auto& parents_value = std::get<ChildrenList>(parent_->value_);
  return parents_value.back().get() == this;
}

Json* Json::GetRoot()
{
  auto node = this;
  while (node->GetParent() != nullptr)
  {
    node = node->GetParent();
  }

  return node;
}

Json* Json::operator[](std::string_view key)
{
  if (value_type_ != ValueType::Object) return nullptr;
  if (!std::holds_alternative<ChildrenList>(value_)) return nullptr;

  for (auto& child : std::get<ChildrenList>(value_))
  {
    if (child->GetKey() == key) return child.get();
  }

  return nullptr;
}

Json* Json::operator[](int index)
{
  if (value_type_ != ValueType::Object && value_type_ != ValueType::Array) return nullptr;
  if (!std::holds_alternative<ChildrenList>(value_)) return nullptr;



  return std::get<ChildrenList>(value_)[index].get();
}

void Json::ForEachChild(const std::function<void(const Json&)>& function) const
{
  if (std::holds_alternative<ChildrenList>(value_))
  {
    auto& children = std::get<ChildrenList>(value_);

    for (auto& element : children)
    {
      function(*element);
    }
  }
}

std::string Json::ToString() const
{
  std::string json_string;

  this->ToString(json_string);

  return json_string;
}

