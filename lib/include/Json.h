// Json.h
#ifndef JSON_H
#define JSON_H

#include <node.h>
#include <node_object_wrap.h>
#include <string>
#include <vector>

// Node object accessible from nodejs
class Json : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> exports);
  static Json* CreateNewJson();
  v8::JSObject Object();
  void addProperty(std::string name, std::string value);

 private:
  explicit Json();
  ~Json();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Object(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;
  v8::JSOject object_;
};

#endif
