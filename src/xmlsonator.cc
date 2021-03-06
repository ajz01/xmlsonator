// xmlsonator - xml to json converter for node using the libxml++ sax parser API
// Andrew Zdenek - ajz01
// Oct 27, 2015

#include <node.h>
#include <node_object_wrap.h>
#include <node_buffer.h>
#include <v8.h>
#include <uv.h>
#include <stdio.h>
#include <memory.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <string>
#include <vector>
#include <map>;

using namespace std;
using namespace v8;
using namespace node;

// reprents a JSON property which can be either a
// key value pair (plain text) or an Object
// the property can also be single instance or an
// array of either of these types
struct Property {
public:
  enum type { pvalue, pobject };
  bool isArray;
  type type;
  string name;
  string value;
  vector<Property*> array;
  map<string, Property*> properties;
  string str;
  Local<Object> obj;
  inline Property(Isolate* isolate) {
    type = Property::pvalue;
    isArray = false;
    str = "";
    obj = Object::New(isolate);
  }

  // print the JSON to stdout
  // mostly for debugging or
  // inspection purposes
  inline void Print(bool arr) {
    if(isArray) {
      printf("{\"%s\": ", name.c_str());
      printf("[");
      for(int i = 0; i < array.size(); i++) {
        array[i]->Print(true);
        printf(", ");
      }
      printf("]}");
    } else {
      if(!arr)
        printf("\"%s\": %s", name.c_str(), value.c_str());
      else
        printf("%s", value.c_str());

      bool set = false;
      if(properties.size() > 1)
        set = true;

      if(set)
        printf("{");
      for(auto p = properties.begin(); p != properties.end(); p++) {
        (*p).second->Print(false);
        printf(",");
      }
      if(set)
        printf("}");
    }
  }

  // clone (deep copy) an existing property
  inline Property* clone(Isolate* isolate) {
    Property* n = new Property(isolate);
    n->type = this->type;
    n->isArray = this->isArray;
    n->name = this->name;
    n->value = this->value;
    for(auto p = this->properties.begin(); p != this->properties.end(); p++) {
      n->properties[(*p).second->name] = (*p).second->clone(isolate);
    }
    n->array.reserve(this->array.size());
    for(int i = 0; i < this->array.size(); i++) {
      n->array[i] = this->array[i]->clone(isolate);
    }
    return n;
  }

  // serialize (convert) to JSON string
  // may use this to send back to
  // javascript in case the user wants
  // to use the strinfified version instead
  // of an actual object. This could be
  // passed back as a custom javascript
  // object with two properties one containing
  // the JSON Object and the other the string
  // since they both can be constructed after
  // a single pass through the parser
  inline void ToString(bool arr, string* s) {
    if(isArray) {
      *s += "{" + name + ":";
      *s += "[";
      for(int i = 0; i < array.size(); i++) {
        array[i]->ToString(true, s);
        *s += ",";
      }
      *s += "]}";
    } else {
      if(!arr)
        *s += "\"" + name + "\": " + value;
      else
        *s += value;

      bool set = false;
      if(properties.size() > 1)
        set = true;

      if(set)
        *s += "{";
      for(auto p = properties.begin(); p != properties.end(); p++) {
        (*p).second->ToString(false, s);
        *s += ",";
      }
      if(set)
        *s += "}";
    }
  }

  // serialize (convert) to the local JSON object
  // for use in javascript code
  inline void ToObject(bool arr, Isolate* isolate) {
    if(isArray) {
      int n = array.size();
      Local<Array> a = Array::New(isolate, n);
      for(int i = 0; i < n; i++) {
        Property* p = array[i];
        if(p->type == Property::pobject)
          a->Set(i, p->obj);
        else
          a->Set(i, String::NewFromUtf8(isolate,p->value.c_str()));
      }
      obj->Set(String::NewFromUtf8(isolate,name.c_str()), a);
    } else {
      if(!properties.empty()) {
        Local<Object> tmp = Object::New(isolate);
        for(auto p = properties.begin(); p != properties.end(); p++) {
          if((*p).second->isArray) {
            tmp->Set(String::NewFromUtf8(isolate,(*p).second->name.c_str()), (*p).second->obj->Get(String::NewFromUtf8(isolate,(*p).second->name.c_str())));
          } else
            if((*p).second->type == Property::pobject)
              tmp->Set(String::NewFromUtf8(isolate,(*p).second->name.c_str()), (*p).second->obj);
            else
              tmp->Set(String::NewFromUtf8(isolate,(*p).second->name.c_str()), String::NewFromUtf8(isolate,(*p).second->value.c_str()));
        }
        obj->Set(String::NewFromUtf8(isolate,name.c_str()), tmp);
      } else {
        if(type == Property::pobject)
          obj->Set(String::NewFromUtf8(isolate,name.c_str()), obj);
        else {
          if(!name.empty() && !value.empty())
            obj->Set(String::NewFromUtf8(isolate,name.c_str()), String::NewFromUtf8(isolate,value.c_str()));
          else if(!name.empty())
            obj->Set(String::NewFromUtf8(isolate,name.c_str()), v8::Null(isolate));
        }
      }
    }
  }
};

// used to initialize a libml SAX2 parser and construct a
// javascript Object containing the json equivlant of an
// xml representation passed as a node Buffer.
class Xmlsonator
{
public:

  // constructor. set return value object and v8 isolate.
  Xmlsonator(Local<Object> obj, Isolate* isolate) {
    this->object_ = obj;
    this->isolate_ = isolate;
    this->inelem = false;
    this->leadingtext = false;
  }

// trim whitespace
static inline std::string trim(std::string& str)
{
str.erase(0, str.find_first_not_of(" \t\n"));       //prefixing spaces
str.erase(str.find_last_not_of(" \t\n")+1);         //surfixing spaces
return str;
}

  // handle element inner text
  static void characters(void * ctx, const xmlChar * ch, int len) {
    Xmlsonator &xsr = *( static_cast<Xmlsonator *>( ctx ) );
      string str((char*)ch, len);
      str = trim(str);
      if(!str.empty()) {
        string str2((char*)ch, len);
        xsr.buffer += str2;
      }
  }

   static void startElementNs( void * ctx,
                               const xmlChar * localname,
                               const xmlChar * prefix,
                               const xmlChar * URI,
                               int nb_namespaces,
                               const xmlChar ** namespaces,
                               int nb_attributes,
                               int nb_defaulted,
                               const xmlChar ** attributes )
   {
      Xmlsonator &xsr = *( static_cast<Xmlsonator *>( ctx ) );

      // handle enclosed text
      if(!xsr.buffer.empty() && !xsr.opstack.empty()) {
        Property* old = xsr.opstack.back();
        Property* tmp = new Property(xsr.isolate_);
        tmp->name = "#text";
        tmp->value = xsr.buffer;
        tmp->ToObject(false, xsr.isolate_);
        old->properties["#text"] = tmp;
        xsr.buffer.clear();
        xsr.leadingtext = true;
        xsr.leadingproperty = tmp;
        xsr.leadingname = (char*)localname;
      }

      Property* p = new Property(xsr.isolate_);
      p->name = string((char*)localname);
      xsr.opstack.push_back(p);

      for ( int indexNamespace = 0; indexNamespace < nb_namespaces; ++indexNamespace )
      {
         const xmlChar *prefix = namespaces[indexNamespace*2];
         const xmlChar *nsURI = namespaces[indexNamespace*2+1];
      }

      // if the element has attributes create an object otherwise store the name
      if(nb_attributes) {
        unsigned int index = 0;
        for ( int indexAttribute = 0;
              indexAttribute < nb_attributes;
              ++indexAttribute, index += 5 )
        {
           const xmlChar *localname = attributes[index];
           //const xmlChar *prefix = attributes[index+1];
           //const xmlChar *nsURI = attributes[index+2];
           const xmlChar *valueBegin = attributes[index+3];
           const xmlChar *valueEnd = attributes[index+4];
           std::string value( (const char *)valueBegin, (const char *)valueEnd );

           Property* ap = new Property(xsr.isolate_);
           string tmp((char*)localname);
           ap->name = '@' + tmp;
           ap->value = value;
           ap->ToObject(false, xsr.isolate_);
           p->properties[ap->name] = ap;
           p->type = Property::pobject;
        }
      }
      xsr.inelem = true;
   }

   static void endElementNs( void * ctx,
                             const xmlChar * localname,
                             const xmlChar * prefix,
                             const xmlChar * URI )
   {
      Xmlsonator &xsr = *( static_cast<Xmlsonator *>( ctx ) );

      Property* property = xsr.opstack.back();
      xsr.opstack.pop_back();
      xsr.property = property;

      // if at the end of a wrapping element (last callback was also an endElement)
      if(!xsr.inelem) {
        property->type = Property::pobject;

        // wrap inner elements
        for(int i = 0; i < xsr.ipstack.size(); i++) {

          // is array
          if(property->properties.find(xsr.ipstack[i]->name) != property->properties.end()) {
            Property* old = property->properties[xsr.ipstack[i]->name];
            if(old->array.empty()) {
              Property* p = old->clone(xsr.isolate_);
              p->isArray = false;
              old->isArray = true;
              p->ToObject(false, xsr.isolate_);
              old->array.push_back(p);
            }
            old->array.push_back(xsr.ipstack[i]);
            old->ToObject(false, xsr.isolate_);
          } else
            property->properties[xsr.ipstack[i]->name] = xsr.ipstack[i];
          xsr.ipstack[i]->ToObject(false, xsr.isolate_);
        }

        xsr.ipstack.clear();
      }

      if(!xsr.buffer.empty()) {
        if(!property->properties.empty()) {
            Property* tmp = new Property(xsr.isolate_);
            tmp->name = "#text";
            tmp->value = xsr.buffer;
            tmp->ToObject(false, xsr.isolate_);
            property->properties["#text"] = tmp;
        } else
          property->value = xsr.buffer;

        xsr.buffer.clear();
      }

      xsr.ipstack.push_back(property);

      xsr.inelem = false;
      xsr.leadingtext = false;
   }

   static void error( void * ctx,
                      const char * msg,
                      ... )
   {
      va_list args;
      va_start(args, msg);
      vprintf( msg, args );
      va_end(args);
   }

   static void warning( void * ctx,
                        const char * msg,
                        ... )
   {
      va_list args;
      va_start(args, msg);
      vprintf( msg, args );
      va_end(args);
   }

   Local<Object> getObject() { return this->object_; }
   Property* getProperty() { return this->property; }

private:

   Local<Object> object_;             // return Object
   Isolate* isolate_;
   bool inelem;
   string buffer;
   vector<Property*> opstack;
   vector<Property*> ipstack;
   Property* property;
   bool leadingtext;
   Property* leadingproperty;
   string leadingname;
};

void parse(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  Local<Object> bufferObj = args[0]->ToObject();
  char* bufferData   = Buffer::Data(bufferObj);
  size_t bufferLength = Buffer::Length(bufferObj);

  xmlSAXHandler saxHandler;
  memset( &saxHandler, 0, sizeof(saxHandler) );
  saxHandler.initialized = XML_SAX2_MAGIC;
  saxHandler.characters = &Xmlsonator::characters;
  saxHandler.startElementNs = &Xmlsonator::startElementNs;
  saxHandler.endElementNs = &Xmlsonator::endElementNs;
  saxHandler.warning = &Xmlsonator::warning;
  saxHandler.error = &Xmlsonator::error;

  Xmlsonator xsr(Object::New(isolate), isolate);
  int result = xmlSAXUserParseMemory( &saxHandler, &xsr, bufferData, int(bufferLength) );
  if ( result != 0 )
  {
    printf("Failed to parse document.\n" );
  } else {
    xsr.getProperty()->ToObject(false, isolate);
    args.GetReturnValue().Set(xsr.getProperty()->obj);
  }

  //xsr.getProperty()->Print(false);
  //xsr.getProperty()->ToString(false, &(xsr.getProperty()->str));

  //printf("\n%s", xsr.getProperty()->str.c_str());

  xmlCleanupParser();

  xmlMemoryDump();

}

void init(Handle<Object> exports) {
  NODE_SET_METHOD(exports, "toJson", parse);
}

NODE_MODULE(xmlsonator, init)
