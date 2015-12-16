//#include <variable.h>

template <typename T>
variable<T>::variable()
	//  : s_val(), v_val()
	      : s_val()
{
}

template <typename T>
variable<T>::variable( const std::string _name, const T& _val)
//  : s_val(), v_val()
  : s_val()
{
  //it's a variable...
  s_val = _val;
  name = _name;
  mytype = VAR;
}

template <typename T>
variable<T>::variable( const std::string _name, const std::vector<T>& _val)
//  :  s_val(), v_val()
  :  v_val()
{
  //it's a variable...
  v_val = _val;
  name = _name;
  mytype = ARRAY;
}
