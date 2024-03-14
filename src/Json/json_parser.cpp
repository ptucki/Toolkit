#include <variant>

#include "json_parser.h"


JsonParser::JsonParser()
  : parsing_state_{ ParsingState::Undefined }
  , stop_flag_{ false }
{
}

Json* JsonParser::AddNewPair(Json* current)
{
  if (!std::holds_alternative<ChildrenList>(current->value_))
  {
    current->value_ = ChildrenList();
  }
  auto& list = std::get<ChildrenList>(current->value_);
  list.push_back(std::make_unique<Json>("", current));
  return list.back().get();
}

void JsonParser::SetParsedValue(const std::string& value, Json* current)
{
  switch (current->value_type_)
  {
  case Json::ValueType::String:
    current->value_ = value;
    break;
  case Json::ValueType::Number:
    current->value_ = std::stod(value);
    break;
  default:
    break;
  }

}

std::unique_ptr<Json> JsonParser::Parse(const std::string& data, const ProgresCallback& progress_callback)
{
  auto root_ = std::make_unique<Json>("", nullptr);
  root_->SetType(Json::ValueType::Undefined);
  auto current = root_.get();

  auto it = std::begin(data);
  auto end = std::end(data);

  parsing_state_ = ParsingState::Started;
  ProgressManager progress_manager{ data.size(), progress_callback, it, end, parsing_state_, stop_flag_};

  for (; it != end; it++)
  {
    switch (*it)
    {
    case '\n':
    case '\f':
    case '\r':
    case '\t':
    case '\v':
    case ' ':
      //ignore trailing whitespaces
      while (it != end && std::isspace(*it)) it++;
      it--;
      break;

    case '{':
      current->SetType(Json::ValueType::Object);
      parsing_state_ = ParsingState::Object;
      break;

    case '[':
      current->SetType(Json::ValueType::Array);
      current->value_ = ChildrenList();
      parsing_state_ = ParsingState::Array;
      break;

    default:
      parsing_state_ = ParsingState::Undefined;
      break;
    }

    if (parsing_state_ != ParsingState::Undefined
      && parsing_state_ != ParsingState::Finished
      && parsing_state_ != ParsingState::Started)
    {
      if ((it + 1) != end)
      {
        std::invoke(GetParsingMethod(parsing_state_), this, ++it, end, current);
        parsing_state_ = current->IsRoot() ? ParsingState::Finished : parsing_state_.load();
      }
      else
      {
        parsing_state_ = ParsingState::Undefined;
      }
    }

    if (parsing_state_ == ParsingState::Undefined)
    {
      root_ = std::make_unique<Json>();
      root_->SetType(Json::ValueType::Undefined);
      break;
    }
  }

  return root_;
}



void JsonParser::ParseString(char_iterator& ch, char_iterator& end, Json* current)
{
  std::string value = "";
  ++ch;

  while (ch != end && !stop_flag_)
  {
    switch (*ch)
    {
    case '\n':
    case '\f':
    case '\r':
    case '\t':
    case '\v':
      parsing_state_ = ParsingState::Undefined;
      return;

    case '\\':
      parsing_state_ = ParsingState::EscapeChar;
      ParseEscapeChar(ch, end, value);
      break;

    case '\"':
      if (current->GetType() == Json::ValueType::Object)
      {
        if (parsing_state_ == ParsingState::Key) {
          current->SetKey(value);
        }
        parsing_state_ = ParsingState::Object;
        return;
      }
      else if (current->GetType() == Json::ValueType::String)
      {
        SetParsedValue(value, current);
        parsing_state_ = ParsingState::Object;
        return;
      }

      else if (current->GetType() == Json::ValueType::Array) parsing_state_ = ParsingState::Array;
      return;
    default:
      value.append(1, *ch);

      break;
    }
    ch++;
  }
}

void JsonParser::ParseNumber(char_iterator& ch, char_iterator& end, Json* current)
{
  std::string value = "";
  bool decimal_point = false;
  value.append(1, *ch);
  ++ch;

  while (ch != end && !stop_flag_)
  {
    switch (*ch)
    {
    case '\n':
    case '\f':
    case '\r':
    case '\t':
    case '\v':
    case ' ':
    case ',':
    case '}':
    case ']':
      if (value.back() >= '0' || value.back() <= '9')
      {
        SetParsedValue(value, current);
        ch--;
        parsing_state_ = ParsingState::Object;
      }
      else
      {
        parsing_state_ = ParsingState::Undefined;
      }
      return;

    case '.':
      if (value.size() == 1 && value[0] == '-' || decimal_point == true)
      {
        parsing_state_ = ParsingState::Undefined;
      }
      else
      {
        decimal_point = true;
        value.append(1, *ch);
      }
      break;

    case 'e':
    case 'E':
      if (value.back() >= '0' || value.back() <= '9')
      {
        value.append(1, *ch);
      }
      else
      {
        parsing_state_ = ParsingState::Undefined;
        return;
      }
      break;

    case '-':
      if (value.back() >= 'E' || value.back() <= 'e')
      {
        parsing_state_ = ParsingState::Undefined;
        return;
      }
      else
      {
        value.append(1, *ch);
      }
      break;

    default:
      if (*ch >= '0' || *ch <= '9')
      {
        value.append(1, *ch);
      }

      break;
    }
    ch++;
  }
}

void JsonParser::ParseEscapeChar(char_iterator& ch, char_iterator& end, std::string& str)
{
  ++ch;

  switch (*ch)
  {
  case 'n':
    str.append(1, '\n');
    break;

  case 't':
    str.append(1, '\t');
    break;

  case 'f':
    str.append(1, '\f');
    break;

  case 'v':
    str.append(1, '\v');
    break;

  case 'b':
    str.append(1, '\b');
    break;

  case 'r':
    str.append(1, '\r');
    break;

  case '\\':
    str.append(1, '\\');
    break;
  case '\"':
    str.append(1, '\"');
    break;

  case '\'':
    str.append(1, '\'');
    break;

  case '\?':
    str.append(1, '\?');
    break;

  case '\a':
    str.append(1, '\a');
    break;

  default:
    parsing_state_ = ParsingState::Undefined;
    break;
  }
  parsing_state_ = ParsingState::String;
}

void JsonParser::ParseArray(char_iterator& ch, char_iterator& end, Json* current)
{
  while (ch != end && !stop_flag_)
  {
    switch (*ch)
    {
    case '\n':
    case '\f':
    case '\r':
    case '\t':
    case '\v':
    case ' ':
      break;
    case ']':
    {
      auto& list = std::get<ChildrenList>(current->value_);
      if (list.size() == 1 &&
        (*list.back()).GetType() == Json::ValueType::Undefined)
      {
        list.pop_back();
      }
    }
    return;

    case ',':
      break;

    case '\"':
    case '[':
    case 't':
    case 'f':
    case 'n':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '0':
    case '-':
      current = AddNewPair(current);
      parsing_state_ = ParsingState::Value;
      std::invoke(GetParsingMethod(parsing_state_), this, ch, end, current);
      current = current->parent_;
      break;

    case '{':
      parsing_state_ = ParsingState::Value;
      current = AddNewPair(current);
      current->SetType(Json::ValueType::Object);
      std::invoke(GetParsingMethod(parsing_state_), this, ch, end, current);
      current = current->parent_;
      break;

    default:
      break;
    }

    ch++;
  }

}

void JsonParser::ParseObject(char_iterator& ch, char_iterator& end, Json* current)
{
  current = AddNewPair(current);

  current->SetType(Json::ValueType::Object);
  std::string name = "";
  bool is_key_set = false;

  while (ch != end && !stop_flag_)
  {
    switch (*ch)
    {
    case '\n':
    case '\f':
    case '\r':
    case '\t':
    case '\v':
    case ' ':
      break;

    case '\"':
      if (parsing_state_ == ParsingState::Object)
      {
        parsing_state_ = ParsingState::Key;
        std::invoke(GetParsingMethod(parsing_state_), this, ch, end, current);
        is_key_set = true;
      }
      else {
        parsing_state_ = ParsingState::Undefined;
        return;
      }
      break;

    case '}':
    {
      current = current->parent_;
      auto& list = std::get<ChildrenList>(current->value_);
      if (list.size() > 0 && list[0]->key_ == "")
      {
        list.pop_back();
      }
    }
    return;

    case ':':
      if (is_key_set)
      {
        parsing_state_ = ParsingState::Value;
        std::invoke(GetParsingMethod(parsing_state_), this, ++ch, end, current);
      }
      else
      {
        parsing_state_ = ParsingState::Undefined;
      }
      break;

    case ',':
      current = current->parent_;
      current = AddNewPair(current);
      current->SetType(Json::ValueType::Object);
      break;

    default:
      name.append(1, *ch);
      break;
    }

    ch++;
  }

}

void JsonParser::ParseValue(char_iterator& ch, char_iterator& end, Json* current)
{
  std::string value = "";

  while (ch != end && !stop_flag_)
  {
    switch (*ch)
    {
    case '\n':
    case '\f':
    case '\r':
    case '\t':
    case '\v':
    case ' ':
      break;

    case '\"':
      current->SetType(Json::ValueType::String);
      parsing_state_ = ParsingState::String;
      std::invoke(GetParsingMethod(parsing_state_), this, ch, end, current);
      return;
      break;

    case 't':
      if (ExpectKeyword(ch, end, "true"))
      {
        current->SetType(Json::ValueType::Bool);
        current->value_ = true;
        parsing_state_ = ParsingState::Object;
        ch += 3;
      }
      else
      {
        parsing_state_ = ParsingState::Undefined;
      }
      return;

    case 'f':
      if (ExpectKeyword(ch, end, "false"))
      {
        current->SetType(Json::ValueType::Bool);
        current->value_ = false;
        parsing_state_ = ParsingState::Object;
        ch += 4;
      }
      else
      {
        parsing_state_ = ParsingState::Undefined;
      }
      return;

    case 'n':
      if (ExpectKeyword(ch, end, "null"))
      {
        current->SetType(Json::ValueType::Null);
        parsing_state_ = ParsingState::Object;
        ch += 3;
      }
      else
      {
        parsing_state_ = ParsingState::Undefined;
      }
      return;

    case '{':
      parsing_state_ = ParsingState::Object;
      std::invoke(GetParsingMethod(parsing_state_), this, ch, end, current);
      return;

    case '[':
      current->SetType(Json::ValueType::Array);
      parsing_state_ = ParsingState::Array;
      current->value_ = ChildrenList();
      std::invoke(GetParsingMethod(parsing_state_), this, ++ch, end, current);
      return;

    case '-':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '0':
      current->SetType(Json::ValueType::Number);
      parsing_state_ = ParsingState::Number;
      std::invoke(GetParsingMethod(parsing_state_),this, ch, end, current);
      return;

    default:
      value.append(1, *ch);
      break;
    }

    ch++;
  }
}

JsonParser::ParsingMethodType JsonParser::GetParsingMethod(ParsingState state)
{
  switch (state)
  {
  case ParsingState::Object:      return &JsonParser::ParseObject;
  case ParsingState::Array:       return &JsonParser::ParseArray;
  case ParsingState::Key:
  case ParsingState::String:      return &JsonParser::ParseString;
  case ParsingState::Number:      return &JsonParser::ParseNumber;
  case ParsingState::Value:       return &JsonParser::ParseValue;
  }
  return nullptr;
}

bool JsonParser::ExpectKeyword(char_iterator& ch, char_iterator& end, std::string keyword)
{
  std::string temp = "";

  if (static_cast<size_t>(end - ch) > keyword.size())
  {
    temp.append(ch, ch + keyword.size());
    if (keyword == temp)
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  return false;
}

JsonParser::ProgressManager::~ProgressManager()
{
  stop_flag_ = true;
}

const std::atomic<bool>& JsonParser::ProgressManager::isStopped() const
{
  return stop_flag_;
}

JsonParser::ProgressManager::ProgressManager (
  size_t data_size,
  const ProgresCallback& progress,
  char_iterator& begin,
  char_iterator& end,
  std::atomic<ParsingState>& parsing_state,
  std::atomic<bool>& stop_flag)
  : data_size_{ data_size }
  , callback_{ progress }
  , stop_flag_{ stop_flag }
  , begin_{ begin }
  , end_{ end }
  , parsing_state_{ parsing_state }
{
  if (callback_)
  {
    thread_ = std::jthread(&ProgressManager::LoopProgressRead, this);
  }
}

void JsonParser::ProgressManager::InvokeCallback(size_t progress)
{
  if (callback_)
  {
    stop_flag_ = !callback_(progress);
  }
}

void JsonParser::ProgressManager::LoopProgressRead()
{
  std::chrono::milliseconds timespan(JSON_PROGESS_READ_TIME);

  while (IsParsingInProgress())
  {
    auto current_progress = GetCurrentProgress();
    InvokeCallback(current_progress);
    std::this_thread::sleep_for(timespan);
  }
  callback_(GetCurrentProgress());
}

bool JsonParser::ProgressManager::IsParsingInProgress() const
{
  constexpr auto underlying = [](auto value) {
    return std::underlying_type_t<ParsingState>(value);
  };
  return underlying(parsing_state_.load()) > underlying(JsonParser::ParsingState::Finished) && !stop_flag_;
}

size_t JsonParser::ProgressManager::GetCurrentProgress() const
{
  return (100* (data_size_ - std::distance(begin_, end_))) / data_size_;
}
