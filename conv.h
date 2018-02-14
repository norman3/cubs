//------------------------------------------------------------------------------
// @file cubs/to_string.h
//------------------------------------------------------------------------------
#ifndef CUBS_TO_STRING_H_
#define CUBS_TO_STRING_H_
#include <stdexcept>
#include <sstream>
#include <string>

namespace cubs {

//------------------------------------------------------------------------------
// @method str(const T& val);
//------------------------------------------------------------------------------

// Builtin 으로 지원하지 않는 경우에 대한 메서드 선언.
template <typename T,
          typename = typename std::enable_if<
            !(std::is_arithmetic<T>::value
              && !(is_same<T, bool>::value),
              && !(is_same<T, char>::value))
            && !std::is_array<T>::value
            && !std::is_same<T, std::string>::value
          >::type>
inline std::string to_string(T val);

template <> inline std::string to_string<bool>(bool val) {
  return val ? "true" : "false" ;
}

template <> inline std::string to_string<char>(char val) {
  return std::string(1, val);
}

// `bool`, `char` 를 제외한 일반적인 실수 타입 변환.
template <typename T>
inline std::string to_string(const T& val,
                             typename std::enable_if<
                             std::is_arithmetic<T>::value
                             && !(is_same<T, bool>::value),
                             && !(is_same<T, char>::value)>::type* = 0) {
  return std::to_string(val);
}

// array type 에 대한 변환은 에러 처리.
template <typename T>
inline std::string to_string(const T& val,
                             typename std::enable_if<
                             std::is_array<T>::value>::type* = 0) {
  // throw std::bad_alloc exception.
  return std::to_string(val);
}

template <typename T>
inline std::string to_string_or_else(const T& val, const std::string& base = "<?>") {
  
}







std::is_arithmetic<T>::value && !std::is_same<T, bool>::value && !std::is_same<T, char>::value

not (std::is_arithmetic<T>::value
      and not std::is_same<T, bool>::value
      and not std::is_same<T, char>::value
      )
  and not std::is_array<T>::value
  and not std::is_same<T, std::string>::value
  and not std::is_pointer<T>::value
  >::type

template <typename T>
inline std::string str(const T& val, typename = typename std::enable_if<
                       ! (


}  // namespace cubs

#endif  // CUBS_TO_STRING_H_
