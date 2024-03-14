#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <string>
#include <memory>
#include <thread>
#include <iostream>
#include "json.h"

#define JSON_PROGESS_READ_TIME 1000



class JsonParser
{
  using char_iterator = std::string::const_iterator;
  using ParsingMethodType = void(JsonParser::*)(char_iterator&, char_iterator&, Json*);

public:
  JsonParser();
  std::unique_ptr<Json> Parse(const std::string& data, const ProgresCallback& progress_callback);

private:
  enum class ParsingState {
    Undefined = -1,
    Interrupted,
    Finished,
    Started,

    Object = 10,
    Array,
    Key,
    Value,
    String,
    Number,
    BooleanNull,
    Null,
    EscapeChar };

  class ProgressManager {
  public:
    ProgressManager(
      size_t data_size,
      const ProgresCallback& progress,
      char_iterator& begin,
      char_iterator& end,
      std::atomic<ParsingState>& parsing_state,
      std::atomic<bool>& stop_flag
    );

    ~ProgressManager();

    const std::atomic<bool>& isStopped() const;
  private:

    void InvokeCallback(size_t progress);
    void LoopProgressRead();
    bool IsParsingInProgress() const;
    size_t GetCurrentProgress() const;

    size_t data_size_;
    ProgresCallback callback_;
    std::jthread thread_;
    std::atomic<bool>& stop_flag_;
    const std::atomic<ParsingState>& parsing_state_;
    const char_iterator& begin_;
    const char_iterator& end_;
  };

  /* Parsing methods */
  void ParseString(char_iterator& ch, char_iterator& end, Json* current);
  void ParseArray(char_iterator& ch, char_iterator& end, Json* current);
  void ParseObject(char_iterator& ch, char_iterator& end, Json* current);
  void ParseValue(char_iterator& ch, char_iterator& end, Json* current);
  void ParseNumber(char_iterator& ch, char_iterator& end, Json* current);
  void ParseEscapeChar(char_iterator& ch, char_iterator& end, std::string& str);
  ParsingMethodType GetParsingMethod(ParsingState state);
  bool ExpectKeyword(char_iterator& ch, char_iterator& end, std::string expected_value);

  /* Maniputaion methods */
  void SetParsedValue(const std::string& value, Json* current);
  Json* AddNewPair(Json* current);
  std::atomic<bool> stop_flag_;
  std::atomic<ParsingState> parsing_state_;
};



#endif // !JSON_PARSER_H
