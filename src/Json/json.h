#ifndef JSON_H
#define JSON_H

#include <string>
#include <vector>
#include <concepts>
#include <memory>
#include <variant>
#include <any>
#include <functional>
#include <utility>
#include <type_traits>

class Json;

template<typename T, typename... U>
concept any_of = std::disjunction_v<std::is_same<T, U>...>;

template<typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

using ChildrenList = std::vector<std::unique_ptr<Json>>;
using Bool = bool;
using Number = double;
using String = std::string;
using Array = std::vector<std::unique_ptr<Json>>;

using JsonValue = std::variant<String, Number, Bool, ChildrenList>;

using ProgresCallback = std::function<bool(size_t)>;

template<class T>
concept StringLike = std::is_convertible_v<T, std::string_view>;

template<class T>
concept Predicate = requires(T predicate, const Json & a)
{
  { predicate(a) } -> std::same_as<bool>;
};



class Json {
public:

  enum class ValueType {
    Undefined = -1,
    String,
    Number,
    Bool,
    Object,
    Array,
    Null,
  };

  Json();
  Json(std::string key, Json* parent);
  Json(const Json& obj);
  Json(Json&& obj) noexcept;
  Json& operator=(const Json& obj);
  Json& operator=(Json&& obj) noexcept;
  ~Json() = default;

  /* Accessors and mutators */
  Json::ValueType GetType() const;
  const std::string& GetKey() const;
  Json* GetParent() const;
  const JsonValue& GetValue() const;

  bool SetKey(std::string name);
  void SetParent(Json* parent);

  template<StringLike T>
  void SetValue(T&& data);

  template<Arithmetic T>
  void SetValue(T&& data);

  void SetValue(bool data);
  void ClearValue();
  std::unique_ptr<Json> Detach();
  bool RemoveChild(int index);

  bool IsValid() const;
  bool IsRoot() const;
  bool IsArrayElement() const;
  bool IsLastChild() const;
  Json* GetRoot();

  Json* operator[](std::string_view key);
  Json* operator[](int index);

  /* Object maniputaion methods */
  /* - Add child elements */
  template<typename T>
  Json* AddChild(T&& data, std::string key);

  template<typename T>
  Json* AddValue(T&& data);

  void ForEachChild(const std::function<void(const Json&)>& function) const;

  /* Json conversion to string */
  std::string ToString() const;

  /* Searching methods */
  template<Predicate T>
  Json* FindIf(const T& predicate);

  template<Predicate T>
  std::list<Json*> FindAllIf(const T& predicate)
  {
    std::list<Json*> list;

    auto result = predicate(*this);
    if (result) list.push_back(this);

    std::list<Json*> children_list;

    if (std::holds_alternative<ChildrenList>(value_))
    {
      auto& children = std::get<ChildrenList>(value_);
      for (auto& element : children)
      {
        children_list.splice(children_list.end(), element->FindAllIf(predicate));
      }
    }

    children_list.splice(children_list.end(), list);

    return children_list;
  }

  /* Parsing methods */
  static std::unique_ptr<Json> Parse(const std::string& data, const ProgresCallback = ProgresCallback());

private:

  /* Accessors and mutators */
  void SetType(ValueType type);
  void ConvertToArray();

  /* Json conversion to string */
  void ToString(std::string& str) const;

  Json* parent_;
  std::string key_;

  //Values
  ValueType value_type_;
  JsonValue value_;

  friend class JsonParser;
};


template<StringLike T>
void Json::SetValue(T&& data) {
  value_ = std::string(std::forward<T>(data));
  value_type_ = ValueType::String;
}

template<Arithmetic T>
void Json::SetValue(T&& data) {
  if (std::is_same_v<T, bool>)
  {
    value_ = static_cast<bool>(std::forward<T>(data));
    value_type_ = ValueType::Bool;
  }
  else {
    value_ = static_cast<double>(std::forward<T>(data));
    value_type_ = ValueType::Number;
  }
}


template<typename T>
Json* Json::AddChild(T&& data, std::string key)
{
  if (value_type_ != ValueType::Object && value_type_ != ValueType::Null) return nullptr;

  if (!std::holds_alternative<ChildrenList>(value_))
  {
    value_type_ = ValueType::Object;
    value_ = ChildrenList();
  }

  auto& children = std::get<ChildrenList>(value_);
  children.push_back(std::make_unique<Json>());

  auto& newObj = *children.back();

  newObj.parent_ = this;
  newObj.key_ = key;
  
  newObj.SetValue(std::forward<T>(data));

  return &newObj;
}

template<typename T>
Json* Json::AddValue(T&& data)
{
  if (value_type_ == ValueType::Null)
  {
    value_type_ = ValueType::Array;
    value_ = ChildrenList();
  }

  if (value_type_ != ValueType::Array) ConvertToArray();

  if (std::holds_alternative<ChildrenList>(value_))
  {
    auto& children = std::get<ChildrenList>(value_);
    children.push_back(std::make_unique<Json>());
    children.back()->SetValue(std::forward<T>(data));
    children.back()->SetParent(this);

    return children.back().get();
  }

  return nullptr;
}

template<Predicate T>
Json* Json::FindIf(const T& predicate) {
  Json* found = nullptr;
  auto result = predicate(*this);
  if (!result) {

    if (std::holds_alternative<ChildrenList>(value_))
    {
      auto& children = std::get<ChildrenList>(value_);
      for (auto& element : children)
      {
        found = element->FindIf(predicate);
      }
    }

  }
  else {
    found = this;
  }
  return found;
}

#endif // !JSON_H
