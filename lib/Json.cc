// Graph.cc
#include "Graph.h"
#include <string>
#include <iostream>
#include "../include/Json.h"

using namespace v8;

Persistent<Function> Json::constructor;

Json::Json() {
}

Json::~Json() {
}

Json* Json::CreateNewJson() {
  return new Json();
}

void Node::Object(v8::JSObject obj) {
  object_ = obj;
}

v8::JSObject Json::Object() { return object_; }

void Node::Init(Handle<Object> exports) {
  Isolate* isolate = Isolate::GetCurrent();

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "Json"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  NODE_SET_PROTOTYPE_METHOD(tpl, "getObject", Object);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "Json"),
               tpl->GetFunction());
}

void Json::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  if (args.IsConstructCall()) {
    Local<String> str = args[0]->IsUndefined() ? Local<String>(String::NewFromUtf8(isolate, "")) : args[0]->ToString();
    String::Utf8Value str_value(str->ToString());
    std::string tmp = std::string(*str_value);

    Local<String> str2 = args[1]->IsUndefined() ? Local<String>(String::NewFromUtf8(isolate, "")) : args[1]->ToString();
    String::Utf8Value str2_value(str2->ToString());
    std::string tmp2 = std::string(*str2_value);

    int width = args[2]->IsUndefined() ? 0 : args[2]->NumberValue();
    int height = args[3]->IsUndefined() ? 0 : args[3]->NumberValue();

    Node* obj = new Node(tmp, tmp2, width, height);
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    // Invoked as plain function `MyObject(...)`, turn into construct call.
    const int argc = 2;
    Local<Value> argv[argc] = { args[0], args[1] };
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(cons->NewInstance(argc, argv));
  }
}

void Node::Key(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  Node* obj = ObjectWrap::Unwrap<Node>(args.Holder());
  //v8::String::Utf8Value param1(obj->key->ToString());
  //std::string str = std::string(*param1);
  args.GetReturnValue().Set(Local<Value>(String::NewFromUtf8(isolate, obj->key_.c_str())));
}

void Node::Label(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  Node* obj = ObjectWrap::Unwrap<Node>(args.Holder());
  //v8::String::Utf8Value param1(obj->key->ToString());
  //std::string str = std::string(*param1);
  args.GetReturnValue().Set(Local<Value>(String::NewFromUtf8(isolate, obj->label_.c_str())));
}

void Node::X(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  Node* obj = ObjectWrap::Unwrap<Node>(args.Holder());

  std::cout << "Wrapped Node X: " << obj->x_ << std::endl;

  args.GetReturnValue().Set(Number::New(isolate, obj->x_));
}

void Node::Y(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  Node* obj = ObjectWrap::Unwrap<Node>(args.Holder());

  args.GetReturnValue().Set(Number::New(isolate, obj->y_));
}

void Node::Width(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  Node* obj = ObjectWrap::Unwrap<Node>(args.Holder());

  args.GetReturnValue().Set(Number::New(isolate, obj->width_));
}

void Node::Height(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  Node* obj = ObjectWrap::Unwrap<Node>(args.Holder());

  args.GetReturnValue().Set(Number::New(isolate, obj->height_));
}

Bend::Bend(double x, double y) : x_(x), y_(y) {
}

Bend* Bend::CreateNewBend(double x, double y) {
  Isolate* isolate = Isolate::GetCurrent();
  const int argc = 2;
  Local<Value> arg = Number::New(isolate, x);
  Local<Value> arg1 = Number::New(isolate, y);
  Local<Value> argv[argc] = { arg, arg1 };
  Local<Function> cons = Local<Function>::New(isolate, constructor);
  Handle<Object> obj = cons->NewInstance(argc, argv);
  Bend* bend = ObjectWrap::Unwrap<Bend>(obj);
  return bend;
}

void Bend::Init(Handle<Object> exports) {
  Isolate* isolate = Isolate::GetCurrent();

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "Bend"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  NODE_SET_PROTOTYPE_METHOD(tpl, "getX", X);
  NODE_SET_PROTOTYPE_METHOD(tpl, "getY", Y);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "Bend"),
               tpl->GetFunction());
}

void Bend::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  if (args.IsConstructCall()) {

    double x = args[0]->IsUndefined() ? 0 : args[0]->NumberValue();
    double y = args[1]->IsUndefined() ? 0 : args[1]->NumberValue();

    Bend* obj = new Bend(x, y);
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    // Invoked as plain function `MyObject(...)`, turn into construct call.
    const int argc = 2;
    Local<Value> argv[argc] = { args[0], args[1] };
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(cons->NewInstance(argc, argv));
  }
}

void Bend::X(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  Bend* obj = ObjectWrap::Unwrap<Bend>(args.Holder());

  std::cout << "Wrapped Node X: " << obj->x_ << std::endl;

  args.GetReturnValue().Set(Number::New(isolate, obj->x_));
}

void Bend::Y(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  Bend* obj = ObjectWrap::Unwrap<Bend>(args.Holder());

  args.GetReturnValue().Set(Number::New(isolate, obj->y_));
}

Bend::~Bend() {
}

Edge::Edge(std::string source, std::string dest) : source_(source), dest_(dest) {
}

Edge::~Edge() {
}

std::string Edge::Source() { return source_; }
std::string Edge::Dest() { return dest_; }
std::vector<Bend*> Edge::Bends() { return bends_; }

void Edge::Init(Handle<Object> exports) {
  Isolate* isolate = Isolate::GetCurrent();

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "Edge"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  NODE_SET_PROTOTYPE_METHOD(tpl, "getSource", Source);
  NODE_SET_PROTOTYPE_METHOD(tpl, "getDest", Dest);
  NODE_SET_PROTOTYPE_METHOD(tpl, "getBends", Bends);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "Edge"),
               tpl->GetFunction());
}

void Edge::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  if (args.IsConstructCall()) {
    Local<String> str = args[0]->IsUndefined() ? Local<String>(String::NewFromUtf8(isolate, "")) : args[0]->ToString();
    String::Utf8Value str_value(str->ToString());
    std::string tmp = std::string(*str_value);

    Local<String> str2 = args[1]->IsUndefined() ? Local<String>(String::NewFromUtf8(isolate, "")) : args[1]->ToString();
    String::Utf8Value str2_value(str2->ToString());
    std::string tmp2 = std::string(*str2_value);

    Edge* obj = new Edge(tmp, tmp2);
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    // Invoked as plain function `MyObject(...)`, turn into construct call.
    const int argc = 2;
    Local<Value> argv[argc] = { args[0], args[1] };
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(cons->NewInstance(argc, argv));
  }
}

void Edge::Source(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  Edge* obj = ObjectWrap::Unwrap<Edge>(args.Holder());
  //v8::String::Utf8Value param1(obj->key->ToString());
  //std::string str = std::string(*param1);
  args.GetReturnValue().Set(Local<Value>(String::NewFromUtf8(isolate, obj->source_.c_str())));
}

void Edge::Dest(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  Edge* obj = ObjectWrap::Unwrap<Edge>(args.Holder());
  //v8::String::Utf8Value param1(obj->key->ToString());
  //std::string str = std::string(*param1);
  args.GetReturnValue().Set(Local<Value>(String::NewFromUtf8(isolate, obj->dest_.c_str())));
}

void Edge::AddBend(Bend* bend) { bends_.push_back(bend); }

void Edge::Bends(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  Edge* obj = ObjectWrap::Unwrap<Edge>(args.Holder());

  int size = obj->bends_.size();
  std::cout << size << std::endl;
  Local<Array> array = Array::New(isolate, size);

  for(int i = 0; i < size; i++) {
    array->Set(i, obj->bends_[i]->handle(isolate));
  }

  std::cout << "return bends" << std::endl;
  args.GetReturnValue().Set(Local<Array>(array));
}

Graph::Graph() {
}

Graph::~Graph() {
  for(auto n : graphNodes_)
    delete n;

  for(auto e : graphEdges_)
    delete e;
}

Graph* Graph::CreateNewGraph() {
  return new Graph();
}

std::vector<Node*> Graph::Nodes() { return nodes_; }
std::vector<Edge*> Graph::Edges() { return edges_; }
std::vector<GraphNode*> Graph::GraphNodes() { return graphNodes_; }
std::vector<GraphEdge*> Graph::GraphEdges() { return graphEdges_; }

void Graph::Init(Handle<Object> exports) {
  Isolate* isolate = Isolate::GetCurrent();

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "Graph"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  NODE_SET_PROTOTYPE_METHOD(tpl, "addNode", AddNode);
  NODE_SET_PROTOTYPE_METHOD(tpl, "addEdge", AddEdge);
  NODE_SET_PROTOTYPE_METHOD(tpl, "getNodes", Nodes);
  NODE_SET_PROTOTYPE_METHOD(tpl, "getEdges", Edges);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "Graph"),
               tpl->GetFunction());
}

void Graph::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  if (args.IsConstructCall()) {
    Graph* obj = new Graph();
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    // Invoked as plain function `MyObject(...)`, turn into construct call.
    const int argc = 0;
    Local<Value> argv[argc] = {};
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(cons->NewInstance(argc, argv));
  }
}

void Graph::AddNode(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  Graph* obj = ObjectWrap::Unwrap<Graph>(args.Holder());
  Node* node = ObjectWrap::Unwrap<Node>(args[0]->ToObject());
  obj->nodes_.push_back(node);

  GraphNode* graphNode = new GraphNode();
  graphNode->key = node->Key();
  graphNode->label = node->Label();
  graphNode->width = node->Width();
  graphNode->height = node->Height();
  obj->graphNodes_.push_back(graphNode);

  //args.GetReturnValue().Set(Local<Value>(String::NewFromUtf8(isolate, obj->source_.c_str())));
}

void Graph::AddEdge(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  Graph* obj = ObjectWrap::Unwrap<Graph>(args.Holder());
  Edge* edge = ObjectWrap::Unwrap<Edge>(args[0]->ToObject());
  obj->edges_.push_back(edge);

  GraphEdge* graphEdge = new GraphEdge();
  graphEdge->source = edge->Source();
  graphEdge->dest = edge->Dest();
  obj->graphEdges_.push_back(graphEdge);
  //args.GetReturnValue().Set(Local<Value>(String::NewFromUtf8(isolate, obj->dest_.c_str())));
}

void Graph::Nodes(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  Graph* obj = ObjectWrap::Unwrap<Graph>(args.Holder());

  int size = obj->nodes_.size();
  std::cout << size << std::endl;
  Local<Array> array = Array::New(isolate, size);

  for(int i = 0; i < size; i++) {
    array->Set(i, obj->nodes_[i]->handle(isolate));
  }

  std::cout << "return nodes" << std::endl;
  args.GetReturnValue().Set(Local<Array>(array));
}

void Graph::Edges(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  Graph* obj = ObjectWrap::Unwrap<Graph>(args.Holder());

  int size = obj->edges_.size();
  std::cout << size << std::endl;
  Local<Array> array = Array::New(isolate, size);

  for(int i = 0; i < size; i++) {
    array->Set(i, obj->edges_[i]->handle(isolate));
  }

  std::cout << "return edges" << std::endl;
  args.GetReturnValue().Set(Local<Array>(array));
}
