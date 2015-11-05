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

struct Property {
public:
  string name;
  string value;
  vector<Property*> array;
  map<string, Property*> properties;
  string str;
  Local<Object> obj;
  Property(Isolate* isolate) {
    str = "";
    //name = "default";
    //value = "default";
    obj = Object::New(isolate);
  }
  void Print(bool arr) {
    if(value == "#array") {
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
      //printf("\np: %i\n", properties.size());
      bool set = false;
      if(properties.size() > 1)
        set = true;

      if(set)
        printf("{");
      for(auto p = properties.begin(); p != properties.end(); p++) {
        //printf(" prop: %s ", (*p).first);
        (*p).second->Print(false);
        printf(",");
      }
      if(set)
        printf("}");
    }
  }
  Property* clone(Isolate* isolate) {
    Property* n = new Property(isolate);
    n->name = this->name;
    n->value = this->value;
    for(auto p = this->properties.begin(); p != this->properties.end(); p++) {
      n->properties[(*p).second->name] = (*p).second->clone(isolate);
    }
    for(int i = 0; i < this->array.size(); i++) {
      n->array[i] = this->array[i]->clone(isolate);
    }
    return n;
  }
  void ToString(bool arr, string* s) {
    if(value == "#array") {
      *s += "{" + name + ":";//printf("{\"%s\": ", name);
      *s += "["; //printf("[");
      for(int i = 0; i < array.size(); i++) {
        array[i]->ToString(true, s);
        *s += ",";//printf(", ");
      }
      *s += "]}"; //printf("]}");
    } else {
      if(!arr)
        *s += "\"" + name + "\": " + value; //printf("\"%s\": %s", name, value);
      else
        *s += value; //printf("%s", value);

      bool set = false;
      if(properties.size() > 1)
        set = true;

      if(set)
        *s += "{"; //printf("{");
      for(auto p = properties.begin(); p != properties.end(); p++) {
        (*p).second->ToString(false, s);
        *s += ","; //printf(",");
      }
      if(set)
        *s += "}"; //printf("}");
    }
  }
  void ToObject(bool arr, Isolate* isolate) {

    if(value == "#array") {
      int n = array.size();
      printf("\narray: %i", n);
      Local<Array> a = Array::New(isolate, n);
      for(int i = 0; i < n; i++) {
        Property* p = array[i];
        a->Set(i, p->obj);//String::NewFromUtf8(isolate,p->value.c_str()));
      }
      obj->Set(String::NewFromUtf8(isolate,name.c_str()), a);
    } else {
      if(!properties.empty()) {
        Local<Object> tmp = Object::New(isolate);
        for(auto p = properties.begin(); p != properties.end(); p++) {
          //(*p).second->ToObject(false, isolate);
          tmp->Set(String::NewFromUtf8(isolate,(*p).second->name.c_str()), (*p).second->obj);
        }
        obj->Set(String::NewFromUtf8(isolate,name.c_str()), tmp);
      } else {
        if(!name.empty() && !value.empty())
          obj->Set(String::NewFromUtf8(isolate,name.c_str()), String::NewFromUtf8(isolate,value.c_str()));
        else if(!name.empty())
          obj->Set(String::NewFromUtf8(isolate,name.c_str()), v8::Null(isolate));
      }
    }

    /*if(value == "#array") {
      int n = array.size();
      Local<Array> a = Array::New(isolate, n);
      for(int i = 0; i < n; i++) {
        Property* p = array[i];
        p->ToObject(arr, isolate);
        a->Set(i, p->obj);
      }
      obj->Set(String::NewFromUtf8(isolate,name.c_str()), a);
    } else {

      Local<Object> property = Object::New(isolate);
      for(auto p = properties.begin(); p != properties.end(); p++) {
        (*p).second->ToObject(false, isolate);
        Local<Array> p2 = (*p).second->obj->GetPropertyNames();
        v8::String::Utf8Value utfname(p2->Get(0)->ToString());
        string strname(*utfname);
        property->Set(String::NewFromUtf8(isolate,strname.c_str()), (*p).second->obj);
      }

      obj->Set(String::NewFromUtf8(isolate,name.c_str()), property);
    }*/
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
        xsr.buffer += str;
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

      Property* p = new Property(xsr.isolate_);
      p->name = string((char*)localname);
      xsr.opstack.push_back(p);

      if(!xsr.buffer.empty()) {
        //printf("\nleading %s \n", xsr.buffer.c_str());
        Property* tmp = new Property(xsr.isolate_);
        tmp->name = "#text";
        tmp->value = xsr.buffer;
        p->properties["#text"] = tmp;
        xsr.buffer.clear();
        xsr.leadingtext = true;
        xsr.leadingproperty = tmp;
        xsr.leadingname = (char*)localname;
      }

      //printf( "startElementNs: name = '%s' prefix = '%s' uri = (%p)'%s'\n", localname, prefix, URI, URI );
      for ( int indexNamespace = 0; indexNamespace < nb_namespaces; ++indexNamespace )
      {
         const xmlChar *prefix = namespaces[indexNamespace*2];
         const xmlChar *nsURI = namespaces[indexNamespace*2+1];
         //printf( "  namespace: name='%s' uri=(%p)'%s'\n", prefix, nsURI, nsURI );
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
           p->properties[tmp] = ap;
           //printf("attribute: %s value: %s\n", localname, value.c_str());
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

      //printf("end prop: %s\n", property.name.c_str());

      // if at the end of a wrapping element (last callback was also an endElement)
      if(!xsr.inelem) {

        //printf("opstack: %i\n", xsr.opstack.size());
        //printf("ipstack: %i\n", xsr.ipstack.size());

        Property* last = property;
        for(int i = 0; i < xsr.ipstack.size(); i++) {
          if(xsr.opstack.size() == 1 && i == 0) {
            property = xsr.opstack.back();
          }

          //printf("wrapper: %s prop: %s\n", property.name.c_str(), xsr.ipstack[i].name.c_str());
          // is array
          if(property->properties.find(xsr.ipstack[i]->name) != property->properties.end()) {
            Property* old = property->properties[xsr.ipstack[i]->name];
            Property* p = old->clone(xsr.isolate_);
            old->value = "#array";
            old->array.push_back(p);
            old->array.push_back(xsr.ipstack[i]);
            old->ToObject(false, xsr.isolate_);
            p->ToObject(false, xsr.isolate_);
          } else
            property->properties[xsr.ipstack[i]->name] = xsr.ipstack[i];
          //printf("properties %i\n", property.properties.size());
          property = last;
          xsr.ipstack[i]->ToObject(false, xsr.isolate_);
        }

        xsr.ipstack.clear();
      }

      if(!xsr.buffer.empty()) {
        //printf("size: %i", property->properties.size());
        if(!property->properties.empty()) {
          if(xsr.leadingtext) {
            //printf(xsr.leadingname.c_str());
            Property* tmp = new Property(xsr.isolate_);
            tmp->name = "#text";
            tmp->value = xsr.leadingproperty->value;
            property->properties[tmp->name] = tmp;
            tmp = new Property(xsr.isolate_);
            tmp->name = xsr.leadingname;
            tmp->value = xsr.buffer;
            property->properties[tmp->name] = tmp;
          } else {
            Property* tmp = new Property(xsr.isolate_);
            tmp->name = "#text";
            tmp->value = xsr.buffer;
            property->properties["#text"] = tmp;
        }
        } else
          property->value = xsr.buffer;

        //printf("\nbuffer: %s\n", xsr.buffer);
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
      //Xmlsonator &xsr = *( static_cast<Xmlsonator *>( ctx ) );
      va_list args;
      va_start(args, msg);
      vprintf( msg, args );
      va_end(args);
   }

   static void warning( void * ctx,
                        const char * msg,
                        ... )
   {
      //Xmlsonator &xsr = *( static_cast<Xmlsonator *>( ctx ) );
      va_list args;
      va_start(args, msg);
      vprintf( msg, args );
      va_end(args);
   }

   Local<Object> getObject() { return this->object_; }
   Property* getProperty() { return this->property; }

private:

   // return Object
   Local<Object> object_;
   Isolate* isolate_;
   bool inelem;
   string buffer;
   vector<string> elemnames;
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
  xsr.getProperty()->ToString(false, &(xsr.getProperty()->str));

  printf("\n%s", xsr.getProperty()->str.c_str());

  xmlCleanupParser();

  xmlMemoryDump();

}

void init(Handle<Object> exports) {
  NODE_SET_METHOD(exports, "parseXML", parse);
}

NODE_MODULE(xmlsonator, init)
